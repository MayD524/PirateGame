// physics.h
#pragma once

#ifndef PHYSICS_H
#define PHYSICS_H

#include <immintrin.h> // For AVX/SSE intrinsics
#include <game_entity.h>
#include <models.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdbool.h>

// Define gravitational constant (modifiable as needed)
#define GRAVITY_Y -981.0f
#define GRAVITY (Vector3){0.0f, GRAVITY_Y, 0.0f}

extern const __m128 HALF_PS;
extern const __m128 ZERO_PS;
extern const __m128 ONE_PS;
extern const __m128 NEGATE_PS;


#define BASE_FRICTION 0.04f

// Structure representing a hit by a ray
typedef struct {
    t_Entity* entity;         // Entity that was hit
    RayCollision collision;   // Collision details
} RayHit;

// Initialize a physics-enabled entity
void Physics_InitEntity(t_Entity* entity, Vector3 position, Vector3 scale, float mass, bool is_static, CollisionType collision_type);

// Apply a force to an entity
void Physics_ApplyForce(t_Entity* entity, Vector3 force);
void Physics_ApplyTorque(t_Entity* entity, Vector3 torque);
void Physics_ApplyForceAtPoint(t_Entity* entity, Vector3 force, Vector3 point);

// Integrate physics for a single entity
void Physics_Integrate(t_Entity* entity, float deltaTime);

// General Collision Detection between two entities
bool Physics_CheckCollision(const t_Entity* a, const t_Entity* b);

// Specific Collision Detection Functions
bool Physics_CheckCollisionAABB_SIMD(const t_Entity* a, const t_Entity* b);
bool Physics_CheckCollisionAABB(const t_Entity* a, const t_Entity* b);
bool Physics_CheckCollisionSphere(const t_Entity* a, const t_Entity* b);

// Ray-Entity Intersection (supports AABB and Sphere)
bool Physics_RayIntersectsEntity(const t_Entity* entity, Ray ray, RayCollision* collision);

// Collision Resolution Functions
void Physics_ResolveCollision(t_Entity* a, t_Entity* b);
void Physics_ResolveCollisionImpulse(t_Entity* a, t_Entity* b);
void Physics_ResolveCollisionWithTorque(t_Entity* a, t_Entity* b, Vector3 contactPoint, Vector3 contactNormal);

// Utility Functions
float Vector3LengthCustom(const Vector3 v);
Vector3 Vector3NormalizeCustom(const Vector3 v);

// Raycasting Function to detect the first collision in a list of entities
RayHit Physics_Raycast(t_Entity* entities[], int entityCount, Ray ray);
void Physics_CleanupEntities(t_Entity* entities[], int entityCount);
void Physics_UpdateAll(t_Entity* entities[], int entityCount, float deltaTime);

#endif // PHYSICS_H
