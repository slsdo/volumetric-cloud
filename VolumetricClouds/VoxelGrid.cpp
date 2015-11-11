#include "VoxelGrid.h"

VoxelGrid::VoxelGrid()
{
	voxelSize = 0.0;
	xCount = 0;
	yCount = 0;
	zCount = 0;
	defaultDensity = 0.0;
}

VoxelGrid::VoxelGrid(double size, int x, int y, int z, double d)
{
	voxelSize = size;
	xCount = x;
	yCount = y;
	zCount = z;
	defaultDensity = d;
	
	// Create voxel grid
	for (double kk = 0; kk < zCount; kk++) {
		for (double jj = 0; jj < yCount; jj++) {
			for (double ii = 0; ii < xCount; ii++) {
				Voxel *voxel = new Voxel();
				voxel->density = defaultDensity;
				voxelGrid.push_back(voxel);
			}
		}
	}

	mat4 mat;	
	mat[0][0] = (double)xCount*voxelSize; mat[0][1] = 0; mat[0][2] = 0; mat[0][3] = (double)xCount*voxelSize*0.5;
	mat[1][0] = 0; mat[1][1] = (double)yCount*voxelSize; mat[1][2] = 0; mat[1][3] = (double)yCount*voxelSize*0.5;
	mat[2][0] = 0; mat[2][1] = 0; mat[2][2] = (double)zCount*voxelSize; mat[2][3] = (double)zCount*voxelSize*0.5;
	mat[3][0] = 0; mat[3][1] = 0; mat[3][2] = 0; mat[3][3] = 1;
	t_inv = mat.inverse();
	mat4 tstar = mat;
	tstar[0][3] = 0.0; tstar[1][3] = 0.0; tstar[2][3] = 0.0;
	tstar_inv = tstar.inverse();
}

VoxelGrid::VoxelGrid(string filename)
{
	string line; // Temporary storage
	ifstream infile(filename.c_str()); // Open as stream
	if (infile.fail()) {
		cout << "Failed to open file!" << endl;
		return;
	}

	getline(infile, line); // Get voxel size
	voxelSize = atof(line.c_str());
	getline(infile, line); // Get voxel grid x, y, z length
	stringstream ssrc(line); ssrc >> xCount >> yCount >> zCount;
	getline(infile, line); // Get default density
	defaultDensity = atof(line.c_str());
	
	while (!infile.eof()) {
		getline(infile, line);
		if (line.empty()) break;
		Voxel *voxel = new Voxel();
		voxel->density = atof(line.c_str());
		voxelGrid.push_back(voxel);
	}

	infile.close();

	mat4 mat;	
	mat[0][0] = (double)xCount*voxelSize; mat[0][1] = 0; mat[0][2] = 0; mat[0][3] = (double)xCount*voxelSize*0.5;
	mat[1][0] = 0; mat[1][1] = (double)yCount*voxelSize; mat[1][2] = 0; mat[1][3] = (double)yCount*voxelSize*0.5;
	mat[2][0] = 0; mat[2][1] = 0; mat[2][2] = (double)zCount*voxelSize; mat[2][3] = (double)zCount*voxelSize*0.5;
	mat[3][0] = 0; mat[3][1] = 0; mat[3][2] = 0; mat[3][3] = 1;
	t_inv = mat.inverse();
	mat4 tstar = mat;
	tstar[0][3] = 0.0; tstar[1][3] = 0.0; tstar[2][3] = 0.0;
	tstar_inv = tstar.inverse();
}

VoxelGrid::~VoxelGrid()
{
	voxelGrid.clear();
}

void VoxelGrid::setVoxelColor(int x, int y, int z, vec3 rgb)
{
	// If outside of grid
	if ((x >= xCount) || (y >= yCount) || (z >= zCount) || (x < 0) || (y < 0) || (z < 0)) {
		return;
	}
	
	// Get array index
	int index = getVoxelIndex(x, y, z);
	voxelGrid[index]->color = rgb;
}

void VoxelGrid::setVoxelColor(vec3 xyz, vec3 rgb)
{
	vec3 ijk = world2voxel(xyz);
	return setVoxelColor(ijk[0], ijk[1], ijk[2], rgb);
}

