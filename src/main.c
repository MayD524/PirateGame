#ifdef _WIN32
    #include <minimal_windows.h>
#endif

#include <core.h>
#include <water_plane.h>
#include <custom3d.h>
#include <player.h>
#include <scene.h>

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
WaterPlane* waterPlane;

const int gridSize = 50;       // Number of grid points along one axis
const float gridSpacing = 0.5f; // Distance between grid points


void main_load(Scene* scene) {
    printf("Setup!\n");

    InitPlayer((Vector3){ 0.0f, 0.0f, 0.0f });
    waterPlane = safe_malloc(sizeof(WaterPlane));
    // Initialize the water shader
    *waterPlane = CreateWaterPlane((Vector3){ 0.0f, 1.0f, 0.0f}, (Vector2){ 1000.0f, 1000.0f}, WATER_ROWS, WATER_COLS, BLUE, 0.5f, 1.0f);
    

    // Optional: ToggleBorderlessWindowed();
    DisableCursor();
    CreateGridWithLabels(10, 1, 1000, g_engine->text_labels, GetFontDefault());
}

void main_render(Scene* scene) {

    BeginMode3D(*g_engine->camera);
        Draw3DGrid(1, 1, 1000);
        // Update shader uniforms
        DrawWaterPlane_SIMD(waterPlane, g_engine->camera->position, MAX_DRAW_DISTANCE*3);

    EndMode3D();

    // Draw crosshair
    int centerX = GetScreenWidth() / 2;
    int centerY = GetScreenHeight() / 2;

    DrawLine(centerX - 10, centerY, centerX + 10, centerY, BLACK); // Horizontal line
    DrawLine(centerX, centerY - 10, centerX, centerY + 10, BLACK); // Vertical line
}

void main_update(Scene* scene, float deltatime) {
    UpdateWaterPlane(waterPlane, PHYSICS_TIME);
}

void SillyUpdate(t_Entity* entity, float dt) {
    float waveHeight = GetWaveHeight(waterPlane, entity->entity3D.position.x, entity->entity3D.position.z);
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
    DestroyWaterPlane(waterPlane);
}

int main() {
    #ifdef DEBUG
        mtrace();
    #endif
    
    WindowInformation* winfo = create_window_information(1080, 720, "Test", 60);
    Engine* engine = create_default_engine(winfo);

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
    //         // e->update=SillyUpdate;
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