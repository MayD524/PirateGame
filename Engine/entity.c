#include <entity.h>

t_EntitySystem* g_entity_system;

#pragma region Shape drawing

ShapeType getShapeType(const char *shapeStr) {
    if (strcmp(shapeStr, "RECT") == 0) return RECTANGLE;
    if (strcmp(shapeStr, "CIRC") == 0) return CIRCLE;
    if (strcmp(shapeStr, "TRI") == 0) return TRIANGLE;
    return UNKNOWN;
}

Shape parseShapeString(const char *str) {
    Shape shape;
    char shapeStr[5];
    char filledStr[6];
    int r, g, b, a;

    sscanf(str, "SHAPE:%4[^:]:%f,%f:%f,%f:%f,%f:%d,%d,%d,%d:%d:%5s",
           shapeStr, &shape.position1.x, &shape.position1.y,
           &shape.position2.x, &shape.position2.y, &shape.position3.x,
           &shape.position3.y, &r, &g, &b, &a, &shape.thickness, filledStr);

    // Assign shape type
    shape.type = getShapeType(shapeStr);

    // Assign color
    shape.color = (Color){ r, g, b, a };

    // Handle boolean parsing
    shape.filled = (strcmp(filledStr, "true") == 0);

    // Default values for optional parameters based on shape
    if (shape.type == RECTANGLE) {
        shape.position3 = (Vector2){0, 0};
    } else if (shape.type == CIRCLE) {
        shape.position2 = (Vector2){shape.position2.x, 0};
        shape.position3 = (Vector2){0, 0};
    }

    return shape;
}

void printShape(const Shape *shape) {
    const char *shapeTypeStr = (shape->type == RECTANGLE) ? "RECTANGLE" :
                                (shape->type == CIRCLE) ? "CIRCLE" :
                                (shape->type == TRIANGLE) ? "TRIANGLE" : "UNKNOWN";

    printf("--------------------------\n");
    printf("Shape: %s\n", shapeTypeStr);
    printf("Position 1: (%.2f, %.2f)\n", shape->position1.x, shape->position1.y);
    if (shape->type == RECTANGLE || shape->type == TRIANGLE) {
        printf("Position 2: (%.2f, %.2f)\n", shape->position2.x, shape->position2.y);
    } else {
        printf("Radius %.2f\n", shape->position2.x);
    }
    if (shape->type == TRIANGLE) {
        printf("Position 3: (%.2f, %.2f)\n", shape->position3.x, shape->position3.y);
    }
    printf("Color: (%d, %d, %d, %d)\n", shape->color.r, shape->color.g, shape->color.b, shape->color.a);
    printf("Thickness: %d\n", shape->thickness);
    printf("Filled: %s\n", shape->filled ? "true" : "false");
    printf("--------------------------\n");
}