void VoxelGrid::setVoxelDensity(int x, int y, int z, double d)
{
	// If outside of grid
	if ((x >= xCount) || (y >= yCount) || (z >= zCount) || (x < 0) || (y < 0) || (z < 0)) {
		return;
	}
	
	// Get array index
	int index = getVoxelIndex(x, y, z);
	voxelGrid[index]->density = d;
}

void VoxelGrid::setVoxelDensity(vec3 xyz, double d)
{
	vec3 ijk = world2voxel(xyz);
	return setVoxelDensity(ijk[0], ijk[1], ijk[2], d);
}

void VoxelGrid::setVoxelTransmissivity(int x, int y, int z, double q)
{
	// If outside of grid
	if ((x >= xCount) || (y >= yCount) || (z >= zCount) || (x < 0) || (y < 0) || (z < 0)) {
		return;
	}
	
	// Get array index
	int index = getVoxelIndex(x, y, z);
	voxelGrid[index]->transmissivity = q;
}

void VoxelGrid::setVoxelTransmissivity(vec3 xyz, double q)
{
	vec3 ijk = world2voxel(xyz);
	return setVoxelTransmissivity(ijk[0], ijk[1], ijk[2], q);
}

vec3 VoxelGrid::getVoxelColor(int x, int y, int z)
{
	// If outside of grid
	if ((x >= xCount) || (y >= yCount) || (z >= zCount) || (x < 0) || (y < 0) || (z < 0)) {
		return vec3(0, 0, 0);
	}

	// Get array index
	int index = getVoxelIndex(x, y, z);
	return voxelGrid[index]->color;
}

vec3 VoxelGrid::getVoxelColor(vec3 xyz)
{
	vec3 ijk = world2voxel(xyz);
	return getVoxelColor(ijk[0], ijk[1], ijk[2]);
}

double VoxelGrid::getVoxelDensity(int x, int y, int z)
{
	// If outside of grid
	if ((x >= xCount) || (y >= yCount) || (z >= zCount) || (x < 0) || (y < 0) || (z < 0)) {
		return defaultDensity;
	}

	// Get array index
	int index = getVoxelIndex(x, y, z);
	return voxelGrid[index]->density;
}

double VoxelGrid::getVoxelDensity(vec3 xyz)
{
	vec3 ijk = world2voxel(xyz);
	return getVoxelDensity(ijk[0], ijk[1], ijk[2]);
}

double VoxelGrid::getVoxelTransmissivity(int x, int y, int z)
{
	// If outside of grid
	if ((x >= xCount) || (y >= yCount) || (z >= zCount) || (x < 0) || (y < 0) || (z < 0)) {
		return 0;
	}

	// Get array index
	int index = getVoxelIndex(x, y, z);
	return voxelGrid[index]->transmissivity;
}

double VoxelGrid::getVoxelTransmissivity(vec3 xyz)
{
	vec3 ijk = world2voxel(xyz);
	return getVoxelTransmissivity(ijk[0], ijk[1], ijk[2]);
}

vec3 VoxelGrid::interpVoxelColor(double x, double y, double z)
{
	vec3 interpColor(0, 0, 0);
	double sizeRatio = 1/voxelSize;

	// Convert from coordinate to index
	int x_index = floor(x*sizeRatio);
	int y_index = floor(y*sizeRatio);
	int z_index = floor(z*sizeRatio);

	// If outside of grid
	if ((x_index >= xCount) || (y_index >= yCount) || (z_index >= zCount) || (x_index < 0) || (y_index < 0) || (z_index < 0)) {
		return vec3(0, 0, 0);
	}

	// Tri-linear interpolation
	for (double kk = z_index; kk <= z_index + 1; kk++) {
		for (double jj = y_index; jj <= y_index + 1; jj++) {
			for (double ii = x_index; ii <= x_index + 1; ii++) {
				double wx = (voxelSize - fabs(x - ii*voxelSize))*sizeRatio;
				double wy = (voxelSize - fabs(y - jj*voxelSize))*sizeRatio;
				double wz = (voxelSize - fabs(z - kk*voxelSize))*sizeRatio;

				double weight = wx*wy*wz;
				interpColor += weight*getVoxelColor(ii, jj, kk);
			}
		}
	}
	
	return interpColor;
}

vec3 VoxelGrid::interpVoxelColor(vec3 xyz)
{
	return interpVoxelColor(xyz[0], xyz[1], xyz[2]);
}

