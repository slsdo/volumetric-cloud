#include "Noise.h"

Noise::Noise(float per, int oct, int seed)
{
	persistence = per;
	octaves = oct;
	
	// Add more randomness
	srand((unsigned int)time(NULL));
	seed = seed + rand()%100;

	seed1 = seed*3; // *0.3
	seed2 = seed*7; // *0.7
}

float Noise::Noise1(int x)
{
	int n = x;
	n = (n<<13)^n;
	float noise = 1.0 - ((n*(n*n*15731 + 789221) + 1376312589) &0x7fffffff)/1073741824.0; // [-1.0, 1.0]
	return noise;
}

float Noise::Noise2(int x, int y)
{
	int n = x + y*seed1;
	n = (n<<13)^n;
    float noise = 1.0 - ((n*(n*n*15731 + 789221) + 1376312589) &0x7fffffff)/1073741824.0; // [-1.0, 1.0]
	return noise;
}

float Noise::Noise3(int x, int y, int z)
{
	int n = x + y*seed1 + z*seed2;
	n = (n<<13)^n;
    float noise = 1.0 - ((n*(n*n*15731 + 789221) + 1376312589) &0x7fffffff)/1073741824.0; // [-1.0, 1.0]
	return noise;
}

float Noise::SmoothNoise1(int x)
{
    float edges = (Noise1(x - 1)  +  Noise1(x + 1))*0.25; // /4
    float center = Noise1(x)*0.5; // /2

	return center + edges;
}

float Noise::SmoothNoise2(int x, int y)
{
	float corners = (Noise2(x - 1, y - 1) + Noise2(x + 1, y - 1) +
					 Noise2(x - 1, y + 1) + Noise2(x + 1, y + 1))*0.0625; // /16
    float edges = (Noise2(x - 1, y) + Noise2(x + 1, y) + Noise2(x, y - 1) + Noise2(x, y + 1))*0.125; // /8
    float center = Noise2(x, y)*0.25; // /4

    return corners + edges + center;
}

float Noise::SmoothNoise3(int x, int y, int z)
{
	float corners = (Noise3(x + 1, y + 1, z + 1) + Noise3(x + 1, y + 1, z - 1) +
					 Noise3(x + 1, y - 1, z + 1) + Noise3(x + 1, y - 1, z - 1) + 
					 Noise3(x - 1, y + 1, z + 1) + Noise3(x - 1, y + 1, z - 1) +
					 Noise3(x - 1, y - 1, z + 1) + Noise3(x - 1, y - 1, z - 1))*0.015625; // /64
	float edges = (Noise3(x + 1, y + 1, z) + Noise3(x + 1, y - 1, z) +
				   Noise3(x - 1, y + 1, z) + Noise3(x - 1, y - 1, z) + 
				   Noise3(x, y + 1, z + 1) + Noise3(x, y + 1, z - 1) +
				   Noise3(x, y - 1, z + 1) + Noise3(x, y - 1, z - 1) + 
				   Noise3(x + 1, y, z + 1) + Noise3(x + 1, y, z - 1) +
				   Noise3(x - 1, y, z + 1) + Noise3(x - 1, y, z - 1))*0.03125; // /32
	float faces = (Noise3(x + 1, y, z) + Noise3(x - 1, y, z) + Noise3(x, y + 1, z) +
				   Noise3(x, y - 1, z) + Noise3(x, y, z + 1) + Noise3(x, y, z - 1))*0.0625; // /16
	float center = Noise3(x, y, z)*0.125; // /8

    return corners + edges + faces + center;
}

float Noise::LinearInterpolate(float a, float b, float x)
{
	return a*(1 - x) + b*x;
}

float Noise::CosineInterpolate(float a, float b, float x)
{
	float ft = x*3.14159265358979323846;
	float f=(1 - cos(ft))*0.5;

	return  a*(1 - f) + b*f;
}

float Noise::CubicInterpolate(float v0, float v1, float v2, float v3, float x)
{
	float p = (v3 - v2) - (v0 - v1);
	float q = (v0 - v1) - p;
	float r = v2 - v0;
	float s = v1;

	return p*x*x*x + q*x*x + r*x + s;
}

