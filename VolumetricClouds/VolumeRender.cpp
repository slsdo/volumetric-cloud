#include "VolumeRender.h"

#include "ProgressBar.h"
#include "RayMarch.h"

VolumeRender::VolumeRender()
{
	vGrid = new VoxelGrid();
	rcGrid = new VoxelGrid();
	gcGrid = new VoxelGrid();
	bcGrid = new VoxelGrid();

	// Set output image resolution and step size
	reso_x = 640.0;
	reso_y = 480.0;
	step = 0.1;
	kapa = 1.0;
}

VolumeRender::~VolumeRender()
{
	delete vGrid;
	delete rcGrid;
	delete gcGrid;
	delete bcGrid;
}

void VolumeRender::initLighting(vec3 lp, vec3 lc, vec3 bc, int qt, int bg)
{
	lpos = lp; // Point light
	lcol = lc; // Point light color
	bcol = bc; // Background color
	quality = qt; // Render quality
	background = bg; // Background color

	computeLightTransmittance(); // Pre-compute light transmittance
}

void VolumeRender::initCamera(vec3 ep, vec3 vd, vec3 uv, double fy)
{
	eyep = ep; // Eye position
	vdir = vd; // Viewing direction
	uvec = uv; // World space up vector
	fovy = fy; // Total field of view in Y
	
	vec3 a = vdir^uvec; // Be sure this is well defined
	vec3 b = vdir^a; // B is now in the correct plane
	midpoint = eyep + vdir; // Now have midpoint of screen
	h_px = (double)(reso_x/reso_y)*vdir.length()*tan((fovy*0.5)*Deg2Rad)*a.normalized(); // Rescale A to H
	v_px = vdir.length()*tan((fovy*0.5)*Deg2Rad)*b.normalized(); // Rescale B to V
}

void VolumeRender::generateVoxelGridRandom(int seed)
{
	VoxelGrid *grid = new VoxelGrid(0.1, 50, 50, 50, 0);
	Noise *noise = new Noise(0.5, 5, seed); // Seed = 188
	
	// Get max length from grid center to grid surface
	vec3 center(grid->getMaxX()*0.5, grid->getMaxY()*0.5, grid->getMaxZ()*0.5);
	double max_distance = (vec3(0 + 0.5, 0 + 0.5, 0 + 0.5) - center).length();
	double max_ratio = 1/max_distance;

	// Generate random density
	for (int kk = 0; kk < grid->getMaxZ(); kk++) {
		for (int jj = 0; jj < grid->getMaxY(); jj++) {
			for (int ii = 0; ii < grid->getMaxX(); ii++) {
				double cloud = noise->PerlinNoise3(ii*grid->getVoxelSize(), jj*grid->getVoxelSize(), kk*grid->getVoxelSize());
				
				vec3 voxel(vec3(ii + 0.5, jj + 0.5, kk + 0.5));
				double distance = (voxel - center).length(); // Distance from current voxel to grid center
				double cover = distance*max_ratio + 0.3; // Amount of cloud (0.93)
				double sharpness = 0.5; // Cloud fuzziness and sharpness
				double density = 5; // Cloud density

				cloud = cloud - cover;
				if (cloud < 0) cloud = 0; cloud = cloud*density;
				cloud = 1.0 - powf(sharpness, cloud);

				grid->setVoxelDensity(ii, jj, kk, cloud);

				vec3 color(1, 1, 1);
				grid->setVoxelColor(ii, jj, kk, color);
			}
		}
		// Render progress 0%-99%
		renderProgress(0, ((double)kk/(grid->getMaxZ() - 1))*100.0);
	}

	vGrid = grid; // Set voxel grid
	// Render progress 100%
	for (int ii = 0; ii < 100; ii++ ) { renderProgress(0, 100); }
}

void VolumeRender::generateVoxelGridFromFile(string filename)
{
	// Render progress 0%-60%
	renderProgress(2, 0);

	vGrid = new VoxelGrid(filename);
	// Render progress 60%-99%
	renderProgress(2, 80);

	for (int kk = 0; kk < vGrid->getMaxZ(); kk++) {
		for (int jj = 0; jj < vGrid->getMaxY(); jj++) {
			for (int ii = 0; ii < vGrid->getMaxX(); ii++) {
				vec3 color(1, 1, 1);
				vGrid->setVoxelColor(ii, jj, kk, color);
			}
		}
	}
	// Render progress 100%
	for (int ii = 0; ii < 100; ii++ ) { renderProgress(2, 100); }
}

