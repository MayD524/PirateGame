// text_label.c
#include <custom3d.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void FlipImageVertical(Image *image) {
    // Ensure the image is valid
    if (image == NULL || image->data == NULL) return;

    int bytesPerPixel = image->format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 ? 4 : 3; // Assuming RGB or RGBA format
    int rowSize = image->width * bytesPerPixel;

    // Temporary buffer for a single row of pixels
    unsigned char *rowBuffer = (unsigned char *)malloc(rowSize);

    if (rowBuffer == NULL) {
        // Handle allocation failure
        fprintf(stderr, "FlipImageVerticalCustom: Failed to allocate row buffer.\n");
        return;
    }

    // Swap rows from top to bottom
    for (int y = 0; y < image->height / 2; y++) {
        unsigned char *topRow = (unsigned char *)image->data + y * rowSize;
        unsigned char *bottomRow = (unsigned char *)image->data + (image->height - 1 - y) * rowSize;

        // Swap rows
        memcpy(rowBuffer, topRow, rowSize);
        memcpy(topRow, bottomRow, rowSize);
        memcpy(bottomRow, rowBuffer, rowSize);
    }

    // Free the temporary buffer
    free(rowBuffer);
}

// Function to create a texture from rendered text
static Texture2D CreateTextTexture(Font font, const char *text, Color color, int labelIndex) {
    // Measure text size
    Vector2 textSize = MeasureTextEx(font, text, font.baseSize, 1.0f);

    // Calculate texture size with padding
    int width = (int)textSize.x + 10;
    int height = (int)textSize.y + 10;

    // Create a RenderTexture2D with the calculated size
    RenderTexture2D target = LoadRenderTexture(width, height);

    // Begin drawing to the RenderTexture
    BeginTextureMode(target);
        ClearBackground((Color){ 0, 0, 0, 0 }); // Transparent background
        DrawTextEx(font, text, (Vector2){ 5, 5 }, font.baseSize, 1.0f, color);
    EndTextureMode();

    // Get the image from the RenderTexture
    Image img = LoadImageFromTexture(target.texture);

    // **Flip the image vertically to correct the orientation**
    FlipImageVertical(&img);

    // Create a new Texture2D from the Image
    Texture2D textTexture = LoadTextureFromImage(img);

    // Unload the Image and RenderTexture
    UnloadImage(img);
    UnloadRenderTexture(target);

    return textTexture;
}

// Initializes the dynamic array
void InitTextLabelArray(TextLabelArray *array) {
    array->capacity = 100; // Initial capacity
    array->count = 0;
    array->labels = (TextLabel *)malloc(array->capacity * sizeof(TextLabel));
    if (array->labels == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Failed to allocate memory for TextLabelArray.\n");
        exit(EXIT_FAILURE);
    }
}

// Adds a new label to the array with dynamic text
void AddTextLabel(TextLabelArray *array, const char *text, Vector3 position, Font font) {
    // Resize if necessary
    if (array->count >= array->capacity) {
        array->capacity *= 2;
        array->labels = (TextLabel *)realloc(array->labels, array->capacity * sizeof(TextLabel));
        if (array->labels == NULL) {
            // Handle memory allocation failure
            fprintf(stderr, "Failed to reallocate memory for TextLabelArray.\n");
            exit(EXIT_FAILURE);
        }
    }

    // Initialize the new label
    TextLabel *label = &array->labels[array->count];
    label->text = strdup(text); // Allocate and copy the text
    if (label->text == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Failed to allocate memory for TextLabel text.\n");
        exit(EXIT_FAILURE);
    }
    label->position = position;
    label->dirty = true; // Mark as needing texture creation
    label->texture.id = 0; // Initialize texture ID

    array->count++;
}

// Updates the text of an existing label
void UpdateTextLabel(TextLabelArray *array, int index, const char *newText, Font font) {
    if (index < 0 || index >= array->count) {
        fprintf(stderr, "UpdateTextLabel: Index %d out of bounds.\n", index);
        return;
    }
    TextLabel *label = &array->labels[index];

    // Free the old text
    free(label->text);

    // Allocate and copy the new text
    label->text = strdup(newText);
    if (label->text == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Failed to allocate memory for updated TextLabel text.\n");
        exit(EXIT_FAILURE);
    }

    label->dirty = true; // Mark as needing texture update
}

// Removes a label from the array and frees its resources
void RemoveTextLabel(TextLabelArray *array, int index) {
    if (index < 0 || index >= array->count) {
        fprintf(stderr, "RemoveTextLabel: Index %d out of bounds.\n", index);
        return;
    }

    // Free the text and texture
    free(array->labels[index].text);
    if (array->labels[index].texture.id != 0) {
        UnloadTexture(array->labels[index].texture);
    }

    // Shift remaining labels
    for(int i = index; i < array->count - 1; i++) {
        array->labels[i] = array->labels[i + 1];
    }
    array->count--;
}

// Draws all labels in the array using billboards with visibility checks
void DrawTextLabels3DBillboard(TextLabelArray *array, Camera3D camera, float maxDistance) {
    for(int i = 0; i < array->count; i++) {
        TextLabel *label = &array->labels[i];
        
        // Check if the label is within the camera's view and within maxDistance
        if (!IsPointInView(camera, label->position)) {
            // Skip rendering if label is not in view or too far
            continue;
        }

        if (label->dirty) {
            // If the label is dirty, create/update its texture
            if (label->texture.id != 0) {
                UnloadTexture(label->texture);
            }
            // Use a contrasting color like WHITE or another suitable color
            label->texture = CreateTextTexture(GetFontDefault(), label->text, BLACK, i);
            label->dirty = false;
        }

        // Draw the texture as a billboard at the label's position
        // Adjust the size as needed; here, 2.0f units
        DrawBillboard(camera, label->texture, label->position, 2.0f, WHITE);
    }
}



// cleanup_text_labels function to free all labels and resources
void cleanup_text_labels(TextLabelArray *array) {
    for(int i = 0; i < array->count; i++) {
        free(array->labels[i].text);
        if (array->labels[i].texture.id != 0) {
            UnloadTexture(array->labels[i].texture);
        }
    }
    free(array->labels);
}

void DrawThickLine3D(Vector3 start, Vector3 end, float thickness, Color color) {
    Vector3 direction = Vector3Subtract(end, start);
    float length = Vector3Length(direction);

    // Calculate the cylinder's orientation
    Vector3 up = { 0.0f, 1.0f, 0.0f };
    Vector3 axis = Vector3CrossProduct(up, direction);
    float angle = Vector3Angle(up, direction);

    // Draw the cylinder to represent the thick line
    rlPushMatrix();
    rlTranslatef(start.x, start.y, start.z);
    rlRotatef(RAD2DEG * angle, axis.x, axis.y, axis.z);
    DrawCylinder((Vector3){ 0.0f, length / 2.0f, 0.0f }, thickness, thickness, length, 10, color);
    rlPopMatrix();
}

void Draw3DGrid(float spacing, float scale, float maxDistance) {
    // Define the number of lines in each direction
    int numLines = (int)(maxDistance / spacing);

    // Draw lines along the X-axis (varying Z)
    for(int i = -numLines; i <= numLines; i++) {
        Vector3 start = { -numLines * spacing * scale, 0.0f, i * spacing * scale };
        Vector3 end = { numLines * spacing * scale, 0.0f, i * spacing * scale };
        DrawLine3D(start, end, GRAY);
    }

    // Draw lines along the Z-axis (varying X)
    for(int i = -numLines; i <= numLines; i++) {
        Vector3 start = { i * spacing * scale, 0.0f, -numLines * spacing * scale };
        Vector3 end = { i * spacing * scale, 0.0f, numLines * spacing * scale };
        DrawLine3D(start, end, GRAY);
    }

    // Draw the origin lines in different colors for clarity
    Vector3 originStartX = { -spacing * maxDistance, 0.0f, 0.0f };
    Vector3 originEndX = { spacing * maxDistance, 0.0f, 0.0f };
    DrawThickLine3D(originStartX, originEndX, 0.05, RED); // X-axis in red
    DrawThickLine3D(originEndX, originStartX, 0.05, RED); // X-axis in red

    Vector3 originStartZ = { 0.0f, 0.0f, -spacing * maxDistance };
    Vector3 originEndZ = { 0.0f, 0.0f, spacing * maxDistance };
    DrawThickLine3D(originStartZ, originEndZ, 0.05, BLUE); // Z-axis in blue
    DrawThickLine3D(originEndZ, originStartZ, 0.05, BLUE); // Z-axis in blue

    DrawPoint3D((Vector3){ 0, 0, 0 }, RED);
}

bool IsPointInView(Camera3D camera, Vector3 point) {
    // Compute vector from camera to point
    Vector3 dir = Vector3Subtract(point, camera.position);

    // Compute the camera's forward vector
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));

    // Compute the dot product between forward vector and direction to the point
    float dotProduct = Vector3DotProduct(forward, Vector3Normalize(dir));

    if (dotProduct <= 0) {
        // The point is behind the camera
        return false;
    }

    // Project the point to screen space
    Vector2 screenPos = GetWorldToScreen(point, camera);

    // Check if the point is within screen bounds
    if (screenPos.x < 0 || screenPos.x > GetScreenWidth() ||
        screenPos.y < 0 || screenPos.y > GetScreenHeight()) {
        return false;
    }

    return true;
}

void CreateGridWithLabels(float spacing, float scale, float maxDistance, TextLabelArray *labelArray, Font font) {
    // Add "0 m" label at the origin
    // AddTextLabel(labelArray, "0 m", (Vector3){0.0f, 0.1f, 0.0f}, font);

    Draw3DGrid(spacing, scale, maxDistance);

    // for(float i = spacing; i <= maxDistance; i += spacing) {
    //     // Labels for X-axis lines (positive direction)
    //     char labelTextX[64];
    //     snprintf(labelTextX, sizeof(labelTextX), "%.0f m", i);
    //     Vector3 labelPosX = { i * scale, 0.1f, 0.0f }; // Slightly above the floor
    //     AddTextLabel(labelArray, labelTextX, labelPosX, font);

    //     // Labels for X-axis lines (negative direction)
    //     char labelTextNegX[64];
    //     snprintf(labelTextNegX, sizeof(labelTextNegX), "%.0f m", i);
    //     Vector3 labelPosNegX = { -i * scale, 0.1f, 0.0f };
    //     AddTextLabel(labelArray, labelTextNegX, labelPosNegX, font);

    //     // Labels for Z-axis lines (positive direction)
    //     char labelTextZ[64];
    //     snprintf(labelTextZ, sizeof(labelTextZ), "%.0f m", i);
    //     Vector3 labelPosZ = { 0.0f, 0.1f, i * scale };
    //     AddTextLabel(labelArray, labelTextZ, labelPosZ, font);

    //     // Labels for Z-axis lines (negative direction)
    //     char labelTextNegZ[64];
    //     snprintf(labelTextNegZ, sizeof(labelTextNegZ), "%.0f m", i);
    //     Vector3 labelPosNegZ = { 0.0f, 0.1f, -i * scale };
    //     AddTextLabel(labelArray, labelTextNegZ, labelPosNegZ, font);
    // }
}
