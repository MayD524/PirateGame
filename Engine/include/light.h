#ifndef LIGHT_H
#define LIGHT_H

#include <raylib.h>
#include <raymath.h>


#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION        330
#else
    #define GLSL_VERSION        100
#endif 

#include <extended_memory.h>
typedef enum {
    LIGHT_DIRECTIONAL,
    LIGHT_POINT,
    LIGHT_SPOT,
    LIGHT_AMBIENT
} LightType;


typedef struct {
    LightType type;      // Type of light
    Vector3 position;    // Light position (for point/spot lights)
    Vector3 direction;   // Light direction (for directional/spot lights)
    Color color;         // Light color
    float intensity;     // Light intensity
    float radius;        // Light radius (for point/spot lights)
    float angle;         // Spot light angle (in degrees)
} Light;



typedef struct {

} ShaderManager;

Light CreateLight(LightType type, Vector3 position, Vector3 direction, Color color, float intensity, float radius, float angle);


#endif // !LIGHT_H