void VolumeRender::generateVoxelGridRed(string filename)
{
	// Render progress 0%-60%
	renderProgress(2, 0);
	
	rcGrid = new VoxelGrid(filename);
	// Render progress 60%-99%
	renderProgress(2, 80);

	for (int kk = 0; kk < vGrid->getMaxZ(); kk++) {
		for (int jj = 0; jj < vGrid->getMaxY(); jj++) {
			for (int ii = 0; ii < vGrid->getMaxX(); ii++) {
				vec3 rc = rcGrid->voxel2world(vec3(ii, jj, kk));
				vec3 rgb(rcGrid->getVoxelDensity(rc[0], rc[1], rc[2]),
						 vGrid->getVoxelColor(ii, jj, kk)[1],
						 vGrid->getVoxelColor(ii, jj, kk)[2]);
				vGrid->setVoxelColor(ii, jj, kk, rgb);
			}
		}
	}
	// Render progress 100%
	for (int ii = 0; ii < 100; ii++ ) { renderProgress(2, 100); }
}

void VolumeRender::generateVoxelGridGreen(string filename)
{
	// Render progress 0%-60%
	renderProgress(2, 0);
	
	gcGrid = new VoxelGrid(filename);
	// Render progress 60%-99%
	renderProgress(2, 80);

	for (int kk = 0; kk < vGrid->getMaxZ(); kk++) {
		for (int jj = 0; jj < vGrid->getMaxY(); jj++) {
			for (int ii = 0; ii < vGrid->getMaxX(); ii++) {
				vec3 gc = gcGrid->voxel2world(vec3(ii, jj, kk));
				vec3 rgb(vGrid->getVoxelColor(ii, jj, kk)[0],
						 gcGrid->getVoxelDensity(gc[0], gc[1], gc[2]),
						 vGrid->getVoxelColor(ii, jj, kk)[2]);
				vGrid->setVoxelColor(ii, jj, kk, rgb);
			}
		}
	}
	// Render progress 100%
	for (int ii = 0; ii < 100; ii++ ) { renderProgress(2, 100); }
}

void VolumeRender::generateVoxelGridBlue(string filename)
{
	// Render progress 0%
	renderProgress(2, 0);
	
	bcGrid = new VoxelGrid(filename);
	// Render progress 80%
	renderProgress(2, 80);

	for (int kk = 0; kk < vGrid->getMaxZ(); kk++) {
		for (int jj = 0; jj < vGrid->getMaxY(); jj++) {
			for (int ii = 0; ii < vGrid->getMaxX(); ii++) {
				vec3 bc = bcGrid->voxel2world(vec3(ii, jj, kk));
				vec3 rgb(vGrid->getVoxelColor(ii, jj, kk)[0],
						 vGrid->getVoxelColor(ii, jj, kk)[1],
						 bcGrid->getVoxelDensity(bc[0], bc[1], bc[2]));
				vGrid->setVoxelColor(ii, jj, kk, rgb);
			}
		}
	}
	// Render progress 100%
	for (int ii = 0; ii < 100; ii++ ) { renderProgress(2, 100); }
}

void VolumeRender::computeLightTransmittance()
{
	for (int kk = 0; kk < vGrid->getMaxZ(); kk++) {
		for (int jj = 0; jj < vGrid->getMaxY(); jj++) {
			for (int ii = 0; ii < vGrid->getMaxX(); ii++) {
				// Get the vectors from center of each voxel to light
				vec3 orig(vGrid->voxel2world(vec3(ii + 0.5, jj + 0.5, kk + 0.5)));
				vec3 dir = (lpos - orig).normalized();

				vector<vec3> points;
				
				switch (quality) {
					case 0: MarchRay(orig, dir, vGrid, step, points); break; // Step-step no interp
					case 1:	MarchRay(orig, dir, vGrid, step, points); break; // Step-voxel no interp
					case 2: MarchRayVoxel(orig, dir, vGrid, points); break; // Voxel-voxel no interp
					case 3: MarchRay(orig, dir, vGrid, step, points); break; // Step-step interp
					case 4:	MarchRayVoxel(orig, dir, vGrid, points); break; // Voxel-step interp
				}
				
				double transmittance = 1.0;
				for (int n = 0; n < points.size(); n++) {
					double rho = vGrid->getVoxelDensity(points[n]);
					if (rho > 0) {
						transmittance *= exp(-kapa*step*rho);
					}
				}

				// Set transmittance values
				vGrid->setVoxelTransmissivity(ii, jj, kk, transmittance);
			}
		}
		// Render progress 0%-99%
		renderProgress(3, ((double)kk/(vGrid->getMaxZ() - 1))*100.0);
	}
	// Render progress 100%
	for (int ii = 0; ii < 100; ii++ ) { renderProgress(3, 100); }
}

