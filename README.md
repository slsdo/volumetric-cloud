Volumetric Clouds
=================

Originally created in 2009, this program first reads a text file containing initialization variables, then uses those variables to create a voxel grid data structure and uses ray marching to read lighting and density information from the grid in order to render fluffy clouds. The program was updated in 2010: the code was completely rewritten to improve efficiency, several new algorithms were implemented for voxel grid traversal, Perlin Noise was implemented to generate random clouds, and a GUI was created to make it more user-friendly (for the few users that actually stumble across it, that is).

Features
--------

- Generates volumetric clouds with ray marching on a voxel density grid
- Trilinear interpolation to produce more realistic results
- Random cloud generator using Perlin noise
- Can read in custom-made cloud configuration files or output current cloud as a new configuration file
- GUI that allows the user to move the camera and see rendering progress
- Optimized for faster speed