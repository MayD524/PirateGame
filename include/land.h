#pragma once

#ifndef LAND_H
#define LAND_H

#include <entity.h>

typedef struct {
    float width, height, depth; // Dimensions of the building or land
    Color color;               // Color of the building or land
    bool is_collidable;        // Whether the entity can collide with others
} BuildingData;

t_Entity create_land(Vector3 position, Vector2 size, Color color);
void render_land(t_Entity* entity);
void destroy_land(t_Entity* entity);

#endif