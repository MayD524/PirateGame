#include <player.h>

bool GetRayCollisionWithGround(Ray ray, float groundY, Vector3 *collisionPoint)
{
    // Calculate the intersection point
    if (ray.direction.y != 0)
    {
        float t = (groundY - ray.position.y) / ray.direction.y;
        if (t >= 0) // Ensure the intersection is in front of the ray
        {
            collisionPoint->x = ray.position.x + t * ray.direction.x;
            collisionPoint->y = groundY;
            collisionPoint->z = ray.position.z + t * ray.direction.z;
            return true;
        }
    }
    return false;
}

static void DrawLineOnGround(Camera3D camera)
{
    static Vector3 startPoint = { 0.0f, 0.0f, 0.0f };
    static Vector3 endPoint = { 0.0f, 0.0f, 0.0f };
    static bool hasLine = false;

    // Get the ray from the camera through the mouse position
    Ray mouseRay = GetMouseRay(GetMousePosition(), camera);

    // Check for ray intersection with the ground plane (y = 0)
    if (mouseRay.direction.y != 0) // Avoid division by zero
    {
        float t = -mouseRay.position.y / mouseRay.direction.y; // Intersection parameter
        if (t >= 0) // Ensure intersection is in front of the ray's origin
        {
            Vector3 intersectionPoint = {
                mouseRay.position.x + t * mouseRay.direction.x,
                0.0f, // Ground plane at y = 0
                mouseRay.position.z + t * mouseRay.direction.z
            };

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                startPoint = camera.position; // Start point is the camera position
                endPoint = intersectionPoint; // End point is the intersection point
                hasLine = true;               // Indicate a valid line exists
            }
        }
    }

    // Draw the line in 3D space
    if (hasLine)
    {
        DrawLine3D(startPoint, endPoint, MAROON); // Line color
    }
}

void PlayerUpdate(t_Entity* entity, float deltaTime) {
    Player* player = (Player*) entity->entity_data;

    HandleInput(player, deltaTime);
    ApplyGravity(player, deltaTime);
    UpdatePlayerCamera(player);

    // DrawLineOnClick3D(player, *player->camera);

    // printf("Player: %f %f %f ||| ", player->position.x, player->position.y, player->position.z);
    // printf("Camera: %f %f %f\n", player->camera.target.x, player->camera.target.y, player->camera.target.z);
}

static void PlaceSpheresAtScreenCenter(Camera3D camera)
{
    static Vector3 redSpherePosition = { 0.0f, 0.0f, 0.0f };
    static bool hasRedSphere = false;

    // Calculate the center of the screen
    Vector2 screenCenter = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };

    // Cast a ray from the screen center into the 3D world
    Ray centerRay = GetMouseRay(screenCenter, camera);

    // Calculate ray-plane intersection with ground plane (y = 0)
    Vector3 blueSpherePosition = { 0.0f, 0.0f, 0.0f }; // Temporary for blue sphere
    if (centerRay.direction.y != 0) // Avoid division by zero
    {
        float t = -centerRay.position.y / centerRay.direction.y; // Intersection parameter
        if (t >= 0) // Ensure intersection is in front of the ray's origin
        {
            blueSpherePosition = (Vector3){
                centerRay.position.x + t * centerRay.direction.x,
                0.0f, // Ground plane at y = 0
                centerRay.position.z + t * centerRay.direction.z
            };

            // Update the red sphere position if the mouse is clicked
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                redSpherePosition = blueSpherePosition; // Red sphere follows the intersection
                hasRedSphere = true;                   // Indicate red sphere exists
            }
        }
    }

    // Draw the blue sphere at the intersection point
    DrawSphere(blueSpherePosition, 0.1f, BLUE);

    // Draw the red sphere at its position if it exists
    if (hasRedSphere)
    {
        DrawSphere(redSpherePosition, 0.5f, RED);
    }

    // Debug: Draw the ray from the screen center
    Vector3 rayEnd = Vector3Add(centerRay.position, Vector3Scale(centerRay.direction, 100.0f));
    DrawLine3D(centerRay.position, rayEnd, GREEN);
}

void PlayerRender(t_Entity* entity) {
    Player* player = (Player*) entity->entity_data;
    

    BeginMode3D(*player->camera);
    PlaceSpheresAtScreenCenter(*player->camera);

    EndMode3D();
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
    player->jumpStrength = 10.0f;
    player->isGrounded = true;

    player->camera = (Camera*) safe_malloc(sizeof(Camera));

    player->camera->position = startPosition;
    player->camera->target = (Vector3){ startPosition.x, startPosition.y, startPosition.z - 1.0f };
    player->camera->up = (Vector3){ 0.0f, 1.0f, 0.0f };
    player->camera->fovy = 45.0f;
    player->camera->projection = CAMERA_PERSPECTIVE;

    entity->entity_data = player;
    entity->on_render = PlayerRender;
    entity->update = PlayerUpdate;
    g_engine->camera = player->camera;

    add_tag(entity, "PLAYER");

    return *player;
}
 // Check for ray collision with a ground plane
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
        // Apply an instantaneous jump by setting Y velocity
        player->player_ent->entity3D.velocity.y = player->jumpStrength;
        player->velocityY = player->jumpStrength;
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

    // if (!player->isGrounded) {
    //     player->velocityY += (-9.81f) * FIXED_TIMESTEP;
    //     player->position.y += player->velocityY * FIXED_TIMESTEP;

    //     printf("VEL: %f\n", player->velocityY);
    //     printf("VEL: %f %f %f\n", player->position.x, player->position.y, player->position.z);
    // }
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