// void drawShape(t_Entity* ent, Shape *shape) {
//     switch (shape->type) {
//         case RECTANGLE: {
//             Vector2 pos1 = {
//                 ent->data.entity2D.position.x + shape->position1.x * ent->data.entity2D.scale,
//                 ent->data.entity2D.position.y + shape->position1.y * ent->data.entity2D.scale
//             };
//             if (shape->filled) {
//                 DrawRectangle(
//                     pos1.x, pos1.y,
//                     (shape->position2.x - shape->position1.x) * ent->data.entity2D.scale,
//                     (shape->position2.y - shape->position1.y) * ent->data.entity2D.scale,
//                     shape->color
//                 );
//             } else {
//                 DrawRectangleLinesEx(
//                     (Rectangle){
//                         pos1.x, pos1.y,
//                         (shape->position2.x - shape->position1.x) * ent->data.entity2D.scale,
//                         (shape->position2.y - shape->position1.y) * ent->data.entity2D.scale
//                     },
//                     shape->thickness, shape->color
//                 );
//             }
//             break;
//         }
//         case CIRCLE: {
//             Vector2 center = {
//                 ent->data.entity2D.position.x + shape->position1.x * ent->data.entity2D.scale,
//                 ent->data.entity2D.position.y + shape->position1.y * ent->data.entity2D.scale
//             };
//             if (shape->filled) {
//                 DrawCircleV(center, shape->position2.x * ent->data.entity2D.scale, shape->color);
//             } else {
//                 DrawCircleLines(center.x, center.y, shape->position2.x * ent->data.entity2D.scale, shape->color);
//             }
//             break;
//         }
//         case TRIANGLE: {
//             Vector2 pos1 = {
//                 ent->data.entity2D.position.x + shape->position1.x * ent->data.entity2D.scale,
//                 ent->data.entity2D.position.y + shape->position1.y * ent->data.entity2D.scale
//             };
//             Vector2 pos2 = {
//                 ent->data.entity2D.position.x + shape->position2.x * ent->data.entity2D.scale,
//                 ent->data.entity2D.position.y + shape->position2.y * ent->data.entity2D.scale
//             };
//             Vector2 pos3 = {
//                 ent->data.entity2D.position.x + shape->position3.x * ent->data.entity2D.scale,
//                 ent->data.entity2D.position.y + shape->position3.y * ent->data.entity2D.scale
//             };
//             if (shape->filled) {
//                 DrawTriangle(pos1, pos2, pos3, shape->color);
//             } else {
//                 DrawTriangleLines(pos1, pos2, pos3, shape->color);
//             }
//             break;
//         }
//         default:
//             break;
//     }
// }

#pragma endregion


int are_floats_equal(float a, float b) {
    return fabs(a - b) < FLT_EPSILON;
}

#pragma region Public methods for entity system
t_EntitySystem* create_entity_system() {
    t_EntitySystem* entity_system = (t_EntitySystem*) safe_malloc(sizeof(t_EntitySystem));
    if (!entity_system) {
        return NULL; // Handle memory allocation failure
    }

    entity_system->entities = (t_Entity**) safe_malloc(INITIAL_ENTITY_CAPACITY * sizeof(t_Entity*));
    if (!entity_system->entities) {
        free(entity_system);
        return NULL; // Handle memory allocation failure
    }

    entity_system->num_entities = 0;
    entity_system->capacity_entities = INITIAL_ENTITY_CAPACITY;

    entity_system->textures = (t_Texture*) safe_malloc(INITIAL_TEXTURE_CAPACITY * sizeof(t_Texture));
    if (!entity_system->textures) {
        free(entity_system->entities);
        free(entity_system);
        return NULL; // Handle memory allocation failure
    }

    entity_system->num_textures = 0;
    entity_system->capacity_textures = INITIAL_TEXTURE_CAPACITY;

    return entity_system;
}

int next_free_entity_slot(t_EntitySystem* entity_system, PriorityRank priority_rank) {
    // Search for an available slot
    for (int i = 0; i < entity_system->capacity_entities; ++i) {
        t_Entity *entity = entity_system->entities[i];
        if (!entity) {
            return i;
        }

        // Optional: Ensure the pointer is valid and not freed memory
        if ((uintptr_t)entity < 0x1000 || (uintptr_t)entity == 0xbebebebebebebebe) { // Check for very low (invalid) addresses
            fprintf(stderr, "Invalid pointer at %d\n", i);
            continue;
        }

        // Check if the entity is inactive or has a lower priority
        if (!entity->is_active || entity->priority_rank < priority_rank) {
            return i; // Found an available slot
        }
    }

    // No available slot, resize the entities array
    if (entity_system->num_entities == entity_system->capacity_entities) {
        int new_capacity = entity_system->capacity_entities * ENTITY_RESIZE_FACTOR;
        t_Entity **new_entities = (t_Entity **)safe_realloc(entity_system->entities, new_capacity * sizeof(t_Entity *));
        if (!new_entities) {
            fprintf(stderr, "Failed to allocate memory for entities\n");
            return -1; // Handle memory allocation failure
        }

        entity_system->entities = new_entities;
        
        // Initialize newly allocated pointers to NULL
        for (int i = entity_system->capacity_entities; i < new_capacity; ++i) {
            entity_system->entities[i] = NULL;
        }

        entity_system->capacity_entities = new_capacity;
    }

    // Add a new slot

    return entity_system->num_entities;
}

