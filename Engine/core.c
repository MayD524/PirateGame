#include <core.h>

t_FileSearch GetFilesInDirectory(const char* path) {
    t_FileSearch file_search;
    char** files = safe_malloc(sizeof(char*) * FILE_SEARCH_SIZE);
    int max = FILE_SEARCH_SIZE;
    int file_count = 0;
    DIR *dir = opendir(path);
    struct dirent *de;

    while((de = readdir(dir))!= NULL) {
        if (file_count >= max) {
            max += FILE_SEARCH_SIZE;
            files = safe_realloc(files, sizeof(char*) * max);
        }
        if (strcmp(de->d_name, ".")!= 0 && strcmp(de->d_name, "..")!= 0 && strcmp(de->d_name, "bh.lua")!= 0) {
            files[file_count] = safe_malloc(strlen(path) + strlen(de->d_name) + 1);
            strcpy(files[file_count], path);
            strcat(files[file_count], de->d_name);
            file_count++;
        }
    }
    closedir(dir);
    file_search.files = files;
    file_search.file_count = file_count;
    return file_search;
}

void cleanup_model_manager(ModelManager* manager) {
    for (int i = 0; i < manager->model_count; i++) {
        if (manager->models[i]) {
            ModelInfo* info = manager->models[i];
            UnloadModel(info->model);
            SAFE_FREE(info->file_path);
            SAFE_FREE(info);
        }
    }
    SAFE_FREE(manager->models); // Free the dynamic array
    SAFE_FREE(manager->free_list); // Free the free list
    SAFE_FREE(manager);
}


WindowInformation* create_window_information(int width, int height, const char* title, int target_fps) {
    WindowInformation* winfo = (WindowInformation*) safe_malloc(sizeof(WindowInformation));
    winfo->game_title    = title;
    winfo->screen_width = width;
    winfo->screen_height = height;
    winfo->target_fps    = target_fps;

    winfo->should_close = false;
    winfo->exit_requested = false;

    return winfo;
}

void initialize_model_manager(ModelManager* manager) {
    manager->current_model_id = 0;
    manager->model_capacity = 10; // Initial capacity
    manager->model_count = 0;
    manager->models = safe_malloc(manager->model_capacity * sizeof(ModelInfo*));
    memset(manager->models, 0, manager->model_capacity * sizeof(ModelInfo*));

    manager->free_list = safe_malloc(manager->model_capacity * sizeof(int));
    manager->free_count = 0;
}

Engine* create_default_engine(WindowInformation* winfo) {
    Engine* engine = (Engine*) safe_malloc(sizeof(Engine));
        
    engine->winfo = winfo;
    engine->entity_system = create_entity_system();
    engine->lua_context  = init_lua();
    engine->scene_manager = create_scene_manager();
    engine->model_manager = (ModelManager*)safe_malloc(sizeof(ModelManager));
    engine->text_labels = safe_malloc(sizeof(TextLabelArray));

    engine->threadpool = threadpool_create(DEFAULT_THREADPOOL_SIZE);

    InitTextLabelArray(engine->text_labels);
    initialize_model_manager(engine->model_manager);

    return engine;
}

void destroy_engine(Engine* engine) {
    printf("Cleaning up lua\n");
    close_lua(engine->lua_context);
    printf("Closing window\n");
    CloseWindow();

    printf("Cleaning up model manager");
    cleanup_model_manager(engine->model_manager);

    Physics_CleanupEntities(engine->entity_system->entities, engine->entity_system->num_entities);
    printf("Cleaning up entity system\n");
    destroy_entity_system(engine->entity_system);

    // free_spatial_grid_3d(engine->grid3D);
    // free(engine->grid3D);

    cleanup_text_labels(engine->text_labels);

    printf("Cleaning up winfo\n");
    SAFE_FREE(engine->winfo);
    printf("Cleaning up engine\n");
    SAFE_FREE(engine);
}

void initialize_engine(Engine* engine) {
    printf("width: %d | height: %d", engine->winfo->screen_width, engine->winfo->screen_height);
    
    InitWindow(engine->winfo->screen_width, engine->winfo->screen_height, engine->winfo->game_title);
    SetTargetFPS(engine->winfo->target_fps);

    SetWindowMonitor(0);

    printf("func_refs: %p\n", engine->lua_context->func_refs);
    t_FileSearch file_search = GetFilesInDirectory(LUA_DIR);
    for (int i = 0; i < file_search.file_count; i++) {
        printf("loading %s...\n", file_search.files[i]);
        execute_lua_file(engine->lua_context->lua_state, file_search.files[i]);
    }

    call_functions(engine->lua_context, ON_START);
}

int main_loop(Engine* engine) {
    // TODO: Setup a better close system (allow for engine->winfo->should_close)
    while (!WindowShouldClose()) {
        Scene* active_scene = engine->scene_manager->active_scene;
        /* PRE-DRAWING */
        float delta_time = GetFrameTime();
        if (active_scene != NULL && active_scene->on_update)
            active_scene->on_update(active_scene, delta_time);

        BeginDrawing();
            ClearBackground(RAYWHITE);
            call_functions(engine->lua_context, PRE_RENDER);
            
            render(engine->entity_system, engine->text_labels, *engine->camera);

            if (active_scene && active_scene->on_render)
                active_scene->on_render(active_scene);

            call_functions(engine->lua_context, POST_RENDER);

        EndDrawing();
        update(engine->entity_system);
        call_functions(engine->lua_context, UPDATE);
    }

    destroy_engine(engine);
    return 0;
}


