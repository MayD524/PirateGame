// water_shader.c

#include "water_shader.h"
#include <rlgl.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>

// Initialize the water shader
WaterShader* InitWaterShader(const char* vertexPath, const char* fragmentPath)
{
    WaterShader* water = (WaterShader*)malloc(sizeof(WaterShader));
    if (!water) {
        printf("Failed to allocate WaterShader.\n");
        return NULL;
    }

    water->shader = LoadShader(vertexPath, fragmentPath);
    if (water->shader.id == 0) {
        printf("Failed to load water shader.\n");
        free(water);
        return NULL;
    }

    // Initialize default sea wave parameters
    water->seaWave.amplitude = 0.5f;
    water->seaWave.wavelength = 4.0f;
    water->seaWave.speed = 1.0f;
    water->seaWave.directionX = 1.0f;
    water->seaWave.directionZ = 0.0f;

    // Initialize default lighting and color parameters
    water->lightPos = (Vector3){ 50.0f, 100.0f, 50.0f };
    water->lightColor = (Vector3){ 1.0f, 1.0f, 1.0f };
    water->waterColor = (Vector4){ 0.0f, 0.5f, 0.7f, 0.8f };
    water->specularColor = (Vector4){ 1.0f, 1.0f, 1.0f, 1.0f };
    water->shininess = 32.0f;

    return water;
}

// Update shader uniforms
void UpdateWaterShader(WaterShader* water, Camera3D camera, float time)
{
    if (!water) return;

    // Set shader uniforms
    int loc_time = GetShaderLocation(water->shader, "time");
    SetShaderValue(water->shader, loc_time, &time, SHADER_UNIFORM_FLOAT);

    int loc_waveSpeed = GetShaderLocation(water->shader, "waveSpeed");
    SetShaderValue(water->shader, loc_waveSpeed, &water->seaWave.speed, SHADER_UNIFORM_FLOAT);

    int loc_waveScale = GetShaderLocation(water->shader, "waveScale");
    SetShaderValue(water->shader, loc_waveScale, &water->seaWave.wavelength, SHADER_UNIFORM_FLOAT); // waveScale corresponds to wavelength

    int loc_waveHeight = GetShaderLocation(water->shader, "waveHeight");
    SetShaderValue(water->shader, loc_waveHeight, &water->seaWave.amplitude, SHADER_UNIFORM_FLOAT);

    int loc_direction = GetShaderLocation(water->shader, "direction");
    Vector2 direction = { water->seaWave.directionX, water->seaWave.directionZ };
    SetShaderValue(water->shader, loc_direction, &direction, SHADER_UNIFORM_VEC2);

    // View position
    Vector3 viewPos = camera.position;
    int loc_viewPos = GetShaderLocation(water->shader, "viewPos");
    SetShaderValue(water->shader, loc_viewPos, &viewPos, SHADER_UNIFORM_VEC3);

    // Matrices
    Matrix model = MatrixIdentity(); // Modify if necessary
    Matrix view = GetCameraMatrix(camera); // Correctly obtain the view matrix
    Matrix projection = MatrixPerspective(DEG2RAD * camera.fovy, 
                                        (float)GetScreenWidth() / (float)GetScreenHeight(), 
                                        0.1f, 1000.0f);


    int loc_model = GetShaderLocation(water->shader, "model");
    SetShaderValueMatrix(water->shader, loc_model, model);

    int loc_viewMat = GetShaderLocation(water->shader, "view");
    SetShaderValueMatrix(water->shader, loc_viewMat, view);

    int loc_projection = GetShaderLocation(water->shader, "projection");
    SetShaderValueMatrix(water->shader, loc_projection, projection);

    // Lighting
    int loc_lightPos = GetShaderLocation(water->shader, "lightPos");
    SetShaderValue(water->shader, loc_lightPos, &water->lightPos, SHADER_UNIFORM_VEC3);

    int loc_lightColor = GetShaderLocation(water->shader, "lightColor");
    SetShaderValue(water->shader, loc_lightColor, &water->lightColor, SHADER_UNIFORM_VEC3);

    // Water color
    int loc_waterColor = GetShaderLocation(water->shader, "waterColor");
    SetShaderValue(water->shader, loc_waterColor, &water->waterColor, SHADER_UNIFORM_VEC4);

    // Specular color
    int loc_specularColor = GetShaderLocation(water->shader, "specularColor");
    SetShaderValue(water->shader, loc_specularColor, &water->specularColor, SHADER_UNIFORM_VEC4);

    // Shininess
    int loc_shininess = GetShaderLocation(water->shader, "shininess");
    SetShaderValue(water->shader, loc_shininess, &water->shininess, SHADER_UNIFORM_FLOAT);
}

// Draw water plane
void DrawWaterPlane(Model waterModel, WaterShader* waterShader)
{
    // Draw the water model with the assigned shader
    DrawModel(waterModel, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
}

// Unload water shader
void UnloadWaterShader(WaterShader* water)
{
    if (water) {
        UnloadShader(water->shader);
        free(water);
    }
}


// Function to generate a grid mesh for water
Mesh GenWaterMesh(int gridSize, float gridSpacing)
{
    Mesh mesh = { 0 };
    mesh.triangleCount = (gridSize - 1) * (gridSize - 1) * 2;
    mesh.vertices = (float *)malloc(gridSize * gridSize * 3 * sizeof(float));
    mesh.texcoords = (float *)malloc(gridSize * gridSize * 2 * sizeof(float));
    mesh.indices = (unsigned short *)malloc(mesh.triangleCount * 3 * sizeof(unsigned short));

    if (!mesh.vertices || !mesh.texcoords || !mesh.indices) {
        printf("Failed to allocate memory for mesh.\n");
        // Handle allocation failure
        if (mesh.vertices) free(mesh.vertices);
        if (mesh.texcoords) free(mesh.texcoords);
        if (mesh.indices) free(mesh.indices);
        return mesh;
    }

    // Generate vertices and texture coordinates
    for (int z = 0; z < gridSize; z++)
    {
        for (int x = 0; x < gridSize; x++)
        {
            mesh.vertices[(z * gridSize + x) * 3 + 0] = (float)x * gridSpacing;
            mesh.vertices[(z * gridSize + x) * 3 + 1] = 0.0f; // Y position (height)
            mesh.vertices[(z * gridSize + x) * 3 + 2] = (float)z * gridSpacing;

            mesh.texcoords[(z * gridSize + x) * 2 + 0] = (float)x / (gridSize - 1);
            mesh.texcoords[(z * gridSize + x) * 2 + 1] = (float)z / (gridSize - 1);
        }
    }

    // Generate indices
    int index = 0;
    for (int z = 0; z < gridSize - 1; z++)
    {
        for (int x = 0; x < gridSize - 1; x++)
        {
            unsigned short topLeft = z * gridSize + x;
            unsigned short topRight = z * gridSize + x + 1;
            unsigned short bottomLeft = (z + 1) * gridSize + x;
            unsigned short bottomRight = (z + 1) * gridSize + x + 1;

            // First triangle
            mesh.indices[index++] = topLeft;
            mesh.indices[index++] = bottomLeft;
            mesh.indices[index++] = topRight;

            // Second triangle
            mesh.indices[index++] = topRight;
            mesh.indices[index++] = bottomLeft;
            mesh.indices[index++] = bottomRight;
        }
    }

    // Assign mesh properties
    mesh.vertexCount = gridSize * gridSize;
    // Removed materialCount and materials as Mesh does not have them in Raylib 5.5

    return mesh;
}