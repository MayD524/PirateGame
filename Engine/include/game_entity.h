#pragma once
#ifndef GAMEENTITY_H
#define GAMEENTITY_H

#include <pthread.h>
#include <raylib.h>

typedef enum {
    NONE,
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL,
    PLAYER,
} PriorityRank;

typedef struct s_Tag {
    const char* tag;
    unsigned int tag_id;
    unsigned long tag_hash;

    struct s_Tag* next;
    struct s_Tag* prev;
} t_Tag;

// Define different collision types
typedef enum {
    COLLISION_NONE,
    COLLISION_AABB,    // Axis-Aligned Bounding Box
    COLLISION_SPHERE,  // Sphere
    COLLISION_MESH
    // Additional collision types can be added here
} CollisionType;

typedef struct {
    Vector3 p1;
    Vector3 p2;
    Vector3 p3;
} Triangle;

typedef struct {
    Triangle* triangles;
    int triangleCount;
} MatoMesh;

typedef struct s_Entity {
    // Common attributes
    char* entity_name;
    t_Tag* root_tag;
    void* entity_data;

    bool is_active;
    unsigned char layer;
    unsigned char num_tags;

    float health;
    float max_life_time;
    float current_life_time;

    PriorityRank priority_rank;

    void (*on_start)(struct s_Entity* entity);
    void (*on_render)(struct s_Entity* entity);
    void (*update)(struct s_Entity* entity, float delta_time);
    void (*on_collision)(struct s_Entity* other, struct s_Entity* entity);
    void (*on_destroy)(struct s_Entity* entity);
    void (*on_grounded)(struct s_Entity* entity);
    void (*on_airborne)(struct s_Entity* entity);

    struct {
        Vector3 position;
        Vector3 velocity;
        Vector3 rotation_axis;
        float rotation;
        Vector3 scale;
        int model_id;
    } entity3D;

    int rigid_body_id;

    // Collision attributes
    CollisionType collision_type;

    // Physics attributes
    Vector3 angular_velocity; // Angular velocity vector
    Vector3 torqueAccum;      // Accumulated torque
    float inverseInertia;     // Inverse of
    Vector3 forceAccum;    // Accumulated forces
    Vector3 acceleration;  // Current acceleration
    float mass;
    float inverseMass;     // For optimization
    bool has_moved;
    bool is_grounded;
    bool is_static;        // If true, the entity is immovable
    float dampingFactor;
    MatoMesh mesh;

    pthread_mutex_t mutex;
} t_Entity;

typedef struct s_Texture {
    char* texture_name;
    Texture2D texture;
} t_Texture;

unsigned long compute_tag_hash(const char* tag);
void print_tags(const t_Entity* entity);
void add_tag(t_Entity* entity, const char* tag);
const char** get_tags(const t_Entity* entity);
void remove_tag_at(t_Entity* entity, int index);
void remove_tag(t_Entity* entity, const char* tag);
int has_tag(const t_Entity* entity, const char* tag);
void remove_all_tags(t_Entity* entity);

#endif