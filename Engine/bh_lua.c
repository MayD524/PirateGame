#include <bh_lua.h>

#pragma region Helpers
FunctionType get_function_type(const char *type_str) {
    if (strcmp(type_str, "update") == 0) return UPDATE;
    if (strcmp(type_str, "pre-render") == 0) return PRE_RENDER;
    if (strcmp(type_str, "post-render") == 0) return POST_RENDER;
    if (strcmp(type_str, "on-awake") == 0) return ON_AWAKE;
    if (strcmp(type_str, "on-destroy") == 0) return ON_DESTROY;
    if (strcmp(type_str, "on-start") == 0) return ON_START;
    return FUNCTION_TYPE_COUNT; // Invalid type
}
#pragma endregion

#pragma region LUA handling
LuaContext* init_lua() {
    LuaContext* L = (LuaContext*)safe_malloc(sizeof(LuaContext));
    printf("%p\n", L);

    if (L == NULL) {
        panic("Could not allocate memory for LuaContext");
    }

    lua_State* l = luaL_newstate();
    luaL_openlibs(l);

    L->lua_state = l;
    L->func_refs = (FunctionReference*)safe_malloc(sizeof(FunctionReference) * INITIAL_CAPACITY);

    if (L->func_refs == NULL) {
        panic("Could not allocate memory for FunctionReference");
    }

    L->num_functions = 0;
    L->function_capacity = INITIAL_CAPACITY;

    register_base_functions(L);

    return L;
}

void close_lua(LuaContext *context) {
    for (int i = 0; i < context->num_functions; i++) {
        luaL_unref(context->lua_state, LUA_REGISTRYINDEX, context->func_refs[i].ref);
    }
    lua_close(context->lua_state);
    SAFE_FREE(context->func_refs);
    SAFE_FREE(context);
}

void bh_lua_error(lua_State* L) {
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
    lua_pop(L, 1); // pop error message from the stack
}

#pragma endregion

#pragma region C -> LUA 


void register_base_functions(LuaContext* context) {
    // basic entity functions
    lua_register(context->lua_state, "set_entity_active"       , lua_set_entity_active       );
    lua_register(context->lua_state, "get_entity_count"        , lua_get_entity_count        );
    lua_register(context->lua_state, "is_entity_active"        , lua_is_entity_active        );
    lua_register(context->lua_state, "create_entity"           , lua_create_entity           );
    lua_register(context->lua_state, "get_entity"              , lua_get_entity              );
    lua_register(context->lua_state, "heal_entity"             , lua_heal_entity             );
    lua_register(context->lua_state, "get_entity_health"       , lua_get_entity_health       );
    lua_register(context->lua_state, "deal_damage"             , lua_deal_damage             );
    lua_register(context->lua_state, "get_entity_by_name"      , lua_get_entity_by_name      );
    lua_register(context->lua_state, "print_movement"          , lua_print_movement          );
    lua_register(context->lua_state, "are_entities_colliding"  , lua_are_entities_colliding  );
    lua_register(context->lua_state, "set_velocity"            , lua_set_velocity            );
    lua_register(context->lua_state, "get_velocity"            , lua_get_velocity            );
    lua_register(context->lua_state, "set_position"            , lua_set_position            );
    lua_register(context->lua_state, "get_position"            , lua_get_position            );
    lua_register(context->lua_state, "set_rotation"            , lua_set_rotation            );
    lua_register(context->lua_state, "get_rotation"            , lua_get_rotation            );
    lua_register(context->lua_state, "get_entity_data"         , lua_get_entity_table        );

    // LUA -> C functions
    lua_register(context->lua_state, "register_function"       , register_lua_function       );
    
    // tag functions
    lua_register(context->lua_state, "print_tags"              , lua_print_tags              );
    lua_register(context->lua_state, "add_tag"                 , lua_add_tag                 );
    lua_register(context->lua_state, "get_tags"                , lua_get_tags                );
    lua_register(context->lua_state, "remove_tag_at"           , lua_remove_tag_at           );
    lua_register(context->lua_state, "remove_tag"              , lua_remove_tag              );
    lua_register(context->lua_state, "has_tag"                 , lua_has_tag                 );
    lua_register(context->lua_state, "remove_all_tags"         , lua_remove_all_tags         );
    
    // texture functions
    lua_register(context->lua_state, "load_texture"            , lua_load_texture            );
    lua_register(context->lua_state, "get_texture"             , lua_get_texture             );
    
    // screen functions
    lua_register(context->lua_state, "get_screen_size"         , lua_get_screen_size         );
    
    // drawing functions
    lua_register(context->lua_state, "draw_text"               , lua_draw_text               );
    lua_register(context->lua_state, "draw_texture"            , lua_draw_texture            );
    // lua_register(context->lua_state, "draw_sprite"             , lua_draw_sprite             );
    // lua_register(context->lua_state, "draw_sprite_batch"       , lua_draw_sprite_batch       );
    // lua_register(context->lua_state, "draw_texture_batch"      , lua_draw_texture_batch      );

    // 2d drawing
    lua_register(context->lua_state, "draw_line"               , lua_draw_line               );
    lua_register(context->lua_state, "draw_rectangle"          , lua_draw_rectangle          );
    lua_register(context->lua_state, "draw_triangle"           , lua_draw_triangle           );   
    lua_register(context->lua_state, "draw_circle"             , lua_draw_circle             );

    lua_register(context->lua_state, "load_gltf"               , lua_load_gltf               );
    lua_register(context->lua_state, "unload_model"            , lua_unload_model            );
    lua_register(context->lua_state, "draw_model"              , lua_draw_model              );
    lua_register(context->lua_state, "begin_draw3d"            , lua_begin_draw3d            );
    lua_register(context->lua_state, "end_draw3d"              , lua_end_draw3d              );
    lua_register(context->lua_state, "anim_total_frames"       , lua_get_total_frames        );
    lua_register(context->lua_state, "model_total_anims"       , lua_get_total_animations    );
    lua_register(context->lua_state, "update_animation"        , lua_update_animation        );
    lua_register(context->lua_state, "lupdate_animation"       , lua_update_animation_looping);

    // 3d drawing
    lua_register(context->lua_state, "draw_cube"               , lua_draw_cube               );
    
}

