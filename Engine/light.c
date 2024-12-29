#include <light.h>



Light CreateLight(LightType type, Vector3 position, Vector3 direction, Color color, float intensity, float radius, float angle) {
    Light light = (Light){ 0 };
    light.type = type;
    light.position = position;
    light.direction = direction;
    light.color = color;
    light.intensity = intensity;
    light.radius = radius;
    light.angle = angle;
    return light;
}