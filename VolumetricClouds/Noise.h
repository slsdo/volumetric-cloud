// Implemented algorithm described in http://freespace.virgin.net/hugo.elias/models/m_perlin.htm

#ifndef NOISE_H_
#define NOISE_H_

#include <math.h>
#include <time.h>
#include <stdlib.h>

class Noise
{
public:
	Noise() { persistence = 0.5; octaves = 5; seed1 = 57, seed2 = 131; }
	~Noise() { }
	Noise(float persistence, int octaves, int seed);

	float PerlinNoise1(float x); // Generate 1D Perlin Noise
	float PerlinNoise2(float x, float y); // Generate 2D Perlin Noise
	float PerlinNoise3(float x, float y, float z); // Generate 3D Perlin Noise

private:
	float Noise1(int x); // Generate 1D noise
	float Noise2(int x, int y); // Generate 2D noise
	float Noise3(int x, int y, int z); // Generate 3D noise

	float SmoothNoise1(int x); // Create 1D smooth noise
	float SmoothNoise2(int x, int y); // Create 3D smooth noise
	float SmoothNoise3(int x, int y, int z); // Create 3D smooth noise

	float LinearInterpolate(float a, float b, float x); // Linear interpolation
	float CosineInterpolate(float a, float b, float x); // Cosine interpolation
	float CubicInterpolate(float v0, float v1, float v2, float v3, float x); // Cubic interpolation

	float InterpolateNoise1(float x); // Create 1D interpolated noise
	float InterpolateNoise2(float x, float y); // Create 2D interpolated noise
	float InterpolateNoise3(float x, float y, float z); // Create 3D interpolated noise
	
	float persistence;
	int octaves;
	int seed1, seed2;
};

#endif