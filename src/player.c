#include <player.h>

void PlayerUpdate(t_Entity* entity, float deltaTime) {
    Player* player = (Player*) entity->entity_data;

    HandleInput(player, deltaTime);
    ApplyGravity(player, deltaTime);
    UpdatePlayerCamera(player);

    // printf("Player: %f %f %f ||| ", player->position.x, player->position.y, player->position.z);
    // printf("Camera: %f %f %f\n", player->camera.target.x, player->camera.target.y, player->camera.target.z);
}

Player InitPlayer(Vector3 startPosition) {
    t_Entity* entity = create_entity3D(
        g_entity_system,
        "Player",
        NULL, // Local player
        startPosition,
        VEC3_ZERO,
        PLAYER,
        FLT_MAX,
        VEC3_ONE,
        100.0f
    );

    Player* player = (Player*)safe_malloc(sizeof(Player));
    player->player_ent = entity;
    player->position = startPosition;
    player->speed = 5.0f;                // Default movement speed
    player->sprintMultiplier = 2.0f;    // Sprint multiplier
    player->yaw = 0.0f;
    player->pitch = 0.0f;
    player->sensitivity = 0.1f;
    player->velocityY = 0.0f;
    player->jumpStrength = 7.0f;
    player->isGrounded = true;

    player->camera = (Camera*) safe_malloc(sizeof(Camera));

    player->camera->position = startPosition;
    player->camera->target = (Vector3){ startPosition.x, startPosition.y, startPosition.z - 1.0f };
    player->camera->up = (Vector3){ 0.0f, 1.0f, 0.0f };
    player->camera->fovy = 45.0f;
    player->camera->projection = CAMERA_PERSPECTIVE;

    entity->entity_data = player;
    entity->update = PlayerUpdate;
    g_engine->camera = player->camera;

    add_tag(entity, "PLAYER");

    return *player;
}

static Vector3 normalize_vector3(Vector3 v) {
    return Vector3NormalizeCustom(v);
}

// Updated HandleInput function for Instantaneous Movement
void HandleInput(Player* player, float delta_time) {
    if (!player || !player->player_ent || !player->player_ent->is_active) return;

    // Handle mouse input for yaw and pitch
    Vector2 mouseDelta = GetMouseDelta();
    player->yaw += mouseDelta.x * player->sensitivity;
    player->pitch -= mouseDelta.y * player->sensitivity;

    // Clamp pitch to avoid flipping
    if (player->pitch > 89.0f) player->pitch = 89.0f;
    if (player->pitch < -89.0f) player->pitch = -89.0f;

    // Calculate direction vectors (XZ plane)
    Vector3 forward = {
        cosf(DEG2RAD * player->yaw),
        0.0f, // No vertical component for horizontal movement
        sinf(DEG2RAD * player->yaw)
    };

    Vector3 right = {
        cosf(DEG2RAD * (player->yaw + 90.0f)),
        0.0f,
        sinf(DEG2RAD * (player->yaw + 90.0f))
    };

    forward = normalize_vector3(forward);
    right = normalize_vector3(right);

    // Determine movement direction
    Vector3 movementDirection = {0.0f, 0.0f, 0.0f};

    if (IsKeyDown(KEY_W)) {
        movementDirection = Vector3Add(movementDirection, forward);
    }
    if (IsKeyDown(KEY_S)) {
        movementDirection = Vector3Subtract(movementDirection, forward);
    }
    if (IsKeyDown(KEY_A)) {
        movementDirection = Vector3Subtract(movementDirection, right);
    }
    if (IsKeyDown(KEY_D)) {
        movementDirection = Vector3Add(movementDirection, right);
    }

    // Normalize the movement direction to prevent faster diagonal movement
    movementDirection = normalize_vector3(movementDirection);

    // Adjust speed for sprinting
    float currentSpeed = player->speed;
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        currentSpeed *= player->sprintMultiplier;
    }

    // Calculate desired horizontal velocity
    Vector3 desiredVelocity = Vector3Scale(movementDirection, currentSpeed);


    // Set the player's horizontal velocity directly
    player->player_ent->entity3D.velocity.x = desiredVelocity.x;
    player->player_ent->entity3D.velocity.z = desiredVelocity.z;

    // Handle jumping
    if (player->isGrounded && IsKeyPressed(KEY_SPACE)) {
        printf("Jump\n");
        // Apply an instantaneous jump by setting Y velocity
        player->player_ent->entity3D.velocity.y = player->jumpStrength;
        player->isGrounded = false;
    }

    // Ground check based on position (simplistic approach)
    if (player->player_ent->entity3D.velocity.y <= 0.0f && player->player_ent->entity3D.position.y <= (player->player_ent->entity3D.scale.y / 2.0f) + 0.1f) {
        player->isGrounded = true;
        // Correct position if below ground
        player->player_ent->entity3D.position.y = (player->player_ent->entity3D.scale.y / 2.0f) + 0.1f;
        // Zero the Y velocity to prevent sinking
        player->player_ent->entity3D.velocity.y = 0.0f;
    }

    // Clamp the player's velocity to the maximum speed (optional redundancy)
    float horizontalSpeed = sqrtf(player->player_ent->entity3D.velocity.x * player->player_ent->entity3D.velocity.x +
                                  player->player_ent->entity3D.velocity.z * player->player_ent->entity3D.velocity.z);
    if (horizontalSpeed > PLAYER_MAX_SPEED) {
        float scale = PLAYER_MAX_SPEED / horizontalSpeed;
        player->player_ent->entity3D.velocity.x *= scale;
        player->player_ent->entity3D.velocity.z *= scale;
    }

    // Update player's position (assuming Physics_Integrate handles position update)
    player->position = player->player_ent->entity3D.position;

    // Debugging information (optional)
    
}

void ApplyGravity(Player *player, float deltaTime) {

    if (!player->isGrounded) {
        player->velocityY += (GRAVITY.y);
        player->position.y += player->velocityY * deltaTime;

        // Simulate ground collision
        if (player->position.y <= 0.0f) {
            player->position.y = 0.0f;
            player->velocityY = 0.0f;
            player->isGrounded = true;
            player->player_ent->is_grounded = true;
        }
    }
}

void UpdatePlayerCamera(Player *player) {
    Vector3 forward = {
        cosf(DEG2RAD * player->yaw),
        sinf(DEG2RAD * player->pitch),
        sinf(DEG2RAD * player->yaw)
    };

    player->camera->position = (Vector3){ player->position.x, player->position.y + 2.0f, player->position.z };
    player->camera->target = Vector3Add(player->camera->position, forward);
}
