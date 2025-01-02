// physics.c
#include <mato_physics.h>
#include <pthread.h>
#include <omp.h>
#include <math.h>
#include <float.h>

#ifdef _MSC_VER
    __declspec(align(16)) const __m128 HALF_PS  = {0.5f, 0.5f, 0.5f, 0.0f};
    __declspec(align(16)) const __m128 ZERO_PS  = {0.0f, 0.0f, 0.0f, 0.0f};
    __declspec(align(16)) const __m128 ONE_PS   = {1.0f, 1.0f, 1.0f, 1.0f};
    __declspec(align(16)) const __m128 NEGATE_PS = {-0.0f, -0.0f, -0.0f, -0.0f}; // Define NEGATE_PS
#elif defined(__GNUC__) || defined(__clang__)
    const __m128 HALF_PS  __attribute__((aligned(16))) = {0.5f, 0.5f, 0.5f, 0.0f};
    const __m128 ZERO_PS  __attribute__((aligned(16))) = {0.0f, 0.0f, 0.0f, 0.0f};
    const __m128 ONE_PS   __attribute__((aligned(16))) = {1.0f, 1.0f, 1.0f, 1.0f};
    const __m128 NEGATE_PS __attribute__((aligned(16))) = {-0.0f, -0.0f, -0.0f, -0.0f}; // Define NEGATE_PS
#else
    const __m128 HALF_PS  = {0.5f, 0.5f, 0.5f, 0.0f};
    const __m128 ZERO_PS  = {0.0f, 0.0f, 0.0f, 0.0f};
    const __m128 ONE_PS   = {1.0f, 1.0f, 1.0f, 1.0f};
    const __m128 NEGATE_PS = {-0.0f, -0.0f, -0.0f, -0.0f}; // Define NEGATE_PS
#endif

static ALWAYS_INLINE bool CanRunCollision(Vector3 a, Vector3 b) {
    // TODO: Eventually we should check for point from mesh to mesh
    return Vector3Distance(a, b) < MAX_COLLISION_CHECK_DISTANCE;
}

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
    if (length < EPSILON) return (Vector3){0.0f, 0.0f, 0.0f};
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
    entity->is_grounded = false;
    entity->in_water = false;
    entity->is_active = true;
    entity->has_moved = true;

    // Initialize mutex for the entity
    pthread_mutex_init(&entity->mutex, NULL);
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

    Vector3 startPos = entity->entity3D.position;
    entity->has_moved = true;

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
    if (!entity->is_grounded && !entity->in_water) {
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

            float friction_magnitude = normal_force * BASE_FRICTION;

            // Calculate friction force vector
            Vector3 friction_force = Vector3Scale(friction_dir, -friction_magnitude);

            // Add friction force to the force accumulator
            entity->forceAccum = Vector3Add(entity->forceAccum, friction_force);

            // **Prevent Over-friction (Optional)**
            // Ensure that friction does not reverse the velocity direction
            // Compute the potential velocity change due to friction
            float potential_velocity_change = friction_magnitude * deltaTime / mass;

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

    if (Vector3LengthCustom(entity->entity3D.velocity) != 0) {
        // printf("%s: (%f,%f,%f)\n", entity->entity_name, entity->entity3D.velocity.x,entity->entity3D.velocity.y,entity->entity3D.velocity.z);
    }

    if (Vector3LengthCustom(entity->entity3D.velocity) < 0.66f && entity->is_grounded) {
        entity->entity3D.velocity = (Vector3){ 0.0f, 0.0f, 0.0f };
    }

    entity->forceAccum = (Vector3){0.0f, 0.0f, 0.0f};
    if (Vector3Distance(entity->entity3D.position, startPos) < EPSILON) {
        entity->has_moved = false;
    }
}

// General Collision Detection between two entities
bool Physics_CheckCollision(const t_Entity* a, const t_Entity* b) {
    if (!a || !b) return false;

    if ((a->is_static || b->is_static) || (has_tag(a, "NO_COLLISION") != -1 || has_tag(b, "NO_COLLISION") != -1)) return false;

    if (!CanRunCollision(a->entity3D.position, b->entity3D.position)) return false;

    // Determine collision type combinations
    if (a->collision_type == COLLISION_AABB && b->collision_type == COLLISION_AABB) {
        return Physics_CheckCollisionAABB_SIMD(a, b);
    }
    else if (a->collision_type == COLLISION_SPHERE && b->collision_type == COLLISION_SPHERE) {
        return Physics_CheckCollisionSphere(a, b);
    }
    // Add more collision type combinations as needed
    else {
        // Handle mixed collision types or unsupported types
        // For simplicity, default to AABB collision
        return Physics_CheckCollisionAABB_SIMD(a, b);
    }
}