int add_model_info(const char* file_path) {
    ModelManager* manager = g_engine->model_manager;
    
    for (int i = 0; i < manager->model_count; i++) {
        // Make sure the slot isn’t NULL
        ModelInfo* existingInfo = manager->models[i];
        if (existingInfo != NULL && existingInfo->file_path != NULL) {
            // Compare file paths
            if (strcmp(existingInfo->file_path, file_path) == 0) {
                // Path is already loaded; return the existing model's ID
                return existingInfo->model_id;
            }
        }
    }
    
    int index;
    ModelInfo* info = safe_malloc(sizeof(ModelInfo));
    info->file_path = strdup(file_path);
    info->model_id = manager->current_model_id;

    info->model = LoadModel(file_path);  
    if (info->model.meshCount == 0) {
        printf("%s has no meshes..\n", file_path);
        SAFE_FREE(info);
        return -1;
    }

    if (manager->free_count > 0) {
        manager->free_count--;
        index = manager->free_list[manager->free_count];
    } else {
        if (manager->model_count >= manager->model_capacity) {
            manager->model_capacity *= 2;
            manager->models = safe_realloc(manager->models,
                manager->model_capacity * sizeof(ModelInfo*));
            memset(manager->models + manager->model_count, 0,
                (manager->model_capacity - manager->model_count) * sizeof(ModelInfo*));

            manager->free_list = safe_realloc(manager->free_list,
                manager->model_capacity * sizeof(int));
        }
        index = manager->model_count;
    }

    info->full_box = get_model_bounding_box(info->model);
    info->mesh_bounding_boxes = get_model_mesh_bounding_boxes(info->model);

    int animCount = 0;
    ModelAnimation* loadedAnims = LoadModelAnimations(file_path, &animCount);
    if (animCount > 0 && loadedAnims) {
        info->animations = loadedAnims;
        info->animation_count = animCount;
    } else {
        info->animations = NULL;
        info->animation_count = 0;
    }

    printf("This model has %d meshes\n", info->model.meshCount);

    // 5) Store it
    manager->current_model_id++;
    manager->model_count++;
    manager->models[index] = info;
    return info->model_id;
}

void update_model_animation(ModelInfo* info, int animationIndex, float frame, bool loop)
{
    // Safety checks
    if (!info || !info->animations || animationIndex >= info->animation_count) return;

    // Raylib’s ModelAnimation to update with
    ModelAnimation anim = info->animations[animationIndex];

    // If the frame is out of range, loop or clamp
    if (frame >= anim.frameCount) {
        if (loop) frame = fmod(frame, (float)anim.frameCount);
        else frame = (float)anim.frameCount - 1; // clamp
    }

    // Update the model to the given frame
    UpdateModelAnimation(info->model, anim, (int)frame);
}

ModelInfo* get_model_info(int model_id) {
    if (model_id == -1 || model_id > g_engine->model_manager->current_model_id) {
        return NULL;
    }
    return g_engine->model_manager->models[model_id];
}

void remove_model_info(int model_id) {
    ModelManager* manager = g_engine->model_manager;

    // Find which index in manager->models corresponds to model_id
    ModelInfo* info = get_model_info(model_id);
    if (!info) return;

    if (info->animations && info->animation_count > 0) {
        UnloadModelAnimations(info->animations, info->animation_count);
        // Note that UnloadModelAnimations() frees the array as well
        info->animations = NULL;
        info->animation_count = 0;
    }

    UnloadModel(info->model);
    free((char*)info->file_path);
    free(info);

    manager->models[model_id] = NULL;
    manager->free_list[manager->free_count++] = model_id;
}

int get_model_animation_count(const ModelInfo* info) {
    if (!info) return 0; // Null check
    return info->animation_count;
}

int get_animation_total_frames(const ModelInfo* info, int animationIndex) {
    if (!info || !info->animations) return -1; // Null check

    if (animationIndex < 0 || animationIndex >= info->animation_count) {
        // Out of range
        return -1;
    }

    // raylib's ModelAnimation struct typically has this field:
    //    int frameCount;     // Number of frames stored
    return info->animations[animationIndex].frameCount;
}

BoundingBox get_model_bounding_box(Model model) {
    BoundingBox combined_bbox = GetMeshBoundingBox(model.meshes[0]);

    for (int i = 1; i < model.meshCount; i++) {
        BoundingBox mesh_bbox = GetMeshBoundingBox(model.meshes[i]);
        combined_bbox.min = Vector3Min(combined_bbox.min, mesh_bbox.min);
        combined_bbox.max = Vector3Max(combined_bbox.max, mesh_bbox.max);
    }

    return combined_bbox;
}

// Function to get an array of bounding boxes for all meshes in a Model
BoundingBox* get_model_mesh_bounding_boxes(Model model) {
    BoundingBox* bounding_boxes = (BoundingBox*)malloc(model.meshCount * sizeof(BoundingBox));

    for (int i = 0; i < model.meshCount; i++) {
        bounding_boxes[i] = GetMeshBoundingBox(model.meshes[i]);
    }

    return bounding_boxes;
}

Vector3 get_model_size(const ModelInfo* info) {
    BoundingBox bbox = get_model_bounding_box(info->model);

    Vector3 size = {
        bbox.max.x - bbox.min.x,
        bbox.max.y - bbox.min.y,
        bbox.max.z - bbox.min.z
    };

    return size;
}