// ui_elements.c
#include <ui_elements.h>
#include <stdio.h>
#include <string.h>

UIButton create_button(int x, int y, int width, int height, const char *text, void (*onClick)()) {
    UIButton button;
    button.base.x = x;
    button.base.y = y;
    button.base.width = width;
    button.base.height = height;
    button.base.type = UI_BUTTON;
    button.text = text;
    button.onClick = onClick;
    return button;
}

UILabel create_label(int x, int y, const char *text) {
    UILabel label;
    label.base.x = x;
    label.base.y = y;
    label.base.width = strlen(text) * 8; // Assuming 8 pixels per character
    label.base.height = 16;             // Assuming 16-pixel height for text
    label.base.type = UI_LABEL;
    label.text = text;
    return label;
}

bool is_point_inside(UIElement *element, int px, int py) {
    return px >= element->x && px <= element->x + element->width &&
           py >= element->y && py <= element->y + element->height;
}

void draw_button(const UIButton *button) {
    printf("[Button] %s at (%d, %d), size (%d x %d)\n",
           button->text, button->base.x, button->base.y, button->base.width, button->base.height);
}

void draw_label(const UILabel *label) {
    printf("[Label] %s at (%d, %d)\n",
           label->text, label->base.x, label->base.y);
}
