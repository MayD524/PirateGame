#include "water_plane.h"
#include <math.h>


float Noise2D(float x, float y) {
    int n = (int)x + (int)y * 57;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float SmoothNoise2D(float x, float y) {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxAmplitude = 0.0f; // For normalization
    int octaves = 4;          // Number of noise layers

    for (int i = 0; i < octaves; i++) {
        total += Noise2D(x * frequency, y * frequency) * amplitude;
        maxAmplitude += amplitude;
        frequency *= 2.0f; // Increase frequency
        amplitude *= 0.5f; // Decrease amplitude
    }

    return total / maxAmplitude; // Normalize to range [-1, 1]
}

// Create a water plane
WaterPlane CreateWaterPlane(Vector3 position, Vector2 size, int rows, int cols, Color color, float waveSpeed, float waveHeight) {
    WaterPlane water = { position, size, rows, cols, waveSpeed, waveHeight, 0.0f, color, NULL };
    water.vertices = (Vector3 *)malloc(rows * cols * sizeof(Vector3));

    float dx = size.x / (cols - 1);
    float dz = size.y / (rows - 1);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            water.vertices[i * cols + j] = (Vector3){ position.x + j * dx, position.y, position.z + i * dz };
        }
    }

    return water;
}

// Update the water plane (simulate waves)
void UpdateWaterPlane(WaterPlane *waterPlane, float deltaTime) {
    waterPlane->time += deltaTime * waterPlane->waveSpeed;

    for (int i = 0; i < waterPlane->rows; i++) {
        for (int j = 0; j < waterPlane->cols; j++) {
            int index = i * waterPlane->cols + j;
            float noiseValue = SmoothNoise2D(
                (float)j * 0.1f, 
                (float)i * 0.1f + waterPlane->time
            );
            waterPlane->vertices[index].y = waterPlane->position.y + noiseValue * waterPlane->waveHeight;
        }
    }
}

float CClamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

Color CalculateVertexColor(WaterPlane* waterPlane, Vector3 v) {
    float highlightThreshold = 0.2f; // Height at which highlights begin
    float maxHeight = 0.5f;          // Maximum height for highlights

    float relativeHeight = CClamp((v.y - waterPlane->position.y - highlightThreshold) / (maxHeight - highlightThreshold), 0.0f, 1.0f);
    unsigned char blue = 255 - (unsigned char)(relativeHeight * 100);  // Base blue gradient
    unsigned char green = (unsigned char)(relativeHeight * 200);      // Add green highlight
    unsigned char alpha = 200 + (unsigned char)(relativeHeight * 55); // Add transparency
    return (Color){ 0, green, blue, alpha };
}

bool IsPositionInsidePlane(WaterPlane *waterPlane, float x, float y, float z) {
    // Calculate the actual bounds of the plane based on its position
    float minX = waterPlane->position.x;                   // Left edge
    float maxX = waterPlane->position.x + waterPlane->size.x; // Right edge
    float minZ = waterPlane->position.z;                   // Bottom edge
    float maxZ = waterPlane->position.z + waterPlane->size.y; // Top edge

    // Define vertical bounds for the wave motion
    float minHeight = waterPlane->position.y - waterPlane->waveHeight; // Base minus wave height
    float maxHeight = waterPlane->position.y + waterPlane->waveHeight; // Base plus wave height

    // Check if (x, y, z) is inside the plane's horizontal and vertical bounds
    return (x >= minX && x <= maxX &&
            z >= minZ && z <= maxZ &&
            y >= minHeight && y <= maxHeight);
}

float GetWaveHeight(WaterPlane *waterPlane, float x, float z) {
    // Check if the position (x, y, z) is inside the water plane bounds
    if (!IsPositionInsidePlane(waterPlane, x, waterPlane->position.y+waterPlane->waveHeight, z)) {
        return -1.0f; // Or any other value to indicate out of bounds
    }

    float waveSpeed = waterPlane->waveSpeed;
    float waveHeight = waterPlane->waveHeight;
    float time = waterPlane->time;

    // Compute the wave height using noise or a sine wave
    float noiseValue = SmoothNoise2D(x * 0.1f, z * 0.1f + time);
    return waterPlane->position.y + noiseValue * waveHeight;
}

void GetWaveHeights(WaterPlane *waterPlane, Vector3 *points, int pointCount, float *heights) {
    for (int i = 0; i < pointCount; i++) {
        heights[i] = GetWaveHeight(waterPlane, points[i].x, points[i].z);
    }
}

// Draw the water plane
void DrawWaterPlane(WaterPlane *waterPlane) {
    int rows = waterPlane->rows;
    int cols = waterPlane->cols;
    int totalCells = (rows - 1) * (cols - 1); // Total number of grid cells to process

    for (int index = 0; index < totalCells; index++) {
        // Calculate row and column from the linear index
        int i = index / (cols - 1); // Current row
        int j = index % (cols - 1); // Current column

        // Fetch vertices for the current cell
        int rowStart = i * cols;
        int nextRowStart = (i + 1) * cols;

        Vector3 v1 = waterPlane->vertices[rowStart + j];
        Vector3 v2 = waterPlane->vertices[rowStart + j + 1];
        Vector3 v3 = waterPlane->vertices[nextRowStart + j];
        Vector3 v4 = waterPlane->vertices[nextRowStart + j + 1];

        // Calculate colors for each vertex
        Color c1 = CalculateVertexColor(waterPlane, v1);
        Color c2 = CalculateVertexColor(waterPlane, v2);
        Color c3 = CalculateVertexColor(waterPlane, v3);
        Color c4 = CalculateVertexColor(waterPlane, v4);

        // Draw the two triangles for the current cell
        DrawTriangle3D(v1, v3, v2, c1);
        DrawTriangle3D(v2, v3, v4, c2);
    }
}

// Free WaterPlane resources
void DestroyWaterPlane(WaterPlane *waterPlane) {
    free(waterPlane->vertices);
    waterPlane->vertices = NULL;
}
