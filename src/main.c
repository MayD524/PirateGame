#ifdef _WIN32
    #include <minimal_windows.h>
#endif

#include <core.h>
#include <water_shader.h>
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

LuaContext* g_lua_context;
Engine* g_engine;
WaterShader* waterShader;

const int gridSize = 50;       // Number of grid points along one axis
const float gridSpacing = 0.5f; // Distance between grid points

typedef struct {
    Mesh waterMesh;     // Mesh for the water plane
    Model waterModel;   // Model created from the water mesh
} TestScene;


void main_load(Scene* scene) {
    printf("Setup!\n");

    InitPlayer((Vector3){ 0.0f, 0.0f, 0.0f });

    // Initialize the water shader
    waterShader = InitWaterShader("shaders/water.vs", "shaders/water.fs");
    if (!waterShader) {
        printf("Failed to initialize water shader.\n");
        exit(EXIT_FAILURE);
    }

    // Initialize TestScene data
    TestScene* data = (TestScene*)scene->SceneData;

    // Generate water mesh
    int gridSize = 50;          // Number of grid points along one axis
    float gridSpacing = 0.5f;   // Distance between grid points
    data->waterMesh = GenWaterMesh(gridSize, gridSpacing);
    data->waterModel = LoadModelFromMesh(data->waterMesh);

    // Assign the shader to the water model
    data->waterModel.materials[0].shader = waterShader->shader;

    // Optional: ToggleBorderlessWindowed();
    DisableCursor();
    CreateGridWithLabels(10, 1, 1000, g_engine->text_labels, GetFontDefault());
}

void main_render(Scene* scene) {
    TestScene* data = (TestScene*)scene->SceneData;

    BeginMode3D(*g_engine->camera);
        Draw3DGrid(1, 1, 1000);
        // Update shader uniforms
        // UpdateWaterShader(waterShader, *g_engine->camera, GetTime());

        // Draw the water model
        // DrawWaterPlane(data->waterModel, waterShader);

    EndMode3D();

    // Draw crosshair
    int centerX = GetScreenWidth() / 2;
    int centerY = GetScreenHeight() / 2;

    DrawLine(centerX - 10, centerY, centerX + 10, centerY, BLACK); // Horizontal line
    DrawLine(centerX, centerY - 10, centerX, centerY + 10, BLACK); // Vertical line
}

void main_update(Scene* scene, float deltatime) {

}

void SillyUpdate(t_Entity* entity, float dt) {
    float rd = RAND_RANGE(0, 3);
    entity->entity3D.rotation += rd;
    if (entity->entity3D.rotation >= 360.0f)
        entity->entity3D.rotation -= 360.0f;

    entity->entity3D.rotation_axis = (Vector3){ 0.0f, 1.0f, 0.0f };
    entity->entity3D.velocity = (Vector3){RAND_RANGE(0, 10), 0.0f, 0.0f};
}

void main_unload(Scene* scene) {
    TestScene* data = (TestScene*)scene->SceneData;

    // Unload water shader
    UnloadWaterShader(waterShader);

    // Unload water model and mesh
    UnloadModel(data->waterModel);
    UnloadMesh(data->waterMesh);
}

int main() {
    #ifdef DEBUG
        mtrace();
    #endif
    
    WindowInformation* winfo = create_window_information(1080, 720, "Test", 60);
    Engine* engine = create_default_engine(winfo);

    TestScene scene_data = {
        .waterMesh = { 0 },  // Initialize to zero; will be set in main_load
        .waterModel = { 0 }   // Initialize to zero; will be set in main_load
    };

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
        .SceneData = &scene_data
    };

    Vector3 test = VEC3_ZERO;
    long entity_id = 0;
    
    for (int i = 0; i < 100; ++i) { 
        for (int j=0; j < 100; ++j) {
            char *formattedString = NULL;
            asprintf(&formattedString, "Test:%d",entity_id++);
            t_Entity* e = create_entity3D(engine->entity_system, formattedString, "./resources/marisa.glb", test, VEC3_ZERO, PLAYER, FLT_MAX, (Vector3){ 1.5, 1.5, 1.5}, 100.0f);
            test = Vector3Add(test, (Vector3){ 2.0f, 0.0f, 0.0f});
            // e->update=SillyUpdate;
        }
        test = VEC3_ZERO;
        test.z = i*2;
    }

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