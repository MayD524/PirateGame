#ifndef SCENE_H
#define SCENE_H

#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <extended_memory.h>
#include <util.h>

typedef struct Scene Scene;
struct Scene{
    const char* name;

    void (*on_load)(Scene* scene);
    void (*on_render)(Scene* scene);
    void (*on_update)(Scene* scene, float delta_time);
    void (*on_unload)(Scene* scene);

    void *SceneData;

    bool is_active;
};

typedef struct {
    Scene** scenes;
    int scene_count;      // Number of scenes currently stored
    int scene_capacity;   // Allocated capacity

    Scene* active_scene;
    const char* default_scene;
} SceneManager;

SceneManager* create_scene_manager();
void destroy_scene_manager(SceneManager* manager);

bool add_scene(SceneManager* manager, Scene scene);
Scene* get_scene(SceneManager* manager, const char* scene_name);
int load_scene(SceneManager* manager, const char* scene_name);

#endif