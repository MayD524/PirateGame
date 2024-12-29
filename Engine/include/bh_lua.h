#pragma once

#ifndef BH_LUA_H
#define BH_LUA_H

#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>

#include <util.h>
#include <metatables.h>
#include <extended_memory.h>
#include <bh_lua_bindings.h>

#define INITIAL_CAPACITY 10


typedef enum {
    UPDATE,
    PRE_RENDER,
    POST_RENDER,
    ON_AWAKE,
    ON_DESTROY,
    ON_START,
    FUNCTION_TYPE_COUNT
} FunctionType;

typedef struct {
    int ref;
    FunctionType type;
} FunctionReference;

typedef struct {
    lua_State* lua_state;

    FunctionReference* func_refs;
    int num_functions;
    int function_capacity;

} LuaContext;

extern LuaContext* g_lua_context;

LuaContext* init_lua();
void close_lua(LuaContext* l);
void register_function(LuaContext* l, const char *name, lua_CFunction func);
void register_base_functions(LuaContext* l);

void bh_lua_error(lua_State* l);


FunctionType get_function_type(const char *type_str);


void call_functions(LuaContext* lua_context, FunctionType function_type);
void call_lua_function(lua_State* l, const char *function_name);
void register_global_function(lua_State* l, const char *name, lua_CFunction func);
int register_lua_function(lua_State* l);
void execute_lua_file(lua_State* l, const char *filename);


#endif // BH_LUA_H