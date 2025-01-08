#pragma once

#ifndef CORE_H
#define CORE_H

#include <stdio.h>
#include <raylib.h>

#include <threadpool.h>
#include <extended_memory.h>
#include <custom3d.h>
#include <scene.h>
#include <entity.h>
#include <util.h>
#include <dirent.h> 
#include <stdio.h> 
#include <models.h>
#include <bh_lua.h>


typedef struct {
    char** files;
    int file_count;
} t_FileSearch;

#define LUA_DIR "./build/scripts/"
#define FILE_SEARCH_SIZE 10

t_FileSearch GetFilesInDirectory(const char* path);

typedef struct {
    const char* game_title;
    int screen_width;
    int screen_height;
    int target_fps;

    bool exit_requested;
    bool should_close;
} WindowInformation;

typedef struct {
    t_EntitySystem* entity_system;
    LuaContext* lua_context;
    SceneManager* scene_manager;
    Camera* camera;

    TextLabelArray* text_labels;
    ModelManager* model_manager;

    WindowInformation* winfo;
    threadpool_t* threadpool;

    // SpatialGrid3D* grid3D;
    
    void* GameData;
} Engine;

extern Engine* g_engine;

WindowInformation* create_window_information(int width, int height, const char* title, int target_fps);
Engine* create_default_engine(WindowInformation* winfo);

void initialize_engine(Engine* engine);
int main_loop(Engine* engine);

#endif