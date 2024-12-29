#include <scene.h>

SceneManager* create_scene_manager(void) 
{
    // Allocate space for the manager itself
    SceneManager* manager = (SceneManager*)safe_malloc(sizeof(SceneManager));
    if (!manager) {
        // Handle error as needed
        return NULL;
    }

    // Decide on an initial capacity (8, 16, 32, etc.)
    int initial_capacity = 8;

    // Allocate the dynamic array of Scene pointers
    manager->scenes = (Scene**)safe_malloc(sizeof(Scene*) * initial_capacity);
    if (!manager->scenes) {
        free(manager);
        return NULL;
    }

    // Initialize the fields
    manager->scene_capacity = initial_capacity;
    manager->scene_count = 0;
    manager->active_scene = NULL;
    manager->default_scene = NULL;

    return manager;
}

void destroy_scene_manager(SceneManager* manager)
{
    if (!manager) return;

    // Free each Scene if we allocated them
    for (int i = 0; i < manager->scene_count; i++) {
        if (manager->scenes[i]) {
            free(manager->scenes[i]);  // or a `destroy_scene()` if you have one
        }
    }

    // Free the array
    free(manager->scenes);

    // Free the manager
    free(manager);
}

Scene* get_scene(SceneManager* manager, const char* scene_name)
{
    if (!manager || !scene_name) return NULL;

    for (int i = 0; i < manager->scene_count; i++) {
        Scene* s = manager->scenes[i];
        if (s && s->name && strcmp(s->name, scene_name) == 0) {
            return s;
        }
    }
    return NULL; // Not found
}

int load_scene(SceneManager* manager, const char* scene_name) {
    Scene* scene = get_scene(manager, scene_name);

    if (!scene) {
        // ...
        return -1;
    }

    if (manager->active_scene && manager->active_scene->on_unload)
        manager->active_scene->on_unload(manager->active_scene);

    manager->active_scene = scene;
    scene->is_active = true;

    if (scene->on_load)
        scene->on_load(scene);
    return 0;
}

bool add_scene(SceneManager* manager, Scene scene)
{
    if (!manager || !scene.name) {
        return false;
    }

    // Check if a scene with this name already exists
    if (get_scene(manager, scene.name) != NULL) {
        // Scene already present
        return false;
    }

    // Grow the array if needed
    if (manager->scene_count >= manager->scene_capacity) {
        manager->scene_capacity *= 2;
        Scene** new_array = (Scene**)safe_realloc(manager->scenes,
                               manager->scene_capacity * sizeof(Scene*));
        if (!new_array) {
            // Failed to safe_realloc, keep old array but can’t add
            return false;
        }
        manager->scenes = new_array;
    }

    // Allocate a new Scene structure on the heap
    Scene* new_scene = (Scene*)safe_malloc(sizeof(Scene));
    if (!new_scene) {
        return false;
    }

    // Copy the incoming `scene` into this newly allocated Scene
    memcpy(new_scene, &scene, sizeof(Scene));

    // Insert into the array
    manager->scenes[manager->scene_count++] = new_scene;

    // Verify it was added
    return (get_scene(manager, scene.name) != NULL);
}