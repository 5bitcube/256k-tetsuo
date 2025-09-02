#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Screen and puzzle constants
const int SCREEN_WIDTH    = 1920;
const int SCREEN_HEIGHT   = 1080;
const int SCREEN_CENTER_X = SCREEN_WIDTH / 2;
const int SCREEN_CENTER_Y = SCREEN_HEIGHT / 2;
const int PUZZLE_VERT_X   = SCREEN_CENTER_X;
const int PUZZLE_TOP_Y    = SCREEN_CENTER_Y - 250;
const int PUZZLE_BOTTOM_Y = SCREEN_CENTER_Y + 250;
const float PUZZLE_MAX_NIBBLE_LEN = 20.0f + 16 * 18.0f;

// Draws a polygon with specified thickness and color
void DrawPolygon2(Vector2 *points, int pointCount, float thickness, Color color, bool closed) {
    if (pointCount < 2) return;

    for (int i = 0; i < (closed ? pointCount : pointCount - 1); i++) {
        int next = (i + 1) % pointCount;
        Vector2 dir = Vector2Subtract(points[next], points[i]);
        float length = Vector2Length(dir);

        if (length == 0) continue;

        Vector2 norm = Vector2Scale(dir, 1.0f / length);
        Vector2 extend = Vector2Scale(norm, thickness * 0.5f);
        Vector2 start = Vector2Subtract(points[i], extend);
        Vector2 end = Vector2Add(points[next], extend);
        DrawLineEx(start, end, thickness, color);
    }
}

// Draws segmented L-shaped lines based on byte data
void DrawSegments(Vector2 start, Vector2 end, int divisions, float thickness, Color color, uint8_t *bytes, bool show_labels) {
    if (divisions < 1 || bytes == NULL) return;
    float yLength = end.y - start.y;
    if (yLength == 0) return;
    float stepSize = yLength / (divisions + 1); // Equal spacing to bottom

    for (int i = 0; i < divisions; i++) {
        float yPos = start.y + (i + 0.5f) * stepSize;
        uint8_t byte = bytes[i];
        uint8_t high = (byte >> 4) + 1; // High nibble, scaled 1-16
        uint8_t low = (byte & 0xF) + 1; // Low nibble, scaled 1-16
        float left_len = 20.0f + high * 18.0f; // Left segment length
        float right_len = 20.0f + low * 18.0f; // Right segment length

        // Draw left L-shape: horizontal then vertical
        Vector2 p1_left = {start.x + thickness * 0.5f, yPos};
        Vector2 p2_left = {start.x - left_len - thickness * 0.5f, yPos};
        DrawLineEx(p1_left, Vector2Subtract(p2_left, (Vector2){thickness / 2, 0.0f}), thickness, color);
        Vector2 down_end_left = {p2_left.x, PUZZLE_BOTTOM_Y + thickness * 0.5f};
        DrawLineEx(p2_left, down_end_left, thickness, color);

        // Draw right L-shape: horizontal then vertical
        Vector2 p1_right = {start.x - thickness * 0.5f, yPos};
        Vector2 p2_right = {start.x + right_len + thickness * 0.5f, yPos};
        DrawLineEx(p1_right, Vector2Add(p2_right, (Vector2){thickness / 2, 0.0f}), thickness, color);
        Vector2 down_end_right = {p2_right.x, PUZZLE_BOTTOM_Y + thickness * 0.5f};
        DrawLineEx(p2_right, down_end_right, thickness, color);

        // Draw byte value as hex label near vertical axis
        if (show_labels) {
            DrawText(TextFormat("%02X", byte), start.x + 5, yPos - 10, 10, BLACK);
        }
    }

    // Draw X-axis labels (0-F) for nibble values
    if (show_labels) {
        for (int i = 0; i <= 15; i++) {
            float len = 20.0f + (i + 1) * 18.0f;
            float x_left = start.x - len;
            float x_right = start.x + len;
            DrawText(TextFormat("%X", i), x_left, PUZZLE_BOTTOM_Y + 20, 10, BLACK);
            DrawText(TextFormat("%X", i), x_right, PUZZLE_BOTTOM_Y + 20, 10, BLACK);
        }
    }
}

// Parses space-separated hex string into uint8_t array
int parse_hex_bytes(const char *hex_str, uint8_t *out, size_t outlen) {
    char *copy = strdup(hex_str);
    if (!copy) return -1;
    int count = 0;
    char *token = strtok(copy, " ");
    while (token && count < outlen) {
        out[count++] = (uint8_t)strtol(token, NULL, 16);
        token = strtok(NULL, " ");
    }
    free(copy);
    return count;
}

int main(void) {
    // Get user input for hex bytes
    char byte_string[256];
    printf("Enter space-separated hex bytes (e.g., 4a 21 0f): ");
    if (!fgets(byte_string, sizeof(byte_string), stdin)) {
        fprintf(stderr, "Input error\n");
        return 1;
    }

    // Remove trailing newline from input
    byte_string[strcspn(byte_string, "\n")] = '\0';

    // Parse input into byte array
    uint8_t bytes[32];
    int byte_count = parse_hex_bytes(byte_string, bytes, sizeof(bytes));

    // Initialize window and camera
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "256K Tetsuo");
    ToggleFullscreen();
    Camera2D camera = { 0 };
    camera.target = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Mouse interaction variables
    Vector2 lastMousePos = { 0.0f, 0.0f };
    bool isDragging = false;
    bool show_labels = true;
    bool show_y_axis = true;

    // Main game loop
    while (!WindowShouldClose()) {
        // Toggle labels with 'L' key
        if (IsKeyPressed(KEY_L)) {
            show_labels = !show_labels;
        }

        // Toggle Y-axis with 'Y' key
        if (IsKeyPressed(KEY_Y)) {
            show_y_axis = !show_y_axis;
        }

        // Zoom with mouse wheel
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            camera.zoom += wheel * 0.05f;
            if (camera.zoom < 0.1f) camera.zoom = 0.1f;
        }

        // Pan camera with mouse drag
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (!isDragging) {
                isDragging = true;
                lastMousePos = GetMousePosition();
            } else {
                Vector2 currentMousePos = GetMousePosition();
                Vector2 delta = Vector2Subtract(lastMousePos, currentMousePos);
                camera.target = Vector2Add(camera.target, Vector2Scale(delta, 1.0f / camera.zoom));
                lastMousePos = currentMousePos;
            }
        } else {
            isDragging = false;
        }

        // Render scene
        BeginDrawing();
        ClearBackground(WHITE);
        BeginMode2D(camera);

        // Draw Y-axis if enabled
        if (show_y_axis) {
            DrawLineEx((Vector2){PUZZLE_VERT_X, PUZZLE_TOP_Y + 8.0f}, (Vector2){PUZZLE_VERT_X, PUZZLE_BOTTOM_Y}, 2.0f, RED);
        }

        // Draw X-axis
        DrawLineEx((Vector2){PUZZLE_VERT_X - PUZZLE_MAX_NIBBLE_LEN, PUZZLE_BOTTOM_Y},
                   (Vector2){PUZZLE_VERT_X + PUZZLE_MAX_NIBBLE_LEN, PUZZLE_BOTTOM_Y}, 2.0f, BLACK);

        // Draw L-shaped segments based on bytes
        DrawSegments((Vector2){PUZZLE_VERT_X, PUZZLE_TOP_Y},
                     (Vector2){PUZZLE_VERT_X, PUZZLE_BOTTOM_Y},
                     byte_count, 2.0f, BLACK, bytes, show_labels);

        EndMode2D();
        EndDrawing();
    }

    // Cleanup
    CloseWindow();
    return 0;
}