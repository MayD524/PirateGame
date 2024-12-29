#pragma once

#ifndef ENTITY_H
#define ENTITY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <raymath.h>
#include <util.h>
#include <math.h>
#include <float.h>
#include <macros.h>
#include <game_entity.h>
#include <mato_physics.h>
#include <SpatialGrid3D.h>
#include <extended_memory.h>
#include <models.h>
#include <custom3d.h>

#ifdef __linux__
    #include <unistd.h>
#endif

#define MIN_DRAW_DISTANCE         5.0f
#define FADE_START_DISTANCE      80.0f
#define MAX_DRAW_DISTANCE       100.0f

// LOD thresholds (you can tweak these)
#define LOD0_DISTANCE           15.0f
#define LOD1_DISTANCE           30.0f

// For demonstration, we define a random range macro
#define RAND_RANGE(low, high)  ((low) + ((float)rand() / RAND_MAX) * ((high)-(low)))


static const Vector2 VEC2_ZERO = { 0.0f, 0.0f };
static const Vector2 VEC2_ONE = { 1.0f, 1.0f };
static const Vector3 VEC3_ZERO = { 0.0f, 0.0f, 0.0f };
static const Vector3 VEC3_ONE = { 1.0f, 1.0f, 1.0f };

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

t_EntitySystem* create_entity_system();
void destroy_entity_system(t_EntitySystem* entity_system);
void update(t_EntitySystem* entity_system);
void render(t_EntitySystem* entity_system, TextLabelArray* text_labels, Camera camera);

int load_texture(t_EntitySystem* entity_system, const char* texture_path);
Texture2D get_texture(t_EntitySystem* entity_system, int id);

// Entity functions
t_Entity* create_entity(t_EntitySystem* es, char* entity_name, char* texturePath, Vector2 start_position, Vector2 start_velocity, PriorityRank priority_rank, float max_life_time, float scale, float health);
t_Entity* create_entity3D(t_EntitySystem* es, char* entity_name, char* modelPath, Vector3 start_position, Vector3 start_velocity, PriorityRank priority_rank, float max_life_time, Vector3 scale3D, float health);
t_Entity* get_entity(t_EntitySystem* es, int index);
t_Entity* get_entity_by_name(t_EntitySystem* es, char* entity_name);

void DrawHitboxes(t_EntitySystem* entity_system, bool enableHitbox);

void print_movement(const t_Entity* entity);
bool are_entities_colliding(const t_Entity* entity_a, const t_Entity* entity_b);

void print_tags(const t_Entity* entity);
void add_tag(t_Entity* entity, const char* tag);
const char** get_tags(const t_Entity* entity);
void remove_tag_at(t_Entity* entity, int index);
void remove_tag(t_Entity* entity, const char* tag);
int has_tag(const t_Entity* entity, const char* tag);
void remove_all_tags(t_Entity* entity);

Rectangle get_entity_source(const t_Entity* entity);
Rectangle get_entity_destination(const t_Entity* entity);

void destroy_entity(t_Entity* entity);

#endif // ENTITY_H
