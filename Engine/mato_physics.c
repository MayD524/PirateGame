// physics.c
#include <mato_physics.h>
#include <math.h>
#include <float.h>

// Helper function to calculate inverse mass
static float CalculateInverseMass(float mass, bool is_static) {
    if (is_static || mass <= 0.0f) return 0.0f;
    return 1.0f / mass;
}


float Vector3LengthCustom(const Vector3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

// Utility function to normalize a vector
Vector3 Vector3NormalizeCustom(const Vector3 v) {
    float length = Vector3LengthCustom(v);
    if (length == 0.0f) return (Vector3){0.0f, 0.0f, 0.0f};
    return Vector3Scale(v, 1.0f / length);
}

// Initialize a physics-enabled entity
void Physics_InitEntity(t_Entity* entity, Vector3 position, Vector3 scale, float mass, bool is_static, CollisionType collision_type) {
    if (!entity) return;

    // Initialize position and movement
    entity->entity3D.position = position;
    entity->entity3D.velocity = (Vector3){0.0f, 0.0f, 0.0f};
    entity->entity3D.rotation_axis = (Vector3){0.0f, 1.0f, 0.0f};
    entity->entity3D.rotation = 0.0f;
    entity->entity3D.scale = scale;
    
    // Initialize physics attributes
    entity->forceAccum = (Vector3){0.0f, 0.0f, 0.0f};
    entity->acceleration = GRAVITY; // Default acceleration due to gravity
    entity->mass = mass;
    entity->inverseMass = CalculateInverseMass(mass, is_static);
    entity->is_static = is_static;
    entity->collision_type = collision_type;
    entity->dampingFactor = 100.0f;
}

// Apply a force to an entity
void Physics_ApplyForce(t_Entity* entity, Vector3 force) {
    if (entity->is_static) return; // Do not apply forces to static entities
    entity->forceAccum = Vector3Add(entity->forceAccum, force);
}

// Integrate physics for a single entity using Euler Integration
void Physics_Integrate(t_Entity* entity, float deltaTime) {
    if (!entity || deltaTime <= 0.0f) return; // Safety checks
    if (entity->is_static) return; // No integration for static entities

    // **Ground Collision Detection and Response**

    // Calculate the minimum Y position (assuming Y is up)
    float minY = (entity->entity3D.scale.y / 2.0f) + EPSILON;

    if (entity->entity3D.position.y <= minY) {
        // Clamp position to ground level
        entity->entity3D.position.y = minY;

        // Reset vertical velocity
        entity->entity3D.velocity.y = 0.0f;

        // Update grounded state and trigger callbacks if necessary
        if (!entity->is_grounded) {
            entity->is_grounded = true;
            if (entity->on_grounded) {
                entity->on_grounded(entity);
            }
        }
    } else {
        // Entity is above ground
        if (entity->is_grounded) {
            entity->is_grounded = false;
            if (entity->on_airborne) {
                entity->on_airborne(entity);
            }
        }
    }

    // **Force Accumulation**

    // Apply gravity if not grounded
    if (!entity->is_grounded) {
        entity->forceAccum = Vector3Add(entity->forceAccum, GRAVITY);
    }

    // **Ground Friction Calculation**

    if (entity->is_grounded) {
        // Extract horizontal velocity (X and Z components)
        Vector3 velocity_horizontal = { entity->entity3D.velocity.x, 0.0f, entity->entity3D.velocity.z };
        float speed = Vector3Length(velocity_horizontal);

        if (speed > EPSILON) {
            // Normalize horizontal velocity to get the direction
            Vector3 friction_dir = Vector3Normalize(velocity_horizontal);

            // Calculate the magnitude of the friction force
            // Friction force = -friction_coefficient * normal_force
            // Assuming normal_force = mass * gravity (since on flat ground)
            float mass = (entity->inverseMass > 0.0f) ? (1.0f / entity->inverseMass) : FLT_MAX;
            float normal_force = mass * fabsf(GRAVITY.y); // Ensure gravity.y is positive

            float friction_magnitude = normal_force * BASE_FRICTION ;

            // Calculate friction force vector
            Vector3 friction_force = Vector3Scale(friction_dir, -friction_magnitude);

            // Add friction force to the force accumulator
            entity->forceAccum = Vector3Add(entity->forceAccum, friction_force);

            // **Prevent Over-friction (Optional)**
            // Ensure that friction does not reverse the velocity direction
            // Compute the potential velocity change due to friction
            float potential_velocity_change = friction_magnitude / mass * deltaTime;

            if (potential_velocity_change > speed) {
                // Friction is strong enough to stop the entity
                entity->entity3D.velocity.x = 0.0f;
                entity->entity3D.velocity.z = 0.0f;
            }
        }
    }

    // **Total Force and Acceleration Calculation**

    // Include any additional accelerations (e.g., from user input)
    Vector3 totalForce = Vector3Add(entity->forceAccum, entity->acceleration);

    // Calculate resulting acceleration: a = F * inverseMass
    Vector3 resultingAcc = Vector3Scale(totalForce, entity->inverseMass);

    // **Velocity Integration**

    // Update velocity: v = v + a * deltaTime
    entity->entity3D.velocity = Vector3Add(entity->entity3D.velocity, Vector3Scale(resultingAcc, deltaTime));

    // **Position Integration**

    // Update position: p = p + v * deltaTime
    entity->entity3D.position = Vector3Add(entity->entity3D.position, Vector3Scale(entity->entity3D.velocity, deltaTime));

    // **Additional Ground Clamp (Redundant but Safe)**

    if (entity->entity3D.position.y < (entity->entity3D.scale.y / 2.0f)) {
        entity->entity3D.position.y = (entity->entity3D.scale.y / 2.0f);
        entity->entity3D.velocity.y = 0.0f;

        if (!entity->is_grounded) {
            entity->is_grounded = true;
            if (entity->on_grounded) {
                entity->on_grounded(entity);
            }
        }
    }

    // **Reset Force Accumulator**

    entity->forceAccum = (Vector3){0.0f, 0.0f, 0.0f};
}


// General Collision Detection between two entities
bool Physics_CheckCollision(const t_Entity* a, const t_Entity* b) {
    if (!a || !b) return false;

    // Determine collision type combinations
    if (a->collision_type == COLLISION_AABB && b->collision_type == COLLISION_AABB) {
        return Physics_CheckCollisionAABB(a, b);
    }
    else if (a->collision_type == COLLISION_SPHERE && b->collision_type == COLLISION_SPHERE) {
        return Physics_CheckCollisionSphere(a, b);
    }
    // Add more collision type combinations as needed
    else {
        // Handle mixed collision types or unsupported types
        // For simplicity, default to AABB collision
        return Physics_CheckCollisionAABB(a, b);
    }
}

// AABB Collision Detection
bool Physics_CheckCollisionAABB(const t_Entity* a, const t_Entity* b) {
    if (!a || !b) return false;

    BoundingBox bboxA, bboxB;

    // Calculate bounding box for entity A
    ModelInfo* modelInfoA = get_model_info(a->entity3D.model_id);
    if (modelInfoA) {
        // Use model bounding box and adjust to entity's position
        BoundingBox modelBoxA = get_model_bounding_box(modelInfoA->model);
        Vector3 bboxCenterA = Vector3Scale(Vector3Add(modelBoxA.min, modelBoxA.max), 0.5f);
        bboxA.min = Vector3Add(Vector3Subtract(modelBoxA.min, bboxCenterA), a->entity3D.position);
        bboxA.max = Vector3Add(Vector3Subtract(modelBoxA.max, bboxCenterA), a->entity3D.position);
    } else {
        // Fallback to scale-based bounding box
        bboxA.min = Vector3Subtract(a->entity3D.position, Vector3Scale(a->entity3D.scale, 0.5f));
        bboxA.max = Vector3Add(a->entity3D.position, Vector3Scale(a->entity3D.scale, 0.5f));
    }

    // Calculate bounding box for entity B
    ModelInfo* modelInfoB = get_model_info(b->entity3D.model_id);
    if (modelInfoB) {
        // Use model bounding box and adjust to entity's position
        BoundingBox modelBoxB = get_model_bounding_box(modelInfoB->model);
        Vector3 bboxCenterB = Vector3Scale(Vector3Add(modelBoxB.min, modelBoxB.max), 0.5f);
        bboxB.min = Vector3Add(Vector3Subtract(modelBoxB.min, bboxCenterB), b->entity3D.position);
        bboxB.max = Vector3Add(Vector3Subtract(modelBoxB.max, bboxCenterB), b->entity3D.position);
    } else {
        // Fallback to scale-based bounding box
        bboxB.min = Vector3Subtract(b->entity3D.position, Vector3Scale(b->entity3D.scale, 0.5f));
        bboxB.max = Vector3Add(b->entity3D.position, Vector3Scale(b->entity3D.scale, 0.5f));
    }

    // Check for overlap on all three axes
    if (bboxA.max.x < bboxB.min.x || bboxA.min.x > bboxB.max.x) return false;
    if (bboxA.max.y < bboxB.min.y || bboxA.min.y > bboxB.max.y) return false;
    if (bboxA.max.z < bboxB.min.z || bboxA.min.z > bboxB.max.z) return false;

    return true;
}

// Sphere Collision Detection
bool Physics_CheckCollisionSphere(const t_Entity* a, const t_Entity* b) {
    if (!a || !b) return false;

    // Calculate radii based on scale (assuming uniform scaling)
    float radiusA = a->entity3D.scale.x * 0.5f; // Assuming scale.x = scale.y = scale.z
    float radiusB = b->entity3D.scale.x * 0.5f;

    // Calculate distance between centers
    Vector3 diff = Vector3Subtract(a->entity3D.position, b->entity3D.position);
    float distance = Vector3LengthCustom(diff);

    // Check if distance is less than sum of radii
    return distance <= (radiusA + radiusB);
}

// Ray-Entity Intersection (supports AABB and Sphere)
bool Physics_RayIntersectsEntity(const t_Entity* entity, Ray ray, RayCollision* collision) {
    if (!entity) return false;

    if (entity->collision_type == COLLISION_AABB) {
        // Define AABB based on entity's position and scale
        BoundingBox bbox;
        bbox.min = Vector3Subtract(entity->entity3D.position, Vector3Scale(entity->entity3D.scale, 0.5f));
        bbox.max = Vector3Add(entity->entity3D.position, Vector3Scale(entity->entity3D.scale, 0.5f));

        *collision = GetRayCollisionBox(ray, bbox);
        return collision->hit;
    }
    else if (entity->collision_type == COLLISION_SPHERE) {
        // Define Sphere based on entity's position and radius
        float radius = entity->entity3D.scale.x * 0.5f; // Assuming uniform scaling
        *collision = GetRayCollisionSphere(ray, entity->entity3D.position, radius);
        return collision->hit;
    }
    // Add more collision types as needed

    return false;
}

// Collision Resolution by Simple Separation along Y-axis (for AABB)
void Physics_ResolveCollision(t_Entity* a, t_Entity* b) {
    if (!a || !b) return;
    if (a->is_static && b->is_static) return;

    BoundingBox bboxA, bboxB;

    // Calculate bounding box for entity A
    ModelInfo* modelInfoA = get_model_info(a->entity3D.model_id);
    if (modelInfoA) {
        BoundingBox modelBoxA = get_model_bounding_box(modelInfoA->model);
        Vector3 bboxCenterA = Vector3Scale(Vector3Add(modelBoxA.min, modelBoxA.max), 0.5f);
        bboxA.min = Vector3Add(Vector3Subtract(modelBoxA.min, bboxCenterA), a->entity3D.position);
        bboxA.max = Vector3Add(Vector3Subtract(modelBoxA.max, bboxCenterA), a->entity3D.position);
    } else {
        bboxA.min = Vector3Subtract(a->entity3D.position, Vector3Scale(a->entity3D.scale, 0.5f));
        bboxA.max = Vector3Add(a->entity3D.position, Vector3Scale(a->entity3D.scale, 0.5f));
    }

    // Calculate bounding box for entity B
    ModelInfo* modelInfoB = get_model_info(b->entity3D.model_id);
    if (modelInfoB) {
        BoundingBox modelBoxB = get_model_bounding_box(modelInfoB->model);
        Vector3 bboxCenterB = Vector3Scale(Vector3Add(modelBoxB.min, modelBoxB.max), 0.5f);
        bboxB.min = Vector3Add(Vector3Subtract(modelBoxB.min, bboxCenterB), b->entity3D.position);
        bboxB.max = Vector3Add(Vector3Subtract(modelBoxB.max, bboxCenterB), b->entity3D.position);
    } else {
        bboxB.min = Vector3Subtract(b->entity3D.position, Vector3Scale(b->entity3D.scale, 0.5f));
        bboxB.max = Vector3Add(b->entity3D.position, Vector3Scale(b->entity3D.scale, 0.5f));
    }

    // Resolve collision along the minimum penetration axis (similar to existing logic)
}

// Collision Resolution using Impulse-Based Response
void Physics_ResolveCollisionImpulse(t_Entity* a, t_Entity* b) {
    if (!a || !b) return;
    if (a->is_static && b->is_static) return;

    // Calculate relative velocity
    Vector3 relativeVelocity = Vector3Subtract(b->entity3D.velocity, a->entity3D.velocity);

    // Determine collision normal based on AABB penetration
    Vector3 normal = {0.0f, 0.0f, 0.0f};
    float penetration = FLT_MAX; // Initialize with maximum float value

    if (a->collision_type == COLLISION_AABB && b->collision_type == COLLISION_AABB) {
        Vector3 aMin = Vector3Subtract(a->entity3D.position, Vector3Scale(a->entity3D.scale, 0.5f));
        Vector3 aMax = Vector3Add(a->entity3D.position, Vector3Scale(a->entity3D.scale, 0.5f));
        Vector3 bMin = Vector3Subtract(b->entity3D.position, Vector3Scale(b->entity3D.scale, 0.5f));
        Vector3 bMax = Vector3Add(b->entity3D.position, Vector3Scale(b->entity3D.scale, 0.5f));

        // Calculate penetration depth on each axis
        float penX1 = aMax.x - bMin.x;
        float penX2 = bMax.x - aMin.x;
        float penY1 = aMax.y - bMin.y;
        float penY2 = bMax.y - aMin.y;
        float penZ1 = aMax.z - bMin.z;
        float penZ2 = bMax.z - aMin.z;

        // Find the axis with the minimal penetration
        float minPen = penX1;
        normal = (Vector3){1.0f, 0.0f, 0.0f}; // X-axis

        if (penX2 < minPen) {
            minPen = penX2;
            normal = (Vector3){-1.0f, 0.0f, 0.0f}; // -X-axis
        }
        if (penY1 < minPen) {
            minPen = penY1;
            normal = (Vector3){0.0f, 1.0f, 0.0f}; // Y-axis
        }
        if (penY2 < minPen) {
            minPen = penY2;
            normal = (Vector3){0.0f, -1.0f, 0.0f}; // -Y-axis
        }
        if (penZ1 < minPen) {
            minPen = penZ1;
            normal = (Vector3){0.0f, 0.0f, 1.0f}; // Z-axis
        }
        if (penZ2 < minPen) {
            minPen = penZ2;
            normal = (Vector3){0.0f, 0.0f, -1.0f}; // -Z-axis
        }

        penetration = minPen;
    } else {
        // Fallback to Y-axis normal if not AABB
        normal = (Vector3){0.0f, 1.0f, 0.0f};
        // Estimate penetration based on some method or set a default value
        penetration = 0.01f; // Example value
    }

    // Calculate velocity along the normal
    float velocityAlongNormal = Vector3DotProduct(relativeVelocity, normal);

    // Do not resolve if velocities are separating
    if (velocityAlongNormal > 0) return;

    // Calculate restitution (bounciness)
    float restitution = 0.5f; // Adjust as needed (0 = inelastic, 1 = perfectly elastic)

    // Calculate impulse scalar
    float j = -(1 + restitution) * velocityAlongNormal;
    j /= (a->inverseMass + b->inverseMass);

    // Apply impulse
    Vector3 impulse = Vector3Scale(normal, j);
    if (!a->is_static) {
        a->entity3D.velocity = Vector3Subtract(a->entity3D.velocity, Vector3Scale(impulse, a->inverseMass));
    }
    if (!b->is_static) {
        b->entity3D.velocity = Vector3Add(b->entity3D.velocity, Vector3Scale(impulse, b->inverseMass));
    }

    // Positional correction to avoid sinking (optional)
    const float percent = 0.8f; // Penetration percentage to correct
    const float slop = 0.01f;   // Penetration allowance

    float correctionMagnitude = (penetration - slop) > 0.0f ? (penetration - slop) / (a->inverseMass + b->inverseMass) * percent : 0.0f;
    if (correctionMagnitude > 0.0f) {
        Vector3 correction = Vector3Scale(normal, correctionMagnitude);
        if (!a->is_static) {
            a->entity3D.position = Vector3Subtract(a->entity3D.position, Vector3Scale(correction, a->inverseMass));
        }
        if (!b->is_static) {
            b->entity3D.position = Vector3Add(b->entity3D.position, Vector3Scale(correction, b->inverseMass));
        }
    }
}


// Raycasting function to detect the first collision in a list of entities
RayHit Physics_Raycast(t_Entity* entities[], int entityCount, Ray ray) {
    RayHit closestHit = {0};
    closestHit.collision.distance = FLT_MAX;

    for (int i = 0; i < entityCount; i++) {
        t_Entity* entity = entities[i];
        if (!entity->is_active) continue;

        RayCollision currentHit;
        bool hit = Physics_RayIntersectsEntity(entity, ray, &currentHit);
        if (hit && currentHit.distance < closestHit.collision.distance) {
            closestHit.entity = entity;
            closestHit.collision = currentHit;
        }
    }

    return closestHit;
}
