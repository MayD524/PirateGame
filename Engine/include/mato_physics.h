// physics.h
#ifndef PHYSICS_H
#define PHYSICS_H

#include <game_entity.h>
#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>

// Define gravitational constant (modifiable as needed)
#define GRAVITY (Vector3){0.0f, -981.0f, 0.0f}

// Structure representing a hit by a ray
typedef struct {
    t_Entity* entity;         // Entity that was hit
    RayCollision collision;   // Collision details
} RayHit;

// Initialize a physics-enabled entity
void Physics_InitEntity(t_Entity* entity, Vector3 position, Vector3 scale, float mass, bool is_static, CollisionType collision_type);

// Apply a force to an entity
void Physics_ApplyForce(t_Entity* entity, Vector3 force);

// Integrate physics for a single entity
void Physics_Integrate(t_Entity* entity, float deltaTime);

// General Collision Detection between two entities
bool Physics_CheckCollision(const t_Entity* a, const t_Entity* b);

// Specific Collision Detection Functions
bool Physics_CheckCollisionAABB(const t_Entity* a, const t_Entity* b);
bool Physics_CheckCollisionSphere(const t_Entity* a, const t_Entity* b);

// Ray-Entity Intersection (supports AABB and Sphere)
bool Physics_RayIntersectsEntity(const t_Entity* entity, Ray ray, RayCollision* collision);

// Collision Resolution Functions
void Physics_ResolveCollision(t_Entity* a, t_Entity* b);
void Physics_ResolveCollisionImpulse(t_Entity* a, t_Entity* b);

// Utility Functions
float Vector3LengthCustom(const Vector3 v);
Vector3 Vector3NormalizeCustom(const Vector3 v);

// Raycasting Function to detect the first collision in a list of entities
RayHit Physics_Raycast(t_Entity* entities[], int entityCount, Ray ray);

#endif // PHYSICS_H