#pragma endregion

#pragma region LUA -> C
int register_lua_function(lua_State* l){
    if (g_lua_context == NULL) {
        panic ("Could not get LuaContext from upvalue");
    }

    if (g_lua_context->num_functions >= g_lua_context->function_capacity) {
        printf("safe_reallocating FunctionReference array\n");
        g_lua_context->function_capacity *= 2;
        g_lua_context->func_refs = (FunctionReference*)safe_realloc(g_lua_context->func_refs, sizeof(FunctionReference) * g_lua_context->function_capacity);
        if (g_lua_context->func_refs == NULL) {
            panic("Could not allocate more memory for FunctionReference");
        }
    }

    const char* function_type_str = luaL_checkstring(l, 1);
    FunctionType function_type = get_function_type(function_type_str);
    if (function_type == FUNCTION_TYPE_COUNT) {
        return luaL_error(l, "Invalid function type: %s", function_type_str);
    }

    lua_pushvalue(l, 2); // Copy the function to the top of the stack
    int ref = luaL_ref(l, LUA_REGISTRYINDEX);

    printf("g_lua_context->func_refs: %p\n", g_lua_context->func_refs);
    if (g_lua_context->func_refs == NULL) {
        panic("g_lua_context->func_refs is NULL, something went wrong");
    }
    g_lua_context->func_refs[g_lua_context->num_functions].type = function_type;
    g_lua_context->func_refs[g_lua_context->num_functions].ref = ref;
    g_lua_context->num_functions++;

    return 0;
}

void execute_lua_file(lua_State* L, const char* filename) {
    if (luaL_dofile(L, filename) != LUA_OK) {
        bh_lua_error(L);
    }
}

void call_functions(LuaContext *context, FunctionType type) {
    // printf("We have %d\n", context->num_functions);
    for (int i = 0; i < context->num_functions; i++) {
        if (context->func_refs[i].type == type && type != UPDATE) {
            lua_rawgeti(context->lua_state, LUA_REGISTRYINDEX, context->func_refs[i].ref);
            if (lua_pcall(context->lua_state, 0, 0, 0) != LUA_OK) {
                fprintf(stderr, "Error in %s function: %s\n", 
                        type == UPDATE ? "update" :
                        type == PRE_RENDER ? "pre-render" :
                        type == POST_RENDER ? "post-render" :
                        type == ON_AWAKE ? "on-awake" :
                        type == ON_DESTROY ? "on-destroy" :
                        "on-start", lua_tostring(context->lua_state, -1));
                lua_pop(context->lua_state, 1); // remove error message from stack
            }
        } else if (context->func_refs[i].type == type && type == UPDATE) {
            lua_rawgeti(context->lua_state, LUA_REGISTRYINDEX, context->func_refs[i].ref);
            lua_pushnumber(context->lua_state, GetFrameTime());
            if (lua_pcall(context->lua_state, 1, 1, 0) != LUA_OK) {
                fprintf(stderr, "Error in update function: %s\n", lua_tostring(context->lua_state, -1));
                lua_pop(context->lua_state, 1); // remove error message from stack
            }
            
        }
    }
}

#pragma endregion