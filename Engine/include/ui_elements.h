// ui_elements.h
#ifndef UI_ELEMENTS_H
#define UI_ELEMENTS_H

#include <stdbool.h>

// UI Element Types
typedef enum {
    UI_BUTTON,
    UI_LABEL
} UIElementType;

// Generic UI Element Structure
typedef struct {
    int x, y;         // Position
    int width, height; // Dimensions
    UIElementType type; // Type of UI element
} UIElement;

// Button Structure
typedef struct {
    UIElement base;    // Base UI element
    const char *text;  // Button text
    void (*onClick)(); // Callback function when button is clicked
} UIButton;

// Label Structure
typedef struct {
    UIElement base;   // Base UI element
    const char *text; // Label text
} UILabel;

// Function Prototypes
UIButton create_button(int x, int y, int width, int height, const char *text, void (*onClick)());
UILabel create_label(int x, int y, const char *text);
bool is_point_inside(UIElement *element, int px, int py);
void draw_button(const UIButton *button);
void draw_label(const UILabel *label);

#endif // UI_ELEMENTS_H
