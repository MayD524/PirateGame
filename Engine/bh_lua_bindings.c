#include <bh_lua_bindings.h>
#include "core.h"

Color bh_lua_color_unpack(lua_State* L, int index) {
    if (!lua_istable(L, index)) { 
        return WHITE;
    }
    
    lua_getfield(L, index, "r"); // Get the field "r" from the table
    int color_r = (int)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "g"); // Get the field "g" from the table
    int color_g = (int)lua_tonumber(L, -1);
    lua_pop(L, 1); 

    lua_getfield(L, index, "b"); // Get the field "b" from the table
    int color_b = (int)lua_tonumber(L, -1);
    lua_pop(L, 1); 

    lua_getfield(L, index, "a"); // Get the field "a" from the table
    int color_a = lua_isnil(L, -1) ? 255 : (int)lua_tonumber(L, -1);
    lua_pop(L, 1);

    return (Color){color_r, color_g, color_b, color_a};
}

Vector2 bh_lua_vec2_unpack(lua_State* L, int index) {
    if (!lua_istable(L, index)) { 
        return VEC2_ZERO;
    }

    Vector2 vec2;

    lua_getfield(L, index, "x"); // Get the field "x" from the table
    vec2.x = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "y"); // Get the field "y" from the table
    vec2.y = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);

    return vec2;
}

Vector3 bh_lua_vec3_unpack(lua_State* L, int index) {
    if (!lua_istable(L, index)) { 
        return VEC3_ZERO;
    }

    Vector3 vec3;

    lua_getfield(L, index, "x"); // Get the field "x" from the table
    vec3.x = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "y"); // Get the field "y" from the table
    vec3.y = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "z"); // Get the field "y" from the table
    vec3.z = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);

    return vec3;
}

Quaternion bh_lua_quaternion_unpack(lua_State *L, int index) {
    luaL_checktype(L, index, LUA_TTABLE); // Ensure the argument is a table

    Quaternion quat;

    lua_getfield(L, index, "x"); // Get "x" field
    quat.x = luaL_optnumber(L, -1, 0.0f); // Default to 0.0 if not present
    lua_pop(L, 1);

    lua_getfield(L, index, "y"); // Get "y" field
    quat.y = luaL_optnumber(L, -1, 0.0f); // Default to 0.0 if not present
    lua_pop(L, 1);

    lua_getfield(L, index, "z"); // Get "z" field
    quat.z = luaL_optnumber(L, -1, 0.0f); // Default to 0.0 if not present
    lua_pop(L, 1);

    lua_getfield(L, index, "w"); // Get "w" field
    quat.w = luaL_optnumber(L, -1, 1.0f); // Default to 1.0 if not present
    lua_pop(L, 1);

    return quat;
}


int lua_create_entity(lua_State *L) {
    const char* entity_name = lua_tostring(L, 1);
    const char* texturePath = lua_tostring(L, 2);
    Vector3 start_position = bh_lua_vec3_unpack(L, 3);
    Vector3 start_velocity = bh_lua_vec3_unpack(L, 4);
    int priority_rank = lua_tointeger(L, 5);
    float max_life_time = (float)lua_tonumber(L, 6);
    Vector3 scale = bh_lua_vec3_unpack(L, 7);
    float health = (float)lua_tonumber(L, 8);

    t_Entity* entity = create_entity3D(g_entity_system, (char*)entity_name, (char*)texturePath, start_position, start_velocity, priority_rank, max_life_time, scale, health);
    lua_pushlightuserdata(L, entity);
    return 1;
}

int lua_is_entity_active(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    lua_pushboolean(L, !entity || entity->is_active);
    return 1;
}

int lua_get_position(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    if (!entity) {
        panic("lua_get_position: entity is null");
    }
    Vector3 position = entity->entity3D.position;
    lua_pushnumber(L, position.x);
    lua_pushnumber(L, position.y);
    lua_pushnumber(L, position.z);
    return 2;
}