float Noise::InterpolateNoise1(float x)
{
	int i_x = int(x);
	float f_x = x - i_x;

	float u[2];

	u[0] = SmoothNoise1(i_x);
	u[1] = SmoothNoise1(i_x + 1);

	return CosineInterpolate(u[0], u[1], f_x);
}

float Noise::InterpolateNoise2(float x, float y)
{
	int i_x = int(x);
	int i_y = int(x);
	float f_x = x - i_x;
	float f_y = y - i_y;

	float u[4], v[2];

	u[0] = SmoothNoise2(i_x, i_y);
	u[1] = SmoothNoise2(i_x + 1, i_y);
	u[3] = SmoothNoise2(i_x, i_y + 1);
	u[4] = SmoothNoise2(i_x + 1, i_y + 1);

    v[0] = CosineInterpolate(u[0], u[1], f_x);
    v[1] = CosineInterpolate(u[2], u[3], f_x);

    return CosineInterpolate(v[0], v[1], f_y);
}

float Noise::InterpolateNoise3(float x, float y, float z)
{
	int i_x = int(x);
	int i_y = int(y);
	int i_z = int(z);
	float f_x = x - i_x;
	float f_y = y - i_y;
	float f_z = z - i_z;

	float u[8], v[4], w[2];

	u[0] = SmoothNoise3(i_x, i_y, i_z);
	u[1] = SmoothNoise3(i_x + 1, i_y, i_z);
	u[2] = SmoothNoise3(i_x, i_y + 1, i_z);
	u[3] = SmoothNoise3(i_x + 1, i_y + 1, i_z);
	u[4] = SmoothNoise3(i_x, i_y, i_z + 1);
	u[5] = SmoothNoise3(i_x + 1, i_y, i_z + 1);
	u[6] = SmoothNoise3(i_x, i_y + 1, i_z + 1);
	u[7] = SmoothNoise3(i_x + 1, i_y + 1, i_z + 1);

	v[0] = CosineInterpolate(u[0], u[1], f_x);
	v[1] = CosineInterpolate(u[2], u[3], f_x);
	v[2] = CosineInterpolate(u[4], u[5], f_x);
	v[3] = CosineInterpolate(u[6], u[7], f_x);

	w[0] = CosineInterpolate(v[0], v[1], f_y);
	w[1] = CosineInterpolate(v[2], v[3], f_y);

	return CosineInterpolate(w[0], w[1], f_z);
}

float Noise::PerlinNoise1(float x)
{
	float total = 0;
	float p = persistence;
	int n = octaves - 1;
	float frequency, amplitude;

	for (int ii = 0; ii < n; ii++) {
		frequency = powf(2, ii);
        amplitude = powf(p, ii);

		total = total + InterpolateNoise1(x*frequency)*amplitude;
	}
	
	// [0.0, 1.0] (need positive density)
	if (total < 0) total = total + 1.0;
    if (total > 1) total = total - 1.0;

	return total;
}

float Noise::PerlinNoise2(float x, float y)
{
	float total = 0;
	float p = persistence;
	int n = octaves - 1;
	float frequency, amplitude;

	for (int ii = 0; ii < n; ii++) {
		frequency = powf(2, ii);
        amplitude = powf(p, ii);

		total = total + InterpolateNoise2(x*frequency, y*frequency)*amplitude;
	}
	
	// [0.0, 1.0] (need positive density)
	if (total < 0) total = total + 1.0;
    if (total > 1) total = total - 1.0;

	return total;
}

float Noise::PerlinNoise3(float x, float y, float z)
{
	float total = 0;
	float p = persistence;
	int n = octaves - 1;
	float frequency, amplitude;

	for (int ii = 0; ii < n; ii++) {
		frequency = powf(2, ii);
        amplitude = powf(p, ii);

		total = total + InterpolateNoise3(x*frequency, y*frequency, z*frequency)*amplitude;
	}

	// [0.0, 1.0] (need positive density)
	if (total < 0) total = total + 1.0;
    if (total > 1) total = total - 1.0;

	return total;
}