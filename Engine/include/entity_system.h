#pragma once

#ifndef ENTITY_SYSTEM_H
#define ENTITY_SYSTEM_H

#include <game_entity.h>

#define INITIAL_ENTITY_CAPACITY 10 // Initial allocation size for entities
#define ENTITY_RESIZE_FACTOR 2 // Resize factor when safe_reallocating
#define INITIAL_TEXTURE_CAPACITY 10 // Initial allocation size for textures
#define TEXTURE_RESIZE_FACTOR 2 // Resize factor for textures

typedef enum {
    RECTANGLE,
    CIRCLE,
    TRIANGLE,
    UNKNOWN
} ShapeType;

typedef struct {
    ShapeType type;
    Vector2 position1;
    Vector2 position2;
    Vector2 position3;
    Color color;
    unsigned short thickness;
    bool filled;
} Shape;

typedef struct s_EntitySystem {
    t_Entity** entities; // Changed to dynamic allocation
    int num_entities;
    int capacity_entities;

    t_Texture* textures; // Changed to dynamic allocation
    int num_textures;
    int capacity_textures;
} t_EntitySystem;

extern t_EntitySystem* g_entity_system;


#endif