#pragma region Update


void update(t_EntitySystem* entity_system) {

    Physics_UpdateAll(entity_system->entities, entity_system->num_entities, PHYSICS_TIME);
    // Iterate through all active entities and integrate physics
    for (int i = 0; i < entity_system->num_entities; ++i) {
        t_Entity* e = entity_system->entities[i];
        if (e == NULL || e->entity_name == NULL || !e->is_active) {
            continue;
        }

        // Update lifetime
        e->current_life_time += PHYSICS_TIME;
        if (e->priority_rank != PLAYER && (e->current_life_time > e->max_life_time && e->max_life_time >= 0)) {
            e->current_life_time = 0;
            e->is_active = false;
            if (is_address_good(e->on_destroy)) {
                e->on_destroy(e);
            }
            continue;
        }

        // Handle health
        if (e->health <= 0) {
            printf("%s has died\n", e->entity_name);
            e->is_active = false;
            if (is_address_good(e->on_destroy)) {
                e->on_destroy(e);
            }
            continue;
        }

        // Call entity-specific update
        if (is_address_good(e->update)) {
            e->update(e, PHYSICS_TIME);
        }

    }

    // Handle collisions between entities
    for (int i = 0; i < entity_system->num_entities; ++i) {
        t_Entity* a = entity_system->entities[i];
        if (a == NULL || !a->is_active) continue;

        for (int j = i + 1; j < entity_system->num_entities; ++j) {
            t_Entity* b = entity_system->entities[j];
            if (b == NULL || !b->is_active) continue;

            // Check collision between entity a and entity b
            if (Physics_CheckCollision(a, b)) {
                // Resolve collision using impulse-based method
                // FUTURE ME THIS IS WHAT MAKES THEM CLIMB
                Physics_ResolveCollisionImpulse(a, b);

                // Call collision callbacks if defined
                if (a->on_collision) a->on_collision(b, a);
                if (b->on_collision) b->on_collision(a, b);
            }
        }
    }

    // Optional: Handle additional updates or cleanup after physics and collisions
}

#pragma endregion

