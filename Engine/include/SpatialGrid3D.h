#pragma once

#ifndef SPATICALGRID3D_H
#define SPATICALGRID3D_H

#include <raylib.h>
#include <raymath.h>
#include <extended_memory.h>
#include <util.h>


typedef struct GridCell3D {
    int *entityIndices;  // Dynamic array of entity indices
    int count;           // How many indices are currently stored
    int capacity;        // Max number before a resize is needed
    int cell_id;
} GridCell3D;

/**
 * The 3D uniform grid itself.
 */
typedef struct SpatialGrid3D {
    GridCell3D *cells;   // A 1D array of grid cells, sized cellsX * cellsY * cellsZ

    int cellsX;          // Number of cells along the X-axis
    int cellsY;          // Number of cells along the Y-axis
    int cellsZ;          // Number of cells along the Z-axis
    
    float cellSize;      // The width/height/depth of each cell in world units
    Vector3 origin;      // The "bottom-left-front" corner of the grid in world space
} SpatialGrid3D;


bool init_spatial_grid_3d(
    SpatialGrid3D *grid,
    int cellsX,
    int cellsY,
    int cellsZ,
    float cellSize,
    Vector3 origin,
    int defaultCapacity
);

int get_cell_index_3d(const SpatialGrid3D *grid, float x, float y, float z);
void free_spatial_grid_3d(SpatialGrid3D *grid);


#endif