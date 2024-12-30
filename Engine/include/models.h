#pragma once

#ifndef MODELS_H
#define MODELS_H

#include <raylib.h>
#include <extended_memory.h>
#include <math.h>

typedef struct {
    const char* file_path;
    int model_id;
    Model model;

    // For raylib animations:
    ModelAnimation *animations; 
    int animation_count;

    BoundingBox full_box;
    BoundingBox* mesh_bounding_boxes;
} ModelInfo;

typedef struct {
    int current_model_id;     // Unique model ID generator
    ModelInfo** models;       // Pointer to an array of ModelInfo pointers
    int model_count;          // Number of models currently in the list
    int model_capacity;       // Capacity of the list
    int* free_list;           // Array of freed indices
    int free_count;           // Number of available slots in the free list
} ModelManager;

void update_model_animation(ModelInfo* info, int animationIndex, float frame, bool loop);
int add_model_info(const char* file_path);
ModelInfo* get_model_info(int model_id);
void remove_model_info(int model_id);
int get_animation_total_frames(const ModelInfo* info, int animationIndex) ;
int get_model_animation_count(const ModelInfo* info);
Vector3 get_model_size(const ModelInfo* info);
BoundingBox get_model_bounding_box(Model model);
BoundingBox* get_model_mesh_bounding_boxes(Model model);

#endif