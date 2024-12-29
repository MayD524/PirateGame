#pragma once

#ifndef PLAYER_H
#define PLAYER_H

#include <mato_physics.h>
#include <entity.h>
#include <raylib.h>
#include <raymath.h>
#include <core.h>
#include <math.h>

#ifndef PLAYER_MAX_SPEED
#define PLAYER_MAX_SPEED 20.0f // Adjust as needed
#endif


typedef struct {
    Camera* camera;         // Player's camera
    t_Entity* player_ent;
    Vector3 position;      // Player's position
    float speed;           // Base movement speed
    float sprintMultiplier; // Sprint speed multiplier
    float yaw;             // Horizontal rotation (yaw)
    float pitch;           // Vertical rotation (pitch)
    float sensitivity;     // Mouse sensitivity
    float velocityY;       // Vertical velocity for jumping
    float jumpStrength;    // Jump strength
    bool isGrounded;       // Check if player is on the ground
} Player;


Player InitPlayer(Vector3 startPosition);
void HandleInput(Player *player, float deltaTime);
void ApplyGravity(Player *player, float deltaTime);
void UpdatePlayerCamera(Player *player);
#endif