int lua_set_position(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }

    if (!entity) {
        panic("lua_get_position: entity is null");
    }

    float x = (float)lua_tonumber(L, 2);
    float y = (float)lua_tonumber(L, 3);
    float z = (float)lua_tonumber(L, 4);
    Vector3 position = (Vector3){ x, y, z };

    entity->entity3D.position = position;
    return 0;
}

int lua_get_entity_by_name(lua_State *L) {
    const char* entity_name = lua_tostring(L, 1);
    t_Entity* entity = get_entity_by_name(g_entity_system, (char*)entity_name);
    lua_pushlightuserdata(L, entity);
    return 1;
}

int lua_get_entity_count(lua_State *L) {
    int count = g_entity_system->num_entities;
    lua_pushinteger(L, count);
    return 1;
}

int lua_get_entity(lua_State *L) {
    int index = lua_tointeger(L, 1);
    t_Entity* entity = get_entity(g_entity_system, index);
    lua_pushlightuserdata(L, entity);
    return 1;
}

int lua_print_movement(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    print_movement(entity);
    return 0;
}

int lua_are_entities_colliding(lua_State *L) {
    t_Entity* entity_a = (t_Entity*)lua_touserdata(L, 1);
    t_Entity* entity_b = (t_Entity*)lua_touserdata(L, 2);
    bool result = are_entities_colliding(entity_a, entity_b);
    lua_pushboolean(L, result);
    return 1;
}

int lua_deal_damage(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    float damage = (float)lua_tonumber(L, 2);
    entity->health -= damage;
    return 0;
}

int lua_set_entity_active(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    bool active = lua_toboolean(L, 2);
    entity->is_active = active;
    return 0;
}

int lua_print_tags(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    print_tags(entity);
    return 0;
}

int lua_add_tag(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    const char* tag = lua_tostring(L, 2);
    add_tag(entity, tag);
    return 0;
}

