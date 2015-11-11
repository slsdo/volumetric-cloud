#ifndef VOXELGRID_H
#define VOXELGRID_H

#include <vector>
#include <fstream>
#include <sstream>

#include "algebra3.h"

struct Voxel {
	vec3 color;
	double density;
	double transmissivity;
};

class VoxelGrid {
public:
	// Constructors and destructor
	VoxelGrid();
	VoxelGrid(double size, int x, int y, int z, double d);
	VoxelGrid(string filename);
	~VoxelGrid();
	
	void setVoxelColor(int x, int y, int z, vec3 rgb); // Set color of voxel given grid coordinate
	void setVoxelColor(vec3 xyz, vec3 rgb); // Set color of voxel given x, y, z coordinate
	void setVoxelDensity(int x, int y, int z, double q); // Set density of voxel given grid coordinate
	void setVoxelDensity(vec3 xyz, double q); // Set density of voxel given x, y, z coordinate
	void setVoxelTransmissivity(int x, int y, int z, double q); // Set transmittance of voxel given grid coordinate
	void setVoxelTransmissivity(vec3 xyz, double q); // Set transmittance of voxel given x, y, z coordinate
		
	vec3 getVoxelColor(int x, int y, int z); // Get color of voxel given grid coordinate
	vec3 getVoxelColor(vec3 xyz); // Get color of voxel given x, y, z coordinate
	double getVoxelDensity(int x, int y, int z); // Get density of voxel given grid coordinate
	double getVoxelDensity(vec3 xyz); // Get density of voxel given x, y, z coordinate
	double getVoxelTransmissivity(int x, int y, int z); // Get transmittance of voxel given grid coordinate
	double getVoxelTransmissivity(vec3 xyz); // Get transmittance of voxel given x, y, z coordinate
	
	vec3 interpVoxelColor(double x, double y, double z); // Interpolate color of voxel given grid coordinate
	vec3 interpVoxelColor(vec3 xyz); // Interpolate color of voxel given x, y, z coordinate
	double interpVoxelDensity(double x, double y, double z); // Interpolate density using tri-linear interpolation
	double interpVoxelDensity(vec3 xyz); // Interpolate density using tri-linear interpolation
	double interpVoxelTransmissivity(double x, double y, double z); // Interpolate transmittance using tri-linear interpolation
	double interpVoxelTransmissivity(vec3 xyz); // Interpolate transmittance using tri-linear interpolation
	
	int getVoxelIndex(int x, int y, int z); // Get array index of voxel given grid index
	vector<Voxel*> getVoxelGrid(); // Get voxel grid
	double getVoxelSize(); // Get voxel size
	double getDefaultDensity();	// Get default density
	int getMaxX(); int getMaxY(); int getMaxZ(); // Get X/Y/Z count
	mat4 getTInv(); mat4 getTStarInv(); // Get transformation matrices for intersection test

	bool isInsideGrid(vec3 xyz); // Check if point is inside the voxel grid
	vec3 world2voxel(vec3 world); // World coordinate to voxel coordinate
	vec3 voxel2world(vec3 voxel); // Voxel coordinate to world coordinate

private:
	double voxelSize;
	int xCount, yCount, zCount;
	double defaultDensity;
	vector<Voxel*> voxelGrid;

	mat4 t_inv;
	mat4 tstar_inv;

	friend class VolumeRender;
};

#endif