static inline void ComputeTransformedBoundingBox_SIMD(const t_Entity* entity, __m128* bbox_min, __m128* bbox_max) {
    // Retrieve the Raylib Model
    ModelInfo* minfo = get_model_info(entity->entity3D.model_id);

    // Initialize entity position vector
    __m128 entityPos = _mm_setr_ps(
        entity->entity3D.position.x,
        entity->entity3D.position.y,
        entity->entity3D.position.z,
        0.0f // w-component unused
    );

    // Load scaling vector
    __m128 scale = _mm_setr_ps(
        entity->entity3D.scale.x,
        entity->entity3D.scale.y,
        entity->entity3D.scale.z,
        1.0f // w-component set to 1.0f
    );

    if (minfo) {
        // Load bounding box min and max
        __m128 boxMin = _mm_setr_ps(
            minfo->full_box.min.x,
            minfo->full_box.min.y,
            minfo->full_box.min.z,
            0.0f
        );
        __m128 boxMax = _mm_setr_ps(
            minfo->full_box.max.x,
            minfo->full_box.max.y,
            minfo->full_box.max.z,
            0.0f
        );

        // Compute scaled bounding box min and max
        __m128 scaledBoxMin = _mm_mul_ps(boxMin, scale);
        __m128 scaledBoxMax = _mm_mul_ps(boxMax, scale);

        // Compute the final bounding boxes by adding the entity position
        *bbox_min = _mm_add_ps(scaledBoxMin, entityPos);
        *bbox_max = _mm_add_ps(scaledBoxMax, entityPos);
    } else {
        // Use the entity's position and scale to define the bounding box
        __m128 halfScale = _mm_mul_ps(scale, _mm_set_ps1(0.5f));
        *bbox_min = _mm_sub_ps(entityPos, halfScale);
        *bbox_max = _mm_add_ps(entityPos, halfScale);
    }
}

bool Physics_CheckCollisionAABB_SIMD(const t_Entity* a, const t_Entity* b) {
    if (a == NULL || b == NULL) return false;

    __m128 bboxA_min, bboxA_max;
    __m128 bboxB_min, bboxB_max;

    // Compute transformed bounding boxes for both entities
    ComputeTransformedBoundingBox_SIMD(a, &bboxA_min, &bboxA_max);
    ComputeTransformedBoundingBox_SIMD(b, &bboxB_min, &bboxB_max);

    // Check for separation on any axis using SIMD
    __m128 cmpMin = _mm_cmpgt_ps(bboxA_min, bboxB_max); // A_min > B_max
    __m128 cmpMax = _mm_cmpgt_ps(bboxB_min, bboxA_max); // B_min > A_max
    __m128 cmpResult = _mm_or_ps(cmpMin, cmpMax);

    // Extract the comparison results: if any axis does not overlap, return false
    return (_mm_movemask_ps(cmpResult) == 0);
}

