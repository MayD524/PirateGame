// water_shader.h

#ifndef WATER_SHADER_H
#define WATER_SHADER_H

#include "raylib.h"
#include "rlgl.h"

typedef struct {
    float amplitude;
    float wavelength;
    float speed;
    float directionX;
    float directionZ;
} SeaWave;

// Structure to hold water shader information
typedef struct WaterShader {
    Shader shader;
    SeaWave seaWave;
    Vector3 lightPos;
    Vector3 lightColor;
    Vector4 waterColor;
    Vector4 specularColor;
    float shininess;
} WaterShader;

// Initializes the water shader with default parameters
WaterShader* InitWaterShader(const char* vertexPath, const char* fragmentPath);

// Updates shader uniforms each frame
void UpdateWaterShader(WaterShader* water, Camera3D camera, float time);

// Draws the water plane using the shader and mesh
void DrawWaterPlane(Model waterModel, WaterShader* waterShader);

// Unloads the water shader resources
void UnloadWaterShader(WaterShader* water);

Mesh GenWaterMesh(int gridSize, float gridSpacing);
#endif // WATER_SHADER_H