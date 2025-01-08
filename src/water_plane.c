#include "water_plane.h"
#include <math.h>


static inline float Noise2D(float x, float y) {
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

WaterPlane CreateWaterPlane_info(WaterPlaneCreationInfo info) {
    return CreateWaterPlane(info.startPosition, info.gridSize, info.rows, info.cols, info.color, info.waveSpeed, info.waveHeight);
}

// Update the water plane (simulate waves)
void UpdateWaterPlane(WaterPlane *waterPlane, float deltaTime) {
    waterPlane->time += deltaTime * waterPlane->waveSpeed;

    // Use a single loop instead of nested loops
    int totalVertices = waterPlane->rows * waterPlane->cols;

    for (int index = 0; index < totalVertices; index++) {
        int i = index / waterPlane->cols; // Calculate the row (i)
        int j = index % waterPlane->cols; // Calculate the column (j)

        float noiseValue = SmoothNoise2D(
            (float)j * 0.1f, 
            (float)i * 0.1f + waterPlane->time
        );

        waterPlane->vertices[index].y = waterPlane->position.y + noiseValue * waterPlane->waveHeight;
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

static inline void DrawDoubleTriangles3D(Vector3 v1, Vector3 v2, Vector3 v3, Color c1,
                                            Vector3 v4, Vector3 v5, Vector3 v6, Color c2) {
    rlColor4ub(c1.r, c1.g, c1.b, c1.a);
    rlVertex3f(v1.x, v1.y, v1.z);
    rlVertex3f(v2.x, v2.y, v2.z);
    rlVertex3f(v3.x, v3.y, v3.z);

    rlColor4ub(c2.r, c2.g, c2.b, c2.a);
    rlVertex3f(v4.x, v4.y, v4.z);
    rlVertex3f(v5.x, v5.y, v5.z);
    rlVertex3f(v6.x, v6.y, v6.z);
}


// Draw the water plane
void DrawWaterPlane(WaterPlane *waterPlane, Vector3 cameraPosition, float renderDistance) {
    int rows = waterPlane->rows;
    int cols = waterPlane->cols;
    int totalCells = (rows - 1) * (cols - 1); // Total number of grid cells to process

    rlBegin(RL_TRIANGLES);
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

        // Calculate the distance from the camera to the vertices of the cell
        float distV1 = Vector3Distance(cameraPosition, v1);
        float distV2 = Vector3Distance(cameraPosition, v2);
        float distV3 = Vector3Distance(cameraPosition, v3);
        float distV4 = Vector3Distance(cameraPosition, v4);

        // Skip rendering if all vertices of the cell are beyond renderDistance
        if (distV1 > renderDistance && distV2 > renderDistance && 
            distV3 > renderDistance && distV4 > renderDistance) {
            continue;
        }

        // Calculate colors for each vertex
        Color c1 = CalculateVertexColor(waterPlane, v1);
        Color c2 = CalculateVertexColor(waterPlane, v2);

        // Draw the two triangles for the current cell
        // DrawTriangle3D(v1, v3, v2, c1);
        // DrawTriangle3D(v2, v3, v4, c2);
        DrawDoubleTriangles3D(v1, v3, v2, c1, v2, v3, v4, c2);
    }
    rlEnd();
}

void DrawWaterPlane_SIMD(WaterPlane *waterPlane, Vector3 cameraPosition, float renderDistance) {
    int rows = waterPlane->rows;
    int cols = waterPlane->cols;
    int totalCells = (rows - 1) * (cols - 1);

    // Prepare SIMD constants
    __m128 camPosX = _mm_set1_ps(cameraPosition.x);
    __m128 camPosY = _mm_set1_ps(cameraPosition.y);
    __m128 camPosZ = _mm_set1_ps(cameraPosition.z);
    __m128 renderDistSq = _mm_set1_ps(renderDistance * renderDistance);

    // Iterate over cells
    // #pragma omp parallel for schedule(dynamic)
    rlBegin(RL_TRIANGLES);
    for (int index = 0; index < totalCells; index++) {
        int i = index / (cols - 1); // Current row
        int j = index % (cols - 1); // Current column

        int rowStart = i * cols;
        int nextRowStart = (i + 1) * cols;

        // Load vertices for the current cell
        Vector3 vertices[4] = {
            waterPlane->vertices[rowStart + j],
            waterPlane->vertices[rowStart + j + 1],
            waterPlane->vertices[nextRowStart + j],
            waterPlane->vertices[nextRowStart + j + 1]
        };
        
        // SSE-optimized distance calculation
        __m128 vx = _mm_set_ps(vertices[3].x, vertices[2].x, vertices[1].x, vertices[0].x);
        __m128 vy = _mm_set_ps(vertices[3].y, vertices[2].y, vertices[1].y, vertices[0].y);
        __m128 vz = _mm_set_ps(vertices[3].z, vertices[2].z, vertices[1].z, vertices[0].z);

        __m128 dx = _mm_sub_ps(vx, camPosX);
        __m128 dy = _mm_sub_ps(vy, camPosY);
        __m128 dz = _mm_sub_ps(vz, camPosZ);

        __m128 distSq = _mm_add_ps(_mm_mul_ps(dx, dx), _mm_add_ps(_mm_mul_ps(dy, dy), _mm_mul_ps(dz, dz)));

        // Check if any vertex is within render distance
        __m128 cmp = _mm_cmple_ps(distSq, renderDistSq);
        int mask = _mm_movemask_ps(cmp);

        if (mask == 0) continue; // Skip if all vertices are out of range

        // Draw the triangles for the current cell
        Color c1 = CalculateVertexColor(waterPlane, vertices[0]);
        Color c2 = CalculateVertexColor(waterPlane, vertices[1]);

        DrawDoubleTriangles3D(vertices[0], vertices[2], vertices[1], c1, vertices[1], vertices[2], vertices[3], c2);
    }
    rlEnd();
}

// Free WaterPlane resources
void DestroyWaterPlane(WaterPlane *waterPlane) {
    free(waterPlane->vertices);
    waterPlane->vertices = NULL;
}