// AABB Collision Detection
bool Physics_CheckCollisionAABB(const t_Entity* a, const t_Entity* b) {
    if (!a || !b) return false;

    BoundingBox bboxA, bboxB;

    // Calculate bounding box for entity A
    ModelInfo* modelInfoA = get_model_info(a->entity3D.model_id);
    if (modelInfoA) {
        // Use model bounding box and adjust to entity's position
        BoundingBox modelBoxA = modelInfoA->full_box;
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
        BoundingBox modelBoxB = modelInfoB->full_box;
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

    // Lock both entities to prevent concurrent modifications
    // To avoid deadlocks, always lock in a consistent order based on memory address
    if (a < b) {
        pthread_mutex_lock(&a->mutex);
        pthread_mutex_lock(&b->mutex);
    } else {
        pthread_mutex_lock(&b->mutex);
        pthread_mutex_lock(&a->mutex);
    }

    // Calculate bounding boxes
    BoundingBox bboxA, bboxB;

    // Calculate bounding box for entity A
    ModelInfo* modelInfoA = get_model_info(a->entity3D.model_id);
    if (modelInfoA) {
        BoundingBox modelBoxA = modelInfoA->full_box;
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
        BoundingBox modelBoxB = modelInfoB->full_box;
        Vector3 bboxCenterB = Vector3Scale(Vector3Add(modelBoxB.min, modelBoxB.max), 0.5f);
        bboxB.min = Vector3Add(Vector3Subtract(modelBoxB.min, bboxCenterB), b->entity3D.position);
        bboxB.max = Vector3Add(Vector3Subtract(modelBoxB.max, bboxCenterB), b->entity3D.position);
    } else {
        bboxB.min = Vector3Subtract(b->entity3D.position, Vector3Scale(b->entity3D.scale, 0.5f));
        bboxB.max = Vector3Add(b->entity3D.position, Vector3Scale(b->entity3D.scale, 0.5f));
    }

    // Simple separation along Y-axis (for demonstration)
    float penetration = 0.0f;

    if (bboxA.max.y > bboxB.min.y && bboxA.min.y < bboxB.max.y) {
        // Calculate penetration depth
        penetration = bboxA.max.y - bboxB.min.y;

        // Adjust positions to resolve collision
        if (!a->is_static && !b->is_static) {
            Vector3 correction = {0.0f, penetration / 2.0f, 0.0f};
            a->entity3D.position.y -= correction.y;
            b->entity3D.position.y += correction.y;
        }
        else if (!a->is_static) {
            Vector3 correction = {0.0f, penetration, 0.0f};
            a->entity3D.position.y -= correction.y;
        }
        else if (!b->is_static) {
            Vector3 correction = {0.0f, penetration, 0.0f};
            b->entity3D.position.y += correction.y;
        }
    }

    // Unlock both entities
    pthread_mutex_unlock(&a->mutex);
    pthread_mutex_unlock(&b->mutex);
}

// Collision Resolution using Impulse-Based Response
void Physics_ResolveCollisionImpulse(t_Entity* a, t_Entity* b) {
    if (!a || !b) return;
    if (a->is_static && b->is_static) return;

    // Lock both entities to prevent concurrent modifications
    // To avoid deadlocks, always lock in a consistent order based on memory address
    if (a < b) {
        pthread_mutex_lock(&a->mutex);
        pthread_mutex_lock(&b->mutex);
    } else {
        pthread_mutex_lock(&b->mutex);
        pthread_mutex_lock(&a->mutex);
    }

    // Calculate relative velocity
    Vector3 relativeVelocity = Vector3Subtract(b->entity3D.velocity, a->entity3D.velocity);

    // Determine collision normal based on AABB penetration
    Vector3 normal = {0.0f, 1.0f, 0.0f};
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
    if (velocityAlongNormal > 0) {
        pthread_mutex_unlock(&a->mutex);
        pthread_mutex_unlock(&b->mutex);
        return;
    }

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

    // Unlock both entities
    pthread_mutex_unlock(&a->mutex);
    pthread_mutex_unlock(&b->mutex);
}

// Raycasting function to detect the first collision in a list of entities
RayHit Physics_Raycast(t_Entity* entities[], int entityCount, Ray ray) {
    RayHit closestHit;
    closestHit.collision.distance = FLT_MAX;
    closestHit.entity = NULL;

    // Temporary storage for thread-local closest hits
    int num_threads = omp_get_max_threads();
    RayHit* threadClosestHits = (RayHit*)malloc(sizeof(RayHit) * num_threads);
    if (!threadClosestHits) return closestHit;

    for (int t = 0; t < num_threads; t++) {
        threadClosestHits[t].collision.distance = FLT_MAX;
        threadClosestHits[t].entity = NULL;
    }

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        RayHit localClosestHit;
        localClosestHit.collision.distance = FLT_MAX;
        localClosestHit.entity = NULL;

        #pragma omp for nowait
        for (int i = 0; i < entityCount; i++) {
            t_Entity* entity = entities[i];
            if (!entity->is_active) continue;

            RayCollision currentHit;
            bool hit = Physics_RayIntersectsEntity(entity, ray, &currentHit);
            if (hit && currentHit.distance < localClosestHit.collision.distance) {
                localClosestHit.entity = entity;
                localClosestHit.collision = currentHit;
            }
        }

        // Store the local closest hit for this thread
        threadClosestHits[thread_id] = localClosestHit;
    }

    // Find the closest hit among all threads
    for (int t = 0; t < num_threads; t++) {
        if (threadClosestHits[t].collision.distance < closestHit.collision.distance) {
            closestHit = threadClosestHits[t];
        }
    }

    free(threadClosestHits);
    return closestHit;
}

// Physics Update Function with Multithreading
void Physics_UpdateAll(t_Entity* entities[], int entityCount, float deltaTime) {
    int integrates = 0;
    
    for (int i = 0; i < entityCount; i++) {
        Physics_Integrate(entities[i], deltaTime);
        integrates++;
    }

    int col_checks = 0;
    // Iterate over all unique pairs
    #pragma omp parallel for reduction(+:col_checks) schedule(dynamic)
    for (int i = 0; i < entityCount; i++) {
        t_Entity* entityA = entities[i];
        if (!entityA->is_active) continue; // Early exit if entityA is inactive or static

        for (int j = i + 1; j < entityCount; j++) {
            t_Entity* entityB = entities[j];
            if (!entityB->is_active || !entityB->has_moved) continue; // Skip inactive entityB
            if (!CanRunCollision(entityA->entity3D.position, entityB->entity3D.position)) continue;
            {
                

                if (Physics_CheckCollision(entityA, entityB)) {
                    // It's assumed that Physics_ResolveCollisionImpulse is thread-safe
                    Physics_ResolveCollisionImpulse(entityA, entityB);
                }
            }
            col_checks++;
        }
    }
    
    

    // printf("Integrates: %d\n", integrates);
    // printf("Collision checks: %d\n", col_checks);

}

// Cleanup function to destroy mutexes (Call during shutdown)
void Physics_CleanupEntities(t_Entity* entities[], int entityCount) {
    for (int i = 0; i < entityCount; i++) {
        if (entities[i]) {
            pthread_mutex_destroy(&entities[i]->mutex);
        }
    }
}