int lua_get_tags(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    const char** tags = get_tags(entity);

    lua_newtable(L);
    for (int i = 0; tags[i] != NULL; i++) {
        lua_pushstring(L, tags[i]);
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

int lua_remove_tag_at(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    int index = lua_tointeger(L, 2);
    remove_tag_at(entity, index);
    return 0;
}

int lua_remove_tag(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    const char* tag = lua_tostring(L, 2);
    remove_tag(entity, tag);
    return 0;
}

int lua_has_tag(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    const char* tag = lua_tostring(L, 2);
    int result = has_tag(entity, tag);
    lua_pushboolean(L, result != -1);
    return 1;
}

int lua_remove_all_tags(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    remove_all_tags(entity);
    return 0;
}

// Texture functions

int lua_load_texture(lua_State *L) {
    const char* texture_path = lua_tostring(L, 1);
    int texture_id = load_texture(g_entity_system, (char*)texture_path);
    lua_pushinteger(L, texture_id);
    return 1;
}

int lua_get_texture(lua_State *L) {
    int texture_id = lua_tointeger(L, 1);
    Texture2D texture = get_texture(g_entity_system, texture_id);
    lua_pushlightuserdata(L, &texture);
    return 1;
}

int lua_heal_entity(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    if (entity == NULL) { return 0; }

    float amount = (float)lua_tonumber(L, 2);
    entity->health += amount;
    return 0;
}

int lua_get_entity_health(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    if (!entity)
        lua_pushnil(L);
    else
        lua_pushnumber(L, entity->health);
    return 1;
}


// Drawing functions

int lua_draw_text(lua_State *L) {
    const char* text = lua_tostring(L, 1);
    Vector2 pos = bh_lua_vec2_unpack(L, 2);
    float scale = (float)lua_tonumber(L, 3);

    Color color = bh_lua_color_unpack(L, 4);
    DrawText(text, pos.x, pos.y, scale, color);
    return 0;
}

int lua_draw_texture(lua_State *L) {
    Texture2D* texture = (Texture2D*)lua_touserdata(L, 1);
    Vector2 position = bh_lua_vec2_unpack(L, 2);
    float rotation = (float)lua_tonumber(L, 3);
    float scale = (float)lua_tonumber(L, 4);
    Color tint = bh_lua_color_unpack(L, 5);

    DrawTextureEx(*texture, position, rotation, scale, tint);
    return 0;
}

int lua_draw_line(lua_State *L) {
    Vector2 start = bh_lua_vec2_unpack(L, 1);
    Vector2 end = bh_lua_vec2_unpack(L, 2);
    Color color = bh_lua_color_unpack(L, 3);
    float thickness = (float)lua_tonumber(L, 4);

    DrawLine(start.x, start.y, end.x, end.y, color);
    return 0;
}

int lua_draw_rectangle(lua_State *L) {
    Vector2 top_left = bh_lua_vec2_unpack(L, 1);
    Vector2 bottom_right = bh_lua_vec2_unpack(L, 2);
    Color color = bh_lua_color_unpack(L, 3);
    float thickness = (float)lua_tonumber(L, 4);

    bool filled = lua_toboolean(L, 5);
    if (filled) {
        DrawRectangle(top_left.x, top_left.y, bottom_right.x - top_left.x, bottom_right.y - top_left.y, color);
    } else {
        DrawRectangleLines(top_left.x, top_left.y, bottom_right.x - top_left.x, bottom_right.y - top_left.y, color);
    }
    return 0;
}

int lua_draw_triangle(lua_State *L) {
    Vector2 v1 = bh_lua_vec2_unpack(L, 1);
    Vector2 v2 = bh_lua_vec2_unpack(L, 2);
    Vector2 v3 = bh_lua_vec2_unpack(L, 3);
    Color color = bh_lua_color_unpack(L, 4);
    float thickness = (float)lua_tonumber(L, 5);
    bool filled = lua_toboolean(L, 6);

    if (filled) {
        DrawTriangle(v1, v2, v3, color);
    } else {
        DrawTriangleLines(v1, v2, v3, color);
    }
    return 0;
}

int lua_draw_circle(lua_State *L) {
    Vector2 center = bh_lua_vec2_unpack(L, 1);
    float radius = (float)lua_tonumber(L, 2);
    Color color = bh_lua_color_unpack(L, 3);
    float thickness = (float)lua_tonumber(L, 4);
    bool filled = lua_toboolean(L, 5);
    
    if (filled) {
        DrawCircle(center.x, center.y, radius, color);
    } else {
        DrawCircleLines(center.x, center.y, radius, color);
    }
    return 0;
}

int lua_draw_plane(lua_State *L);

int lua_draw_cube(lua_State *L) {
    Vector3 position = bh_lua_vec3_unpack(L, 1);
    Vector3 size = bh_lua_vec3_unpack(L, 2);
    Color color = bh_lua_color_unpack(L, 3);

    DrawCubeV(position, size, color);
    return 0;
}

int lua_draw_pyramid(lua_State *L);
int lua_draw_sphere(lua_State *L);

int lua_begin_draw3d(lua_State *L) {
    if (g_engine->camera)
        BeginMode3D(*g_engine->camera);
    else {
        luaL_error(L, "Failed to begin drawing 3d as camera is not set!");
    }
    return 0;
}

int lua_end_draw3d(lua_State *L) {
    EndMode3D();
    return 0;
}

int lua_load_gltf(lua_State *L) {
    const char* file_path = luaL_checkstring(L, 1);

    // Store the model in the model manager
    int model_id = add_model_info(file_path);
    if (model_id == -1) {
        lua_pushstring(L, "Failed to load GLTF/GLB file");
        lua_error(L);
        return 1;
    }

    // Return the model ID to Lua
    lua_pushinteger(L, model_id);
    return 1;
}

int lua_update_animation(lua_State *L) {
    int model_id = luaL_checkinteger(L, 1);
    int animationIndex = luaL_checkinteger(L, 2);
    float frame = luaL_checknumber(L, 3);

    ModelInfo* minfo = get_model_info(model_id);
    if (model_id == -1) {
        lua_pushstring(L, "Failed to get model by id");
        lua_error(L);
        return 1;
    }

    update_model_animation(minfo, animationIndex, frame, false);

    return 0;
}

int lua_update_animation_looping(lua_State *L) {
    int model_id = luaL_checkinteger(L, 1);
    int animationIndex = luaL_checkinteger(L, 2);
    float frame = luaL_checknumber(L, 3);

    ModelInfo* minfo = get_model_info(model_id);
    if (model_id == -1) {
        lua_pushstring(L, "Failed to get model by id");
        lua_error(L);
        return 1;
    }

    update_model_animation(minfo, animationIndex, frame, true);

    return 0;
}

int lua_get_total_animations(lua_State *L) {
    int model_id = luaL_checkinteger(L, 1);
    
    ModelInfo* minfo = get_model_info(model_id);
    if (model_id == -1) {
        lua_pushstring(L, "Failed to get model by id");
        lua_error(L);
        return 1;
    }
    lua_pushinteger(L, get_model_animation_count(minfo));
    return 1;
}

int lua_get_total_frames(lua_State *L) {
    int model_id = luaL_checkinteger(L, 1);
    int animationIndex = luaL_checkinteger(L, 2);

    ModelInfo* minfo = get_model_info(model_id);
    if (model_id == -1) {
        lua_pushstring(L, "Failed to get model by id");
        lua_error(L);
        return 1;
    }

    lua_pushinteger(L, get_animation_total_frames(minfo, animationIndex));
    return 1;
}

int lua_unload_model(lua_State *L) {
    int model_id = luaL_checkinteger(L, 1);

    // Remove the model from the model manager
    remove_model_info(model_id);

    return 0;
}

int lua_draw_model(lua_State *L) {
    int model_id = luaL_checkinteger(L, 1);
    Vector3 position = bh_lua_vec3_unpack(L, 2);
    Vector3 scale = bh_lua_vec3_unpack(L, 3);
    Quaternion rotation = bh_lua_quaternion_unpack(L, 4);

    // Retrieve the model from the model manager
    ModelInfo* info = get_model_info(model_id);
    if (!info) {
        lua_pushstring(L, "Invalid model ID");
        lua_error(L);
        return 0;
    }

    // Draw the model
    Vector3 eulerRotation = QuaternionToEuler(rotation);
    float angle = eulerRotation.y; // Assuming Y-axis rotation for 3D models
    DrawModelEx(info->model, position, (Vector3){0.0f, 1.0f, 0.0f}, angle, scale, WHITE);

    return 0;
}


// entity functions

int lua_set_entity_rotation(lua_State *L){ 
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    if (entity == NULL) { return 0; }

    float rotation = luaL_checknumber(L, 2);
    entity->entity3D.rotation = rotation;
    return 0;
}

int lua_get_entity_rotation(lua_State *L){ 
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    if (!entity)
        lua_pushnil(L);
    else {
        push_struct_to_lua(L, &entity->entity3D.rotation, Vector3Meta);

    }
    return 1;
}

int lua_set_scale(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    if (entity == NULL) { return 0; }

    Vector3 scale = bh_lua_vec3_unpack(L, 2);
    entity->entity3D.scale = scale;
    return 0;
}

int lua_set_velocity(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    if (entity == NULL) { return 0; }

    Vector3 velocity = bh_lua_vec3_unpack(L, 2);
    entity->entity3D.velocity = velocity;
    return 0;
}

int lua_get_velocity(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    push_struct_to_lua(L, &entity->entity3D.velocity, Vector3Meta);
    return 1;
}

int lua_set_rotation(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    float rotation = luaL_checknumber(L, 2);
    entity->entity3D.rotation = rotation;
    return 0;
}

int lua_get_rotation(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    push_struct_to_lua(L, &entity->entity3D.rotation, Vector3Meta);
    return 1;
}

int lua_get_entity_table(lua_State *L) {
    t_Entity* entity = (t_Entity*)lua_touserdata(L, 1);
    if (entity == NULL) { return 0; }
    if (entity != NULL && entity->is_active) 
        push_struct_to_lua(L, entity, EntityMeta);
    else
        lua_pushnil(L);
    return 1;
}

int lua_get_screen_size(lua_State *L) {
    Vector2 size = { .x = GetScreenWidth(),.y = GetScreenHeight() };
    push_struct_to_lua(L, &size, Vector2Meta);
    return 1;
}