// water_plane.h
#ifndef WATER_PLANE_H
#define WATER_PLANE_H

#include <simd_compat.h> // For AVX intrinsics
#include "raylib.h"
#include <rlgl.h>
#include <raymath.h>
#include <stdlib.h>

#ifndef NO_OMP
#include <omp.h>
#endif

#define WATER_UPDATE_RATE 3

typedef struct {
    Vector3 position;   // Origin position of the water plane
    Vector2 size;       // Size of the water plane (width, length)
    int rows;           // Number of rows in the plane (grid resolution)
    int cols;           // Number of columns in the plane (grid resolution)
    float waveSpeed;    // Speed of the wave movement
    float waveHeight;   // Height of the waves
    float time;         // Internal time tracker for wave movement
    Color color;        // Color of the water
    Vector3 *vertices;  // Dynamically allocated vertices of the water plane
    int update_ticks;
} WaterPlane;

typedef struct {
    Vector3 startPosition;
    Vector2 gridSize;

    int rows;
    int cols;
    
    Color color;

    float waveSpeed;
    float waveHeight;
} WaterPlaneCreationInfo;

// Initialize a WaterPlane
WaterPlane CreateWaterPlane(Vector3 position, Vector2 size, int rows, int cols, Color color, float waveSpeed, float waveHeight);
WaterPlane CreateWaterPlane_info(WaterPlaneCreationInfo info);

// Update the WaterPlane (handles wave animation)
void UpdateWaterPlane(WaterPlane *waterPlane, float deltaTime);

// Draw the WaterPlane
void DrawWaterPlane_SIMD(WaterPlane *waterPlane, Vector3 cameraPosition, float renderDistance);
void DrawWaterPlane(WaterPlane *waterPlane, Vector3 cameraPosition, float renderDistance);

// Free WaterPlane resources
void DestroyWaterPlane(WaterPlane *waterPlane);

void GetWaveHeights(WaterPlane *waterPlane, Vector3 *points, int pointCount, float *heights);
float GetWaveHeight(WaterPlane *waterPlane, float x, float z);

#endif