#pragma region Rendering
void DrawHitboxes(t_EntitySystem* entity_system, bool enableHitbox) {
    bool drawMultipleBoxes=true;
    if (!enableHitbox) return; // Exit if hitbox drawing is disabled

    for (int i = 0; i < entity_system->num_entities; ++i) {
        t_Entity* entity = entity_system->entities[i];
        if (!entity || !entity->is_active) continue; // Skip inactive or null entities

        // Set hitbox color based on collision type
        Color hitboxColor;
        switch (entity->collision_type) {
            case COLLISION_AABB:
                hitboxColor = RED;
                break;
            case COLLISION_SPHERE:
                hitboxColor = BLUE;
                break;
            default:
                hitboxColor = GRAY;
                break;
        }

        // Draw hitbox based on collision type
        switch (entity->collision_type) {
            case COLLISION_AABB: {
                if (entity->entity3D.model_id != -1) {
                    ModelInfo* minfo = get_model_info(entity->entity3D.model_id);
                    if (minfo) {
                        Model model = minfo->model;

                        if (drawMultipleBoxes) {
                            // Draw multiple bounding boxes (one for each mesh)
                            int meshCount = model.meshCount;
                            BoundingBox* meshBoundingBoxes = minfo->mesh_bounding_boxes;

                            if (meshBoundingBoxes) {
                                for (int j = 0; j < meshCount; ++j) {
                                    BoundingBox modelBox = meshBoundingBoxes[j];

                                    // Scale the bounding box to match the entity's size
                                    Vector3 size = Vector3Subtract(modelBox.max, modelBox.min);
                                    Vector3 scaledSize = Vector3Multiply(size, entity->entity3D.scale);
                                    Vector3 center = Vector3Scale(Vector3Add(modelBox.min, modelBox.max), 0.5f);

                                    // Adjust the bounding box position and scale
                                    Vector3 offset = Vector3Subtract(entity->entity3D.position, center);
                                    BoundingBox transformedBox;
                                    transformedBox.min = Vector3Add(Vector3Subtract(center, Vector3Scale(scaledSize, 0.5f)), offset);
                                    transformedBox.max = Vector3Add(Vector3Add(center, Vector3Scale(scaledSize, 0.5f)), offset);

                                    // Draw the transformed bounding box
                                    DrawBoundingBox(transformedBox, hitboxColor);
                                }
                            }
                        } else {
                            // Draw a single bounding box covering the whole model
                            BoundingBox modelBox = minfo->full_box;

                            // Scale the bounding box to match the entity's size
                            Vector3 size = Vector3Subtract(modelBox.max, modelBox.min);
                            Vector3 scaledSize = Vector3Multiply(size, entity->entity3D.scale);
                            Vector3 center = Vector3Scale(Vector3Add(modelBox.min, modelBox.max), 0.5f);

                            // Adjust the bounding box position and scale
                            Vector3 offset = Vector3Subtract(entity->entity3D.position, center);
                            BoundingBox transformedBox;
                            transformedBox.min = Vector3Add(Vector3Subtract(center, Vector3Scale(scaledSize, 0.5f)), offset);
                            transformedBox.max = Vector3Add(Vector3Add(center, Vector3Scale(scaledSize, 0.5f)), offset);

                            // Draw the transformed bounding box
                            DrawBoundingBox(transformedBox, hitboxColor);
                        }
                    }
                } else {
                    // Fallback to scale-based bounding box
                    Vector3 boxSize = { entity->entity3D.scale.x, entity->entity3D.scale.y, entity->entity3D.scale.z };
                    BoundingBox bbox;
                    bbox.min = Vector3Subtract(entity->entity3D.position, Vector3Scale(boxSize, 0.5f));
                    bbox.max = Vector3Add(entity->entity3D.position, Vector3Scale(boxSize, 0.5f));

                    DrawBoundingBox(bbox, hitboxColor);
                }

                break;
            }

            case COLLISION_SPHERE: {
                // Calculate the radius based on the scale (assuming uniform scale)
                float radius = entity->entity3D.scale.x / 2.0f; // Adjust if non-uniform scaling is used

                // Draw the wireframe sphere
                DrawSphereWires(entity->entity3D.position, radius, 16, 16, hitboxColor);
                break;
            }
            default:
                // Handle other collision types if necessary
                break;
        }
    }
}


bool CheckObjectInFOV(Camera3D camera, Vector3 objectPosition, float screenWidth, float screenHeight)
{

    Vector3 cameraDirection = Vector3Subtract(camera.target, camera.position);
    cameraDirection = Vector3Normalize(cameraDirection);


    Vector3 toObject = Vector3Subtract(objectPosition, camera.position);
    float distance = Vector3Length(toObject);

    if (distance == 0.0f) return true;

    Vector3 toObjectDir = Vector3Scale(toObject, 1.0f / distance); // Normalize

    float dot = Vector3DotProduct(cameraDirection, toObjectDir);

    float aspectRatio = screenWidth / screenHeight;
    float fovRadians = DEG2RAD * camera.fovy;
    float tanHalfFovy = tanf(fovRadians / 2.0f);
    float tanHalfFovx = tanHalfFovy * aspectRatio;

    float fovx = 2.0f * atanf(tanHalfFovx);

    float tanHalfFovd = sqrtf(tanHalfFovy * tanHalfFovy + tanHalfFovx * tanHalfFovx);
    float fovd = 2.0f * atanf(tanHalfFovd);

    float cosHalfFovd = cosf(fovd / 2.0f);

    return dot >= cosHalfFovd;
}

