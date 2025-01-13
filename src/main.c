#ifdef _WIN32
    #include <minimal_windows.h>
#endif

#include <core.h>
#include <water_plane.h>
#include <custom3d.h>
#include <player.h>
#include <scene.h>
#include <pirate_game.h>

#define DEBUG_DONT_HIDE

#ifdef __linux__
    #define DEBUG
#endif

#ifdef DEBUG
    #include <mcheck.h>
#endif

#define WATER_COLS 1000
#define WATER_ROWS 1000

LuaContext* g_lua_context;
Engine* g_engine;
PirateGame* pirateGame;

void DrawSimpleBoat(Vector3 position, Vector3 size, float rotation, Color hullColor, Color deckColor) {
    // Curved Hull (using a series of cylinders to approximate curvature)
    float hullHeight = size.y * 0.5f;
    float hullWidth = size.x;
    float hullDepth = size.z;
    int hullSegments = 8;
    for (int i = 0; i < hullSegments; i++) {
        float t = (float)i / (hullSegments - 1); // Normalized position (0 to 1)
        float segmentHeight = hullHeight * (1.0f - t * t); // Parabolic curve
        float segmentWidth = hullWidth * (1.0f - t * 0.6f); // Narrower at ends
        float segmentDepth = hullDepth;

        Vector3 segmentPosition = (Vector3){
            position.x + (t - 0.5f) * hullWidth,
            position.y + segmentHeight / 2,
            position.z
        };

        DrawCube(segmentPosition, segmentWidth, segmentHeight, segmentDepth, hullColor);
    }

    // Deck (slightly inset rectangle)
    Vector3 deckSize = (Vector3){hullWidth * 0.8f, hullHeight * 0.1f, hullDepth * 0.9f};
    DrawCube((Vector3){position.x, position.y + hullHeight, position.z}, deckSize.x, deckSize.y, deckSize.z, deckColor);

    // Mast (two vertical masts)
    float mastHeight = size.y * 1.5f;
    float mastRadius = size.x * 0.05f;
    float mastOffset = size.z * 0.3f;
    DrawCylinder((Vector3){position.x, position.y + hullHeight + mastHeight / 2, position.z - mastOffset}, mastRadius, mastRadius, mastHeight, 16, DARKBROWN);
    DrawCylinder((Vector3){position.x, position.y + hullHeight + mastHeight / 2, position.z + mastOffset}, mastRadius, mastRadius, mastHeight, 16, DARKBROWN);

    // Sails (rectangles on masts)
    Vector3 sailSize = (Vector3){mastHeight * 0.6f, mastHeight * 0.9f, 0.1f};
    DrawCube((Vector3){position.x, position.y + hullHeight + mastHeight * 0.6f, position.z - mastOffset}, sailSize.x, sailSize.y, sailSize.z, WHITE);
    DrawCube((Vector3){position.x, position.y + hullHeight + mastHeight * 0.6f, position.z + mastOffset}, sailSize.x, sailSize.y, sailSize.z, WHITE);

    // Railings (thin rectangles around the deck)
    float railingHeight = size.y * 0.1f;
    DrawCube((Vector3){position.x, position.y + hullHeight + deckSize.y + railingHeight / 2, position.z - deckSize.z / 2}, deckSize.x, railingHeight, mastRadius * 2, LIGHTGRAY);
    DrawCube((Vector3){position.x, position.y + hullHeight + deckSize.y + railingHeight / 2, position.z + deckSize.z / 2}, deckSize.x, railingHeight, mastRadius * 2, LIGHTGRAY);
    DrawCube((Vector3){position.x - deckSize.x / 2, position.y + hullHeight + deckSize.y + railingHeight / 2, position.z}, mastRadius * 2, railingHeight, deckSize.z, LIGHTGRAY);
    DrawCube((Vector3){position.x + deckSize.x / 2, position.y + hullHeight + deckSize.y + railingHeight / 2, position.z}, mastRadius * 2, railingHeight, deckSize.z, LIGHTGRAY);

    // Rudder (thin rectangle at the back)
    Vector3 rudderSize = (Vector3){size.x * 0.1f, size.y * 0.3f, size.z * 0.05f};
    DrawCube((Vector3){position.x - hullWidth / 2 - rudderSize.x / 2, position.y + hullHeight / 2, position.z}, rudderSize.x, rudderSize.y, rudderSize.z, DARKGRAY);

    // Cabin (small cube on deck)
    Vector3 cabinSize = (Vector3){size.x * 0.4f, size.y * 0.3f, size.z * 0.4f};
    DrawCube((Vector3){position.x, position.y + hullHeight + deckSize.y + cabinSize.y / 2, position.z}, cabinSize.x, cabinSize.y, cabinSize.z, BROWN);
}

