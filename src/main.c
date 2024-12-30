#ifdef _WIN32
    #include <minimal_windows.h>
#endif

#include <core.h>
#include <custom3d.h>
#include <player.h>
#include <scene.h>

#ifdef __linux__
    #define DEBUG
#endif

#ifdef DEBUG
    #include <mcheck.h>
#endif

LuaContext* g_lua_context;
Engine* g_engine;

void main_load(Scene* scene) {
    printf("setup!\n");

    InitPlayer((Vector3){ 0.0f, 0.0f, 0.0f});

    // ToggleBorderlessWindowed();
    DisableCursor();
}

void main_render(Scene* scene) {
    // BeginMode3D(g_engine->camera);
    //     DrawCube(data->cube_pos, 2.0f,2.0f,2.0f, GREEN);
    //     DrawGrid(100.0f, 1.0f);
    // EndMode3D();

    // BeginMode3D(g_engine->camera);
    //     DrawCube(Vector3AddValue(data->cube_pos, 5), 2.0f,2.0f,2.0f, GREEN);
    //     DrawGrid(100.0f, 1.0f);
    // EndMode3D();

    // char cam_string[100];
    // snprintf(cam_string, 100, "Position: %f, %f, %f", g_engine->camera.position.x, g_engine->camera.position.y, g_engine->camera.position.z);
    // DrawText(cam_string, 10, 30, 20, GRAY);

    // snprintf(cam_string, 100, "Target: %f, %f, %f", g_engine->camera.target.x, g_engine->camera.target.y, g_engine->camera.target.z);
    // DrawText(cam_string, 10, 50, 20, GRAY);

    // snprintf(cam_string, 100, "Cube: %f, %f, %f", data->cube_pos.x, data->cube_pos.y, data->cube_pos.z);
    // DrawText(cam_string, 10, 70, 20, GRAY);
    BeginMode3D(*g_engine->camera);
        CreateGridWithLabels(2.0f, 1.0f, 1000.0f, g_engine->text_labels, GetFontDefault());
    EndMode3D();

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
    };

    Vector3 test = VEC3_ZERO;

    for (int i = 0; i < 3; ++i) { 
        for (int j=0; j < 3; ++j) {
            char *formattedString = NULL;
            asprintf(&formattedString, "Test:%d",i+j);
            t_Entity* e = create_entity3D(engine->entity_system, formattedString, "./resources/marisa.glb", test, VEC3_ZERO, PLAYER, FLT_MAX, (Vector3){ 1.5, 1.5, 1.5}, 100.0f);
            test = Vector3Add(test, (Vector3){ 2.0f, 0.0f, 0.0f});
            // e->update=SillyUpdate;
        }
        test = VEC3_ZERO;
        test.z = i*2;
    }

    test.y = 10;

    // t_Entity* ent = create_entity3D(engine->entity_system, "test11", "./resources/marisa.glb", test, VEC3_ZERO, PLAYER, FLT_MAX, Vector3Scale(VEC3_ONE, 1.5f), 100.0f);
    // ent->is_static = true;

    add_scene(engine->scene_manager, scene);
    load_scene(engine->scene_manager, "main");

    int retc = main_loop(engine);

    #ifdef DEBUG
        muntrace();
    #endif

    return retc;
}