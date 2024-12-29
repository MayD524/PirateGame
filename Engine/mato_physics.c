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
    if (entity->is_static) return; // No integration for static entities

    if (entity->entity3D.position.y <= (entity->entity3D.scale.y / 2.0f) + EPSILON) {
        entity->entity3D.position.y = (entity->entity3D.scale.y / 2.0f) + EPSILON; // Clamp to ground level
        entity->entity3D.velocity.y = 0.0f; // Reset vertical velocity
        if (!entity->is_grounded) {
            entity->is_grounded = true;
            // Optional: Trigger grounded-related callbacks
            if (entity->on_grounded) {
                entity->on_grounded(entity);
            }
        }
    } else {
        // Entity is above ground
        if (entity->is_grounded) {
            entity->is_grounded = false;
            // Optional: Trigger airborne-related callbacks
            if (entity->on_airborne) {
                entity->on_airborne(entity);
            }
        }
    }

    if (!entity->is_grounded) {
        entity->forceAccum = Vector3Add(entity->forceAccum, GRAVITY);
    }

    Vector3 totalForce = Vector3Add(entity->forceAccum, entity->acceleration); // Include any additional accelerations
    Vector3 resultingAcc = Vector3Scale(totalForce, entity->inverseMass);

    entity->entity3D.velocity = Vector3Add(entity->entity3D.velocity, Vector3Scale(resultingAcc, deltaTime));

    entity->entity3D.position = Vector3Add(entity->entity3D.position, Vector3Scale(entity->entity3D.velocity, deltaTime));

    // Adjust for entity height (assuming position.y is at the center)
    if (entity->entity3D.position.y < (entity->entity3D.scale.y / 2.0f)) {
        entity->entity3D.position.y = (entity->entity3D.scale.y / 2.0f); // Clamp position to y = entity height / 2
        entity->entity3D.velocity.y = 0.0f; // Reset vertical velocity
        if (!entity->is_grounded) {
            entity->is_grounded = true;
            // Optional: Trigger grounded-related callbacks
            if (entity->on_grounded) {
                entity->on_grounded(entity);
            }
        }
    }

    // 7. Reset Force Accumulator for the Next Frame
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

    // Calculate min and max for entity A
    Vector3 aMin = Vector3Subtract(a->entity3D.position, Vector3Scale(a->entity3D.scale, 0.5f));
    Vector3 aMax = Vector3Add(a->entity3D.position, Vector3Scale(a->entity3D.scale, 0.5f));

    // Calculate min and max for entity B
    Vector3 bMin = Vector3Subtract(b->entity3D.position, Vector3Scale(b->entity3D.scale, 0.5f));
    Vector3 bMax = Vector3Add(b->entity3D.position, Vector3Scale(b->entity3D.scale, 0.5f));

    // Check for overlap on all three axes
    if (aMax.x < bMin.x || aMin.x > bMax.x) return false;
    if (aMax.y < bMin.y || aMin.y > bMax.y) return false;
    if (aMax.z < bMin.z || aMin.z > bMax.z) return false;

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

    // For simplicity, resolve only AABB collisions
    if (a->collision_type != COLLISION_AABB || b->collision_type != COLLISION_AABB) return;

    // Calculate the overlap on Y-axis
    float aHalf = a->entity3D.scale.y / 2.0f;
    float bHalf = b->entity3D.scale.y / 2.0f;
    float distance = a->entity3D.position.y - b->entity3D.position.y;
    float overlap = (aHalf + bHalf) - fabsf(distance);

    if (overlap > 0.0f) {
        // Simple separation along Y-axis
        float separation = overlap / (a->inverseMass + b->inverseMass);
        if (!a->is_static) {
            a->entity3D.position.y += separation * a->inverseMass * ((distance < 0.0f) ? -1.0f : 1.0f);
        }
        if (!b->is_static) {
            b->entity3D.position.y -= separation * b->inverseMass * ((distance < 0.0f) ? -1.0f : 1.0f);
        }

        // Simple collision response: zero the Y velocity if moving towards each other
        if (a->entity3D.velocity.y < 0.0f && !a->is_static) a->entity3D.velocity.y = 0.0f;
        if (b->entity3D.velocity.y > 0.0f && !b->is_static) b->entity3D.velocity.y = 0.0f;
    }
}

// Collision Resolution using Impulse-Based Response
void Physics_ResolveCollisionImpulse(t_Entity* a, t_Entity* b) {
    if (!a || !b) return;
    if (a->is_static && b->is_static) return;

    // Calculate relative velocity
    Vector3 relativeVelocity = Vector3Subtract(b->entity3D.velocity, a->entity3D.velocity);

    // Calculate the normal vector (assuming collision on Y-axis)
    Vector3 normal = (Vector3){0.0f, 1.0f, 0.0f}; // Upwards

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

    // Calculate penetration depth
    float penetration = 0.0f;
    if (a->collision_type == COLLISION_AABB && b->collision_type == COLLISION_AABB) {
        Vector3 aMin = Vector3Subtract(a->entity3D.position, Vector3Scale(a->entity3D.scale, 0.5f));
        Vector3 aMax = Vector3Add(a->entity3D.position, Vector3Scale(a->entity3D.scale, 0.5f));
        Vector3 bMin = Vector3Subtract(b->entity3D.position, Vector3Scale(b->entity3D.scale, 0.5f));
        Vector3 bMax = Vector3Add(b->entity3D.position, Vector3Scale(b->entity3D.scale, 0.5f));

        // Calculate penetration on Y-axis
        float penY1 = aMax.y - bMin.y;
        float penY2 = bMax.y - aMin.y;
        penetration = (penY1 < penY2) ? penY1 : penY2;
    }

    penetration = penetration - slop;
    if (penetration > 0.0f) {
        Vector3 correction = Vector3Scale(normal, (penetration / (a->inverseMass + b->inverseMass)) * percent);
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
