#ifndef RAYMARCH_H_
#define RAYMARCH_H_

#include "algebra3.h"
#include "VoxelGrid.h"

double RayCubeSlabsIntersection(vec3 const& ro, vec3 const& rd, mat4 const& t_inv, mat4 const& tstar_inv);

void MarchRay(vec3 orig, vec3 dir, VoxelGrid *grid, double step, vector<vec3> &points)
{
	double t = RayCubeSlabsIntersection(orig, dir, grid->getTInv(), grid->getTStarInv());

	// Check if ray hits the grid
	if (t < EPSILON) {
		points.push_back(vec3(-1, -1, -1));
		return;
	}

	// If ray is outside of grid
	if (!grid->isInsideGrid(orig)) {
		orig = orig + (t + EPSILON)*dir;
	}

	// Ray is in the grid
	while (grid->isInsideGrid(orig)) {
		points.push_back(orig);
		orig = orig + step*dir;
	}
}

void MarchRayVoxel(vec3 orig, vec3 dir, VoxelGrid *grid, vector<vec3> &points)
{
	double t = RayCubeSlabsIntersection(orig, dir, grid->getTInv(), grid->getTStarInv());

	// Check if ray hits the grid
	if (t < EPSILON) {
		points.push_back(vec3(-1, -1, -1));
		return;
	}

	vec3 start, end;

	// Get entering and exiting points
	if (!grid->isInsideGrid(orig)) {
		start = orig + (t + EPSILON)*dir;
		end = orig + (RayCubeSlabsIntersection(start, dir, grid->getTInv(), grid->getTStarInv()) + EPSILON)*dir;
	}
	else {
		start = orig;
		end = orig + (t + EPSILON)*dir;
	}

	vec3 point = start;

	double voxelSize = grid->getVoxelSize();
		
	// Ratio to decrease number of division
	double dirX, dirY, dirZ;
	if (dir[0] != 0) dirX = 1/dir[0];
	if (dir[1] != 0) dirY = 1/dir[1];
	if (dir[2] != 0) dirZ = 1/dir[2];

	// Get root position of current voxel in world coordinate
	vec3 voxel = grid->world2voxel(point);
	vec3 voxel0 = grid->voxel2world(voxel);
	vec3 voxel1 = grid->voxel2world(voxel + vec3(1, 1, 1));

	// Distance to reach the next voxel in x, y, z direction
	double tMaxX = 1000000;	double tMaxY = 1000000;	double tMaxZ = 1000000;
	if (dir[0] < 0.0) tMaxX = (voxel0[0] - point[0])*dirX;
	if (dir[0] > 0.0) tMaxX = (voxel1[0] - point[0])*dirX;
	if (dir[1] < 0.0) tMaxY = (voxel0[1] - point[1])*dirY;
	if (dir[1] > 0.0) tMaxY = (voxel1[1] - point[1])*dirY;
	if (dir[2] < 0.0) tMaxZ = (voxel0[2] - point[2])*dirZ;
	if (dir[2] > 0.0) tMaxZ = (voxel1[2] - point[2])*dirZ;

	// Step size in x, y, z direction
	double stepX = (dir[0] < 0.0) ? -voxelSize : voxelSize;
	double stepY = (dir[1] < 0.0) ? -voxelSize : voxelSize;
	double stepZ = (dir[2] < 0.0) ? -voxelSize : voxelSize;

	// Distance to move one voxel width in x, y, z direction
	double tDeltaX = voxelSize*fabs(dirX);
	double tDeltaY = voxelSize*fabs(dirY);
	double tDeltaZ = voxelSize*fabs(dirZ);
	
	do { // Take steps in the smallest of x, y, z at each iteration until out of voxel space
		points.push_back(point);

		if (tMaxX < tMaxY) {
			if (tMaxX < tMaxZ) {
				point[0] = point[0] + stepX;
				tMaxX = tMaxX + tDeltaX;
			}
			else {
				point[2] = point[2] + stepZ;
				tMaxZ = tMaxZ + tDeltaZ;
			}
		}
		else {
			if (tMaxY < tMaxZ) {
				point[1] = point[1] + stepY;
				tMaxY = tMaxY + tDeltaY;
			}
			else {
				point[2] = point[2] + stepZ;
				tMaxZ = tMaxZ + tDeltaZ;
			}
		}
	} while ((grid->isInsideGrid(point)));
}