void VolumeRender::render()
{
	int size = 3*reso_x*reso_y;
	unsigned char *output = new unsigned char[size]; // Output file
	char *outfile = new char();	outfile = "output.bmp"; // Output filename
	
	// Set background color
	vec3 bg_gradient;
	double bg_ratio = 1.0/480.0;
	switch (background) {
		case 0: bg_gradient = vec3(0, 0, 0); break; // Light blue
		case 1:	bg_gradient = vec3(0.3, 0.4, 0.2); break; // Blue gradient
	}

	// Generate rays for every pixel on screen
	for (int y = 0; y < reso_y; y++) {
		for (int x = 0; x < reso_x; x++) {
			// Any point in NDC is (Sx, Sy)
			vec3 orig = midpoint + (2.0*(double)(x/(reso_x - 1)) - 1)*h_px + (2.0*(double)(y/(reso_y - 1)) - 1)*v_px;
			vec3 dir = (orig - eyep).normalize();
			vec3 color(0.0, 0.0, 0.0);

			vector<vec3> points;

			// March, my little rays, MARCH!
			switch (quality) {
				case 0: MarchRay(orig, dir, vGrid, step, points); break; // Step-step no interp
				case 1:	MarchRayVoxel(orig, dir, vGrid, points); break; // Step-voxel no interp
				case 2: MarchRayVoxel(orig, dir, vGrid, points); break; // Voxel-voxel no interp
				case 3: MarchRay(orig, dir, vGrid, step, points); break; // Step-step interp
				case 4:	MarchRay(orig, dir, vGrid, step, points); break; // Voxel-step interp
			}

			// Volumetric rendering
			if (points.size() > 1) {
				double transmittance = 1;
				for (int n = 0; n < points.size(); n++) {
					double rho; // Density
					if (quality < 3) rho = vGrid->getVoxelDensity(points[n]);
					else rho = vGrid->interpVoxelDensity(points[n]);

					if (rho > 0) {
						vec3 rgb; // Color
						if (quality < 3) rgb = prod(vGrid->getVoxelColor(points[n]), lcol);
						else rgb = prod(vGrid->interpVoxelColor(points[n]), lcol);
						
						double deltaT = exp(-kapa*step*rho);
						transmittance *= deltaT;

						// When transmittance becomes close to zero
						if (transmittance < 1e-6)
							break;
						
						double q; // Transmittance
						if (quality < 3) q = vGrid->getVoxelTransmissivity(points[n]);
						else q = vGrid->interpVoxelTransmissivity(points[n]);

						// Rendering equation
						color += (1 - deltaT)/kapa*rgb*transmittance*q;
					}
				}

				// Fills the rest with background color
				color += bcol*transmittance;
			}
			else {
				color = bcol;
			}
	
			// Check if any color goes over 1 (255)
			if(color[0] > 1) color[0] = 1;
			if(color[1] > 1) color[1] = 1;
			if(color[2] > 1) color[2] = 1;

			// Stores the color
			int index = (reso_x*y + x)*3;
			output[index + 0] = color[0]*255.0;
			output[index + 1] = color[1]*255.0;
			output[index + 2] = color[2]*255.0;
		}
		// Render progress 0%-99%
		if (y%10 == 0) { renderProgress(4, (y/reso_y)*100.0); }
		
		bcol += bg_gradient*bg_ratio; // Update background
	}
	// Render progress 100%
	for (int ii = 0; ii < 100; ii++ ) { renderProgress(4, 100); }

	// Saves the data to a BMP image
	SOIL_save_image(outfile, SOIL_SAVE_TYPE_BMP, reso_x, reso_y, 3, output);

	delete output;
}

void VolumeRender::setCloudColor(int rgb, double value)
{
	for (int kk = 0; kk < vGrid->getMaxZ(); kk++) {
		for (int jj = 0; jj < vGrid->getMaxY(); jj++) {
			for (int ii = 0; ii < vGrid->getMaxX(); ii++) {
				vec3 col = vGrid->getVoxelColor(ii, jj, kk);
				col[rgb] += value;
				
				// Check if any color goes below or over 0/1 (0/255)
				if(col[0] < 0) col[0] = 0; if(col[1] < 0) col[1] = 0; if(col[2] < 0) col[2] = 0;
				if(col[0] > 1) col[0] = 1; if(col[1] > 1) col[1] = 1; if(col[2] > 1) col[2] = 1;
				vGrid->setVoxelColor(ii, jj, kk, col);
			}
		}
	}
}

void VolumeRender::outputCloud()
{
	// Output to file
	ofstream outfile("output.txt");
	outfile << vGrid->getVoxelSize() << endl;
	outfile << vGrid->getMaxX() << " "<< vGrid->getMaxY() << " " << vGrid->getMaxZ() << endl;
	outfile << vGrid->getDefaultDensity() << endl;
			
	for (int kk = 0; kk < vGrid->getMaxZ(); kk++) {
		for (int jj = 0; jj < vGrid->getMaxY(); jj++) {
			for (int ii = 0; ii < vGrid->getMaxX(); ii++) {
				outfile << vGrid->getVoxelDensity(ii, jj, kk) << endl;
			}
		}
		// Render progress 0%-99%
		renderProgress(1, ((double)kk/(vGrid->getMaxZ() - 1))*100.0);
	}

	outfile.close(); // Close file
	// Render progress 100%
	for (int ii = 0; ii < 100; ii++ ) { renderProgress(1, 100); }
}

VoxelGrid* VolumeRender::getGrid()
{
	return vGrid;
}