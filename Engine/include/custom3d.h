// custom3d.h
#ifndef TEXT_LABEL_H
#define TEXT_LABEL_H

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdbool.h>


// Structure to hold label information with dynamic text
typedef struct TextLabel {
    char *text;          // Dynamically allocated text content
    Texture2D texture;   // Texture containing the rendered text
    Vector3 position;    // 3D position of the label
    bool dirty;          // Flag to indicate if the texture needs to be updated
} TextLabel;

// Dynamic array structure for TextLabels
typedef struct TextLabelArray {
    TextLabel *labels;   // Pointer to the array of labels
    int count;           // Current number of labels
    int capacity;        // Maximum capacity before resizing
} TextLabelArray;

void InitTextLabelArray(TextLabelArray *array);
void AddTextLabel(TextLabelArray *array, const char *text, Vector3 position, Font font);
void UpdateTextLabel(TextLabelArray *array, int index, const char *newText, Font font);
void RemoveTextLabel(TextLabelArray *array, int index);
void cleanup_text_labels(TextLabelArray *array);
void CreateGridWithLabels(float spacing, float scale, float maxDistance, TextLabelArray *labelArray, Font font);
void DrawTextLabels3DBillboard(TextLabelArray *array, Camera3D camera, float maxDistance);
void Draw3DGrid(float spacing, float scale, float maxDistance);
bool IsPointInView(Camera3D camera, Vector3 point);


#endif // TEXT_LABEL_H