void render(t_EntitySystem* entity_system, TextLabelArray* text_array, Camera camera) {
    char FPS_string[100];
    snprintf(FPS_string, 100, "FPS: %d", GetFPS());
    DrawText(FPS_string, 10, 10, 20, GRAY);

    float screen_width = GetScreenWidth();
    float screen_height = GetScreenHeight();

    for (int i = 0; i < entity_system->num_entities; ++i) {
        t_Entity* e = entity_system->entities[i];

        if (e->entity_name == NULL || !e->is_active || has_tag(e, "HIDDEN") != -1) {
            continue;
        }

        if (e->on_render) {
            e->on_render(e);
        }

        if (e->entity3D.model_id == -1) { continue; }

        ModelInfo* minfo = get_model_info(e->entity3D.model_id);
        Model model = minfo->model;
        if (model.meshCount == 0) { continue; }

        float dist = Vector3Distance(e->entity3D.position, camera.position);
        if (dist > MAX_DRAW_DISTANCE || dist < 0) continue; 

        bool withinMinDistance = dist < MIN_DRAW_DISTANCE;
        if (!withinMinDistance && !CheckObjectInFOV(camera, e->entity3D.position, screen_width, screen_height)) continue;

        float alpha = 1.0f;
        if (dist > FADE_START_DISTANCE) {
            float t = (dist - FADE_START_DISTANCE) / (MAX_DRAW_DISTANCE - FADE_START_DISTANCE);
            alpha = 1.0f - t; // linear fade from 1 down to 0
        }

        // Calculate the model's offset using its bounding box
        BoundingBox bbox = minfo->full_box;
        Vector3 bboxCenter = Vector3Scale(Vector3Add(bbox.min, bbox.max), 0.5f);
        Vector3 adjustedPosition = Vector3Subtract(e->entity3D.position, bboxCenter);

        BeginMode3D(camera);
            DrawTextLabels3DBillboard(text_array, camera, MAX_DRAW_DISTANCE);

            Color originalColor = model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color;

            // Set new color with alpha
            Color tintedColor = originalColor;
            tintedColor.a = (unsigned char)(255 * alpha);
            model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = tintedColor;

            DrawModelEx(
                model,
                adjustedPosition,            // Adjusted position to account for bounding box center
                e->entity3D.rotation_axis,
                e->entity3D.rotation,
                e->entity3D.scale,
                tintedColor
            );
        EndMode3D();
        
    }

    BeginMode3D(camera);
    DrawHitboxes(entity_system, true);
    EndMode3D();
}

#pragma endregion

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif

#pragma region Methods for base entity


t_Entity* create_entity3D(t_EntitySystem* es, char* entity_name, char* modelPath, Vector3 start_position, Vector3 start_velocity, PriorityRank priority_rank, float max_life_time, Vector3 scale3D, float health) {
    int slot = next_free_entity_slot(es, priority_rank);

    if (slot == -1) {
        printf("Failed to allocate memory for new entity.\n");
        printf("This entity will not be created.\n");
        return NULL;
    }

    // Allocate memory for the new entity
    t_Entity *entity = (t_Entity*)safe_malloc(sizeof(t_Entity));
    if (entity == NULL) {
        printf("Memory allocation failed for new entity.\n");
        return NULL;
    }

    // Initialize entity attributes
    entity->entity_name = entity_name;
    entity->is_active = true;
    entity->priority_rank = priority_rank;
    entity->health = health;
    entity->max_life_time = max_life_time;
    entity->current_life_time = 0.0f;

    // Initialize 3D data
    entity->entity3D.position = start_position;
    entity->entity3D.velocity = start_velocity;
    entity->entity3D.scale = scale3D;
    entity->entity3D.rotation = 0.0f;
    entity->entity3D.rotation_axis = (Vector3){0.0f, 1.0f, 0.0f}; // Default rotation axis
    entity->is_grounded = true;

    // Load model if the path exists
    if (access(modelPath, F_OK) == 0) {
        entity->entity3D.model_id = add_model_info(modelPath);

        // Optionally, adjust collision type based on model's properties
        // For simplicity, we'll keep the collision_type passed as a parameter
    } else {
        entity->entity3D.model_id = -1; // No model loaded
    }

    printf("Entity: %s has model id %d\n", entity->entity_name, entity->entity3D.model_id);

    // Initialize physics attributes using the new physics system
    Physics_InitEntity(entity, start_position, scale3D, health, false, COLLISION_AABB);
    // Parameters:
    // - position: start_position
    // - scale: scale3D
    // - mass: health (assuming health correlates with mass; adjust as needed)
    // - is_static: false (dynamic entity)
    // - collision_type: as specified

    // Initialize tags
    t_Tag *root = safe_malloc(sizeof(t_Tag));
    if (root == NULL) {
        printf("Memory allocation failed for entity tags.\n");
        // Handle cleanup if necessary
    } else {
        root->tag_id = 0;
        root->next = NULL;
        root->prev = NULL;
        root->tag = "ENTITY_ROOT";
        entity->root_tag = root;
        entity->num_tags = 0;
    }

    // Assign the entity to the entity system
    es->entities[slot] = entity;
    es->num_entities++;

    return entity;
}


