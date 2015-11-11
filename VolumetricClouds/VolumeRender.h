#ifndef VOLUMERENDER_H
#define VOLUMERENDER_H

#include <fstream>

#include "VoxelGrid.h"
#include "noise.h"

#ifdef __APPLE__
#include <GLUT/GLUT.h>
#else
#include <GL/glut.h>
#endif
#include <SOIL/SOIL.h>

class VolumeRender {
public:
	// Constructor and destructor
	VolumeRender();
	~VolumeRender();
	
	void initLighting(vec3 lpos, vec3 lcol, vec3 bcol, int quality, int bg); // Set up the lights
	void initCamera(vec3 eyep, vec3 vdir, vec3 uvec, double fovy); // Set up the camera for rendering

	void generateVoxelGridRandom(int seed); // Generate a voxel grid with random clouds
	void generateVoxelGridFromFile(string filename); // Read input from configuration files
	
	void generateVoxelGridRed(string filename); // Set red component of cloud color
	void generateVoxelGridGreen(string filename); // Set green component of cloud color
	void generateVoxelGridBlue(string filename); // Set blue component of cloud color

	void computeLightTransmittance();
	void render();

	void setCloudColor(int rgb, double value);
	void outputCloud();
	VoxelGrid* getGrid();

private:
	VoxelGrid *vGrid;

	VoxelGrid *rcGrid;
	VoxelGrid *gcGrid;
	VoxelGrid *bcGrid;

	double reso_x, reso_y; // Screen resolution
	vec3 midpoint, h_px, v_px; // Midpoint, width and height of screen in terms of pixel
	vec3 eyep; // Eye position
	vec3 vdir; // Viewing direction
	vec3 uvec; // World space up vector
	double fovy; // Total field of view in Y
	vec3 lpos; // Light position
	vec3 lcol; // Light color
	vec3 bcol; // Background color
	double step; // Step size
	double kapa; // Kapa for light computation

	int quality; // Quality of render
	int background; // Background color
};

#endif