void MarchRayVoxelBresenham(vec3 orig, vec3 dir, VoxelGrid *grid, vector<vec3> &points)
{
	double t = RayCubeSlabsIntersection(orig, dir, grid->getTInv(), grid->getTStarInv());

	// Check if ray hits the grid
	if (t < EPSILON) {
		points.push_back(vec3(-1, -1, -1));
		return;
	}

	vec3 start, end;

	// Get entering and exiting points
	if (!grid->isInsideGrid(orig)) {
		start = orig + (t + EPSILON)*dir;
		end = orig + (RayCubeSlabsIntersection(start, dir, grid->getTInv(), grid->getTStarInv()) + EPSILON)*dir;
	}
	else {
		start = orig;
		end = orig + (t + EPSILON)*dir;
	}

	vec3 point = start;
	double err_1, err_2;

	// Delta x, y, z
	double dx = end[0] - start[0];
	double dy = end[1] - start[1];
	double dz = end[2] - start[2];

	double inc = grid->getVoxelSize(); // Voxel size
	double x_inc = (dx < 0) ? -inc : inc; // Get x direction
	double y_inc = (dy < 0) ? -inc : inc; // Get y direction
	double z_inc = (dz < 0) ? -inc : inc; // Get z direction

	// Get total x, y, z, length to travel
	double x_length = fabs(dx);
	double y_length = fabs(dy);
	double z_length = fabs(dz);

	// Multiply by 2 from original integral algorithm
    double dx2 = dx*2;
    double dy2 = dy*2;
    double dz2 = dz*2;

	// If x distance is largest
    if ((x_length >= y_length) && (x_length >= z_length)) {
        err_1 = dy2 - x_length;
        err_2 = dz2 - x_length;
		// Step through total x length one voxel at a time
        for (double ii = 0; ii < x_length; ii += inc) {
			points.push_back(point);
            if (err_1 > 0) {
                point[1] += y_inc;
                err_1 -= dx2;
            }
            if (err_2 > 0) {
                point[2] += z_inc;
                err_2 -= dx2;
            }
            err_1 += dy2;
            err_2 += dz2;
            point[0] += x_inc;
        }
    }
	// If y distance is largest
	else if ((y_length >= x_length) && (y_length >= z_length)) {
        err_1 = dx2 - y_length;
        err_2 = dz2 - y_length;
		// Step through total y length one voxel at a time
        for (double ii = 0; ii < y_length; ii += inc) {
			points.push_back(point);
            if (err_1 > 0) {
                point[0] += x_inc;
                err_1 -= dy2;
            }
            if (err_2 > 0) {
                point[2] += z_inc;
                err_2 -= dy2;
            }
            err_1 += dx2;
            err_2 += dz2;
            point[1] += y_inc;
        }
    }
	// If z distance is largest
	else {
        err_1 = dy2 - z_length;
        err_2 = dx2 - z_length;
		// Step through total z length one voxel at a time
        for (double ii = 0; ii < z_length; ii += inc) {
			points.push_back(point);
            if (err_1 > 0) {
                point[1] += y_inc;
                err_1 -= dz2;
            }
            if (err_2 > 0) {
                point[0] += x_inc;
                err_2 -= dz2;
            }
            err_1 += dy2;
            err_2 += dx2;
            point[2] += z_inc;
        }
    }
	// Get last point
	points.push_back(point);
}

double RayCubeSlabsIntersection(vec3 const& ro, vec3 const& rd, mat4 const& t_inv, mat4 const& tstar_inv)
{
	vec3 o = t_inv*ro;
	vec3 d = tstar_inv*rd;
	vec3 dn = d.normalized();
	
	double t = -1.0;

	double t_min = -100000;
	double t_max = 100000;
	int nside; // To check which side the normal is pointing out of

	vec3 ac = vec3(0, 0, 0);
	vec3 p = ac - o;

	for (int ii = 0; ii < 3; ii++) {
		double e = p[ii];
		double f = dn[ii];
		// Check if ray direction is perpendicular to the normal direction of the slab
		if (fabs(f) > EPSILON) {
			double fdiv = 1/f; // Division optimization
			double t1 = (e + 0.5)*fdiv;
			double t2 = (e - 0.5)*fdiv;
			// Ensures the minimum of t1 and t2 is stored in t1
			if (t1 > t2) swap(t1, t2);
			if (t1 > t_min) { t_min = t1; nside = (ii + 1); }
			if (t2 < t_max) { t_max = t2; }
			if (t_min > t_max || t_max < 0) return t; // Ray misses the box or box is behind ray
		}
		// If ray is parallel to the slab
		else if (-e - 0.5 > 0 || -e + 0.5 < 0) return t;
	}
	
	if (t_min > EPSILON) { t = t_min/d.length(); nside *= 1; }
	else { t = t_max/d.length(); nside *= -1; }
	
	// Get the normal in world coordinate
	vec3 n3o(0, 0, 0);
	switch (nside) {
		case  1: n3o = vec3( 1, 0, 0); break;
		case -1: n3o = vec3(-1, 0, 0); break;
		case  2: n3o = vec3(0,  1, 0); break;
		case -2: n3o = vec3(0, -1, 0); break;
		case  3: n3o = vec3(0, 0,  1); break;
		case -3: n3o = vec3(0, 0, -1); break;
		default: break;
	}

	return t;
}

#endif