// t_Entity* create_entity(t_EntitySystem* es, char* entity_name, char* texturePath, Vector2 start_position, Vector2 start_velocity, PriorityRank priority_rank, float max_life_time, float scale, float health) {
//     int slot = next_free_entity_slot(es, priority_rank);

//     if (slot == -1) {
//         printf("Failed to allocate memory for new entity. \n");
//         printf("This entity will not be created. \n");
//         return NULL;
//     }

//     t_Entity *entity = es->entities[slot];
//     memset(entity, 0, sizeof(t_Entity)); // Initialize memory to zero

//     entity->entity_name = entity_name;
//     entity->is_active = true;
//     entity->priority_rank = priority_rank;
//     entity->health = health;
//     entity->max_life_time = max_life_time;
//     entity->current_life_time = 0.0f;
//     entity->is_3d = false;

//     // Initialize 2D data
//     entity->entity2D.position = start_position;
//     entity->entity2D.velocity = start_velocity;
//     entity->entity2D.scale = scale;
//     entity->entity2D.sprite_origin = VEC2_ZERO;
//     entity->entity2D.rotation = 0.0f;

//     if (access(texturePath, F_OK) == 0) {
//         int textureId = load_texture(es, texturePath);
//         entity->entity2D.texture = get_texture(es, textureId);

//         Texture2D texture = entity->entity2D.texture;
//         entity->entity2D.sprite_origin = (Vector2){texture.width / 2.0f, texture.height / 2.0f};
//         entity->entity2D.source = (Rectangle){0.0f, 0.0f, texture.width, texture.height};
//         entity->entity2D.shape = (Shape){0};
//     } else {
//         entity->entity2D.texture = (Texture2D){0};
//         entity->entity2D.source = (Rectangle){0};
//         entity->entity2D.shape = (Shape){0};
//     }

//     if (strncmp("SHAPE", texturePath, strlen("SHAPE")) == 0) {
//         entity->entity2D.shape = parseShapeString(texturePath);
//     }

//     // Initialize tags
//     t_Tag *root = safe_malloc(sizeof(t_Tag));
//     root->tag_id = 0;
//     root->next = NULL;
//     root->prev = NULL;
//     root->tag = "ENTITY_ROOT";
//     entity->root_tag = root;
//     entity->num_tags = 0;

//     es->num_entities++;
//     return entity;
// }

// Rectangle get_entity_source(const t_Entity* entity) {
//     if (entity->is_3d) {
//         return entity->entity2D.source;
//     }
// }

t_Entity* get_entity(t_EntitySystem* es, int index) {
    return es->entities[index];
}

t_Entity* get_entity_by_name(t_EntitySystem* es, char* entity_name) {\
    for (int i = 0; i < es->num_entities; ++i) {
        t_Entity* e = get_entity(es, i);
        if (e == NULL || e->entity_name == NULL) { continue; }
        if (strcmp(e->entity_name, entity_name) == 0) {
            return e;
        }
    }
    return NULL;
}

bool are_entities_colliding(const t_Entity* e1, const t_Entity* e2) {
    if (!e1 || !e2 || !e1->is_active || !e2->is_active) {
        return false;
    }

    BoundingBox box1 = {
        .min = Vector3Subtract(e1->entity3D.position, Vector3Scale(e1->entity3D.scale, 0.5f)),
        .max = Vector3Add(e1->entity3D.position, Vector3Scale(e1->entity3D.scale, 0.5f))
    };

    BoundingBox box2 = {
        .min = Vector3Subtract(e2->entity3D.position, Vector3Scale(e2->entity3D.scale, 0.5f)),
        .max = Vector3Add(e2->entity3D.position, Vector3Scale(e2->entity3D.scale, 0.5f))
    };

    return CheckCollisionBoxes(box1, box2);
    // } else if (!e1->is_3d && !e2->is_3d) {
    //     Rectangle rect1 = {
    //         e1->entity2D.position.x - e1->data.entity2D.sprite_origin.x,
    //         e1->data.entity2D.position.y - e1->data.entity2D.sprite_origin.y,
    //         e1->data.entity2D.texture.width * e1->data.entity2D.scale,
    //         e1->data.entity2D.texture.height * e1->data.entity2D.scale
    //     };

    //     Rectangle rect2 = {
    //         e2->data.entity2D.position.x - e2->data.entity2D.sprite_origin.x,
    //         e2->data.entity2D.position.y - e2->data.entity2D.sprite_origin.y,
    //         e2->data.entity2D.texture.width * e2->data.entity2D.scale,
    //         e2->data.entity2D.texture.height * e2->data.entity2D.scale
    //     };

    //     return CheckCollisionRecs(rect1, rect2);
    // }

}

void print_movement(const t_Entity* entity) {
    printf("Movement info for %s:\n", entity->entity_name);
    printf("\tPosition: %.2f, %.2f, %.2f\n", entity->entity3D.position.x, entity->entity3D.position.y, entity->entity3D.position.z);
    printf("\tVelocity: %.2f, %.2f, %.2f\n", entity->entity3D.velocity.x, entity->entity3D.velocity.y, entity->entity3D.velocity.z);

}

#pragma region Entity tag system
void print_tags(const t_Entity* entity) {
    const char** tags = get_tags(entity);
    if (tags == NULL) {
        fprintf(stderr, "ERROR: print_tags: Failed to get tags\n");
        return;
    }
    for (int i = 0; i < entity->num_tags; ++i) {
        if (tags[i] == NULL) { return; }
        printf("TAG: %s \n", tags[i]);
    }

    free(tags);
}

const char** get_tags(const t_Entity* entity) {
    if (!entity || !entity->root_tag) {
        fprintf(stderr, "Entity or root_tag is NULL\n");
        return NULL;
    }

    t_Tag* current = entity->root_tag->next;
    if (!current) {
        fprintf(stderr, "root_tag->next is NULL\n");
        return NULL;
    }
    printf("num_tags: %d \n", entity->num_tags);
    printf("memory allocated for %lld bytes\n", sizeof(char*) * entity->num_tags+1);
    const char** tags = safe_malloc(sizeof(char*) * entity->num_tags+1);
    if (tags == NULL) {
        fprintf(stderr, "ERROR: get_tags: Memory allocation failed\n");
        return NULL;
    }

    int i = 0;
    while (current != entity->root_tag) {
        if (i >= entity->num_tags) {
            fprintf(stderr, "Too many tags for entity %s\n", entity->entity_name);
            break;
        }

        if (!current) {
            fprintf(stderr, "Current tag pointer is NULL\n");
            free(tags);
            return NULL;
        }

        if (!current->tag) {
            fprintf(stderr, "Current tag is NULL\n");
            free(tags);
            return NULL;
        }

        tags[i] = current->tag;
        current = current->next;
        ++i;
    }

    return tags;
}

void add_tag(t_Entity* entity, const char* tag) {
    t_Tag* new_tag = safe_malloc(sizeof(t_Tag));
    new_tag->tag_id = entity->num_tags+1;
    new_tag->tag = tag;
    
    new_tag->next = entity->root_tag;

    if (entity->root_tag->prev != NULL) {
        // whatever comes after the root tag will now point to the new tag
        new_tag->prev = entity->root_tag->prev;
        entity->root_tag->prev->next = new_tag;
        entity->root_tag->prev = new_tag;

    } else { // there is no tag before the root
        new_tag->prev = entity->root_tag;
        entity->root_tag->next = new_tag;
    }

    // the new tag will now be the new end (circular linked list)
    entity->root_tag->prev = new_tag;
    entity->num_tags++;
}

int has_tag(const t_Entity* entity, const char* tag) {
    // entity has no tags
    if (entity->root_tag->next == NULL) {
        return -1;
    }

    t_Tag* current = entity->root_tag->next;
    while (current != entity->root_tag) {
        if (strcmp(current->tag, tag) == 0) {
            return current->tag_id;
        }
        current = current->next;
    }
    return -1;

}

void remove_tag_at(t_Entity* entity, int index) {
    t_Tag* current = entity->root_tag->next;
    int i = 0;
    while (current != entity->root_tag) {
        if (i == index) {
            current->prev->next = current->next;
            current->next->prev = current->prev;
            free(current);
            entity->num_tags--;
            return;
        }
        current = current->next;
        ++i;
    }
    // we didn't find the tag to remove
}

void remove_tag(t_Entity* entity, const char* tag) {
    int index = has_tag(entity, tag);
    if (index == -1) {
        return;
    }

    remove_tag_at(entity, index);
}

void remove_all_tags(t_Entity* entity) {
    t_Tag* current = entity->root_tag->next;
    while (current != entity->root_tag) {
        t_Tag* next = current->next;
        free(current);
        current = next;
    }
    entity->root_tag->next = NULL;
    entity->root_tag->prev = NULL;
    entity->num_tags = 0;
}
#pragma endregion // Entity tag system
#pragma endregion // functions for base entity

#pragma region Texture handling

Texture2D get_texture(t_EntitySystem* es, int id) {
    return es->textures[id].texture;
}

// Wait why don't we just return the t_Texture?
// Not sure why we don't lol
// oh well it works for now
int load_texture(t_EntitySystem* es, const char* texture_path) {
    // Check if the texture already exists
    for (int i = 0; i < es->num_textures; ++i) {
        if (strcmp(es->textures[i].texture_name, texture_path) == 0) {
            return i; // Return existing texture index
        }
    }

    // Resize the textures array if necessary
    if (es->num_textures == es->capacity_textures) {
        int new_capacity = es->capacity_textures * TEXTURE_RESIZE_FACTOR;
        t_Texture* new_textures = (t_Texture*) safe_realloc(es->textures, new_capacity * sizeof(t_Texture));
        if (!new_textures) {
            fprintf(stderr, "Failed to allocate memory for textures\n");
            return -1; // Handle memory allocation failure
        }

        es->textures = new_textures;
        es->capacity_textures = new_capacity;
    }

    // Load the texture and add it to the array
    Texture2D texture = LoadTexture(texture_path);
    if (texture.id == 0) { // Assuming 0 indicates a failed texture load
        fprintf(stderr, "Failed to load texture: %s\n", texture_path);
        return -1;
    }

    t_Texture new_texture = { .texture_name = strdup(texture_path), .texture = texture };
    if (!new_texture.texture_name) {
        fprintf(stderr, "Failed to allocate memory for texture name\n");
        return -1;
    }

    es->textures[es->num_textures] = new_texture;
    es->num_textures++;

    return es->num_textures - 1; // Return the index of the new texture
}

#pragma endregion

#pragma region Deconstruction
void destroy_entity(t_Entity* entity) {

    if (!is_address_good(entity)) { entity = NULL; }

    entity->is_active = false;
    
    if (is_address_good(entity->on_destroy)) {
        entity->on_destroy(entity);
    }

    if (is_address_good(entity->entity_data)) {
        SAFE_FREE(entity->entity_data);
    }

    SAFE_FREE(entity);
}

void destroy_entity_system(t_EntitySystem* es) {
    for (int i = 0; i < es->num_entities; ++i) {
        destroy_entity(es->entities[i]);
    }

    SAFE_FREE(es);
}

#pragma endregion