double VoxelGrid::interpVoxelDensity(double x, double y, double z)
{
	double interpDensity = 0;
	double sizeRatio = 1/voxelSize;

	// Convert from coordinate to index
	int x_index = floor(x*sizeRatio);
	int y_index = floor(y*sizeRatio);
	int z_index = floor(z*sizeRatio);

	// If outside of grid
	if ((x_index >= xCount) || (y_index >= yCount) || (z_index >= zCount) || (x_index < 0) || (y_index < 0) || (z_index < 0)) {
		return defaultDensity;
	}

	// Tri-linear interpolation
	for (double kk = z_index; kk <= z_index + 1; kk++) {
		for (double jj = y_index; jj <= y_index + 1; jj++) {
			for (double ii = x_index; ii <= x_index + 1; ii++) {
				double wx = (voxelSize - fabs(x - ii*voxelSize))*sizeRatio;
				double wy = (voxelSize - fabs(y - jj*voxelSize))*sizeRatio;
				double wz = (voxelSize - fabs(z - kk*voxelSize))*sizeRatio;

				double weight = wx*wy*wz;
				interpDensity += weight*getVoxelDensity(ii, jj, kk);
			}
		}
	}
	
	return interpDensity;
}

double VoxelGrid::interpVoxelDensity(vec3 xyz)
{
	return interpVoxelDensity(xyz[0], xyz[1], xyz[2]);
}

double VoxelGrid::interpVoxelTransmissivity(double x, double y, double z)
{
	double interpTransmissivity = 0;
	double sizeRatio = 1/voxelSize;

	// Convert from coordinate to index
	int x_index = floor(x*sizeRatio);
	int y_index = floor(y*sizeRatio);
	int z_index = floor(z*sizeRatio);

	// If outside of grid
	if ((x_index >= xCount) || (y_index >= yCount) || (z_index >= zCount) || (x_index < 0) || (y_index < 0) || (z_index < 0)) {
		return 0;
	}

	// Tri-linear interpolation
	for (double kk = z_index; kk <= z_index + 1; kk++) {
		for (double jj = y_index; jj <= y_index + 1; jj++) {
			for (double ii = x_index; ii <= x_index + 1; ii++) {
				double wx = (voxelSize - fabs(x - ii*voxelSize))*sizeRatio;
				double wy = (voxelSize - fabs(y - jj*voxelSize))*sizeRatio;
				double wz = (voxelSize - fabs(z - kk*voxelSize))*sizeRatio;

				double weight = wx*wy*wz;
				interpTransmissivity += weight*getVoxelTransmissivity(ii, jj, kk);
			}
		}
	}
	
	return interpTransmissivity;
}

double VoxelGrid::interpVoxelTransmissivity(vec3 xyz)
{
	return interpVoxelTransmissivity(xyz[0], xyz[1], xyz[2]);
}

int VoxelGrid::getVoxelIndex(int x, int y, int z)
{
	// Return index of voxel in array based on its x, y, z index
	return x + y*xCount + z*xCount*yCount;
}

vector<Voxel*> VoxelGrid::getVoxelGrid() { return voxelGrid; }
double VoxelGrid::getVoxelSize() { return voxelSize; }
double VoxelGrid::getDefaultDensity() { return defaultDensity; }
int VoxelGrid::getMaxX() { return xCount; }
int VoxelGrid::getMaxY() { return yCount; }
int VoxelGrid::getMaxZ() { return zCount; }
mat4 VoxelGrid::getTInv() { return t_inv; }
mat4 VoxelGrid::getTStarInv() { return tstar_inv; }

bool VoxelGrid::isInsideGrid(vec3 xyz)
{
	double x_max = xCount*voxelSize;
	double y_max = yCount*voxelSize;
	double z_max = zCount*voxelSize;

	if (xyz[0] < x_max && xyz[1] < y_max && xyz[2] < z_max && xyz[0] >= 0 && xyz[1] >= 0 && xyz[2] >= 0) {
		return true;
	}

	return false;
}

vec3 VoxelGrid::world2voxel(vec3 world)
{
	vec3 grid_root(0.0, 0.0, 0.0);
	vec3 ijk = (world - grid_root)/voxelSize; // component-wise division
	ijk = floor(ijk);
	return ijk;
}

vec3 VoxelGrid::voxel2world(vec3 ijk)
{
	vec3 grid_root(0.0, 0.0, 0.0);
	vec3 world = ijk*voxelSize + grid_root;
	return world;
}