void main_load(Scene* scene) {
    printf("Setup!\n");

    InitPlayer((Vector3){ 0.0f, 0.0f, 0.0f });
    pirateGame->waterPlane = safe_malloc(sizeof(WaterPlane));
    // Initialize the water shader
    *pirateGame->waterPlane = CreateWaterPlane((Vector3){ 0.0f, 2.0f, 0.0f}, (Vector2){ 1000.0f, 1000.0f}, WATER_ROWS, WATER_COLS, BLUE, 0.5f, 1.0f);
    
    // Optional: ToggleBorderlessWindowed();
    DisableCursor();
    CreateGridWithLabels(10, 1, 1000, g_engine->text_labels, GetFontDefault());
}

void main_render(Scene* scene) {

    BeginMode3D(*g_engine->camera);
        Draw3DGrid(1, 1, 1000);
        // Update shader uniforms
        DrawWaterPlane_SIMD(pirateGame->waterPlane, g_engine->camera->position, MAX_DRAW_DISTANCE*2);
        DrawSimpleBoat(VEC3_ZERO, (Vector3){ 4.0f, 2.0f, 2.0f}, 0, RED, BLUE);
    EndMode3D();

    // Draw crosshair
    int centerX = GetScreenWidth() / 2;
    int centerY = GetScreenHeight() / 2;

    DrawLine(centerX - 10, centerY, centerX + 10, centerY, BLACK); // Horizontal line
    DrawLine(centerX, centerY - 10, centerX, centerY + 10, BLACK); // Vertical line
}

void main_update(Scene* scene, float deltatime) {
    UpdateWaterPlane(pirateGame->waterPlane, PHYSICS_TIME);
}

void SillyUpdate(t_Entity* entity, float dt) {
    float waveHeight = GetWaveHeight(pirateGame->waterPlane, entity->entity3D.position.x, entity->entity3D.position.z);
    // printf("WaveHeight: %f\n", waveHeight);
    float distanceToSurface = waveHeight - entity->entity3D.position.y;
    Vector3 correctiveForce = (Vector3){ 0.0f, distanceToSurface * 1000.0f, 0.0f }; // Adjust multiplier for responsiveness
    
    // printf("Distance: %f\n", distanceToSurface);
    // printf("DistanceCorrective: %f\n",distanceToSurface * 100.0f);
    // entity->entity3D.position.y = waveHeight; 
    Physics_ApplyForce(entity, correctiveForce);
    entity->in_water = true;
}

void main_unload(Scene* scene) {
    DestroyWaterPlane(pirateGame->waterPlane);
}

int main() {
    #ifdef DEBUG
        mtrace();
    #endif
    
    WindowInformation* winfo = create_window_information(1080, 720, "Test", 60);
    Engine* engine = create_default_engine(winfo);

    pirateGame = PirateGame_CreateGame();
    g_lua_context = engine->lua_context;
    g_entity_system = engine->entity_system;
    g_engine = engine;
    initialize_engine(engine);

    Scene scene = (Scene) {
        .is_active = false,
        .name = "main",
        .on_update = main_update,
        .on_load = main_load,
        .on_render = main_render,
        .on_unload = main_unload,
    };

    Vector3 test = { 10, 10, 10 };
    long entity_id = 0;
    
    // for (int i = 0; i < 100; ++i) { 
    //     for (int j=0; j < 100; ++j) {
    //         char *formattedString = NULL;
    //         asprintf(&formattedString, "Test:%d",entity_id++);
    //         t_Entity* e = create_entity3D(engine->entity_system, formattedString, "./resources/marisa.glb", test, VEC3_ZERO, PLAYER, FLT_MAX, (Vector3){ 1.5, 1.5, 1.5}, 100.0f);
    //         test = Vector3Add(test, (Vector3){ 2.0f, 0.0f, 0.0f});
    //         e->update=SillyUpdate;
    //     }
    //     test = VEC3_ZERO;
    //     test.z = i*2;
    // }
    
    t_Entity* e = create_entity3D(engine->entity_system, "TEST", "./resources/marisa.glb", test, VEC3_ZERO, PLAYER, FLT_MAX, (Vector3){ 1.5, 1.5, 1.5}, 100.0f);
    e->update = SillyUpdate;
    // t_Entity* ent = create_entity3D(engine->entity_system, "test11", "./resources/test_ship_small.glb", test, VEC3_ZERO, PLAYER, FLT_MAX, Vector3Scale(VEC3_ONE, 0.075f), 100.0f);
    // ent->is_static = true;
    // add_tag(ent, "NO_COLLISION");
    // ent->entity3D.rotation = 180 ;
    // ent->entity3D.rotation_axis = (Vector3) { 0.0f, 1.0f, 1.0f };

    add_scene(engine->scene_manager, scene);
    load_scene(engine->scene_manager, "main");

    int retc = main_loop(engine);

    #ifdef DEBUG
        muntrace();
    #endif

    return retc;
}