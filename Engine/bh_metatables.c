#include <metatables.h>

static int lua_call_func_ptr(lua_State* L) {
    // Retrieve the function pointer
    callback_t func = (callback_t)lua_touserdata(L, lua_upvalueindex(1));
    // Call the function
    func();
    return 0;
}

void push_linked_list_to_lua(lua_State* L, void* struct_ptr, FieldMeta* meta) {
    lua_newtable(L);  // Create a new table for the linked list

    if (struct_ptr == NULL) {
        return;
    }

    void* start = struct_ptr;
    void* node_ptr = struct_ptr;
    int index = 1;

    do {
        lua_pushinteger(L, index++);
        push_struct_to_lua(L, node_ptr, meta);
        lua_settable(L, -3);

        node_ptr = *(void**)((char*)node_ptr + meta->next_offset);
    } while (node_ptr != start && node_ptr != NULL);
}

void push_struct_to_lua(lua_State* L, void* struct_ptr, FieldMeta* meta) {
    if (struct_ptr == NULL) {
        lua_pushnil(L);
        return;
    }

    lua_newtable(L);  // Create a new table and push it onto the stack
    for (int i = 0; meta[i].name != NULL; i++) {
        lua_pushstring(L, meta[i].name);  // Push the field name

        void* field_ptr = (char*)struct_ptr + meta[i].offset;

        switch (meta[i].type) {
            case TYPE_STRING:
                lua_pushstring(L, *(const char**)field_ptr);
                break;

            case TYPE_INT:
                lua_pushinteger(L, *(int*)field_ptr);
                break;

            case TYPE_FLOAT:
                lua_pushnumber(L, *(float*)field_ptr);
                break;

            case TYPE_DOUBLE:
                lua_pushnumber(L, *(double*)field_ptr);
                break;

            case TYPE_BOOL:
                lua_pushboolean(L, *(int*)field_ptr);
                break;

            case TYPE_CHAR:
                lua_pushlstring(L, (char*)field_ptr, 1);  // Push single character as string
                break;

            case TYPE_FUNC_PTR:
                lua_pushlightuserdata(L, *(void**)field_ptr);
                lua_pushcclosure(L, lua_call_func_ptr, 1);
                break;

            case TYPE_ENUM:
                lua_pushstring(L, meta[i].enum_strings[*(int*)field_ptr]);
                break;

            case TYPE_VOID_PTR:
                lua_pushlightuserdata(L, *(void**)field_ptr);
                break;

            case TYPE_UINT:
                lua_pushinteger(L, *(unsigned int*)field_ptr);
                break;

            case TYPE_UCHAR:
                lua_pushinteger(L, *(unsigned char*)field_ptr);
                break;

            case TYPE_LINKED_LIST:
                push_linked_list_to_lua(L, *(void**)field_ptr, meta[i].list_meta);
                break;

            case TYPE_STRUCT:
                push_struct_to_lua(L, field_ptr, meta[i].sub_meta);
                break;

            case TYPE_STRUCT_PTR:
                if (*(void**)field_ptr != NULL) {
                    push_struct_to_lua(L, *(void**)field_ptr, meta[i].sub_meta);
                } else {
                    lua_pushnil(L);
                }
                break;

            default:
                luaL_error(L, "Unsupported field type for '%s'", meta[i].name);
                break;
        }

        lua_settable(L, -3);  // Set the table field
    }
}

int lua_to_enum(lua_State* L, int index, const char** enum_strings) {
    const char* str = lua_tostring(L, index);
    for (int i = 0; enum_strings[i] != NULL; i++) {
        if (strcmp(str, enum_strings[i]) == 0) {
            return i;
        }
    }
    return -1;  // Error handling can be improved
}

void lua_to_linked_list(lua_State* L, int index, void** struct_ptr, FieldMeta* meta) {
    if (!lua_istable(L, index)) {
        luaL_error(L, "Expected table at index %d", index);
    }

    int length = lua_rawlen(L, index);
    if (length == 0) {
        *struct_ptr = NULL;
        return;
    }

    void* head = NULL;
    void* tail = NULL;

    for (int i = 1; i <= length; i++) {
        lua_rawgeti(L, index, i);
        void* node = safe_malloc(meta->offset + sizeof(void*) * 2); // Allocate memory for each node
        lua_to_struct(L, -1, node, meta);
        lua_pop(L, 1);  // Remove the value from the stack

        if (head == NULL) {
            head = node;
        } else {
            *(void**)((char*)tail + meta->next_offset) = node;
            *(void**)((char*)node + meta->prev_offset) = tail;
        }
        tail = node;
    }

    *(void**)((char*)tail + meta->next_offset) = head; // Complete the circular link
    *(void**)((char*)head + meta->prev_offset) = tail;

    *struct_ptr = head;
}

void lua_to_struct(lua_State* L, int index, void* struct_ptr, FieldMeta* meta) {
    if (!lua_istable(L, index)) {
        luaL_error(L, "Expected table at index %d", index);
    }

    for (int i = 0; meta[i].name != NULL; i++) {
        lua_getfield(L, index, meta[i].name);

        void* field_ptr = (char*)struct_ptr + meta[i].offset;

        switch (meta[i].type) {
            case TYPE_STRING:
                *(const char**)field_ptr = lua_tostring(L, -1);
                break;

            case TYPE_INT:
                *(int*)field_ptr = lua_tointeger(L, -1);
                break;

            case TYPE_FLOAT:
                *(float*)field_ptr = lua_tonumber(L, -1);
                break;

            case TYPE_DOUBLE:
                *(double*)field_ptr = lua_tonumber(L, -1);
                break;

            case TYPE_BOOL:
                *(int*)field_ptr = lua_toboolean(L, -1);
                break;

            case TYPE_CHAR:
                *(char*)field_ptr = *lua_tostring(L, -1);
                break;

            case TYPE_FUNC_PTR:
                *(void**)field_ptr = lua_touserdata(L, -1);  // Handle function pointers as light userdata
                break;

            case TYPE_ENUM:
                *(int*)field_ptr = lua_to_enum(L, -1, meta[i].enum_strings);
                break;

            case TYPE_VOID_PTR:
                *(void**)field_ptr = lua_touserdata(L, -1);
                break;

            case TYPE_UINT:
                *(unsigned int*)field_ptr = lua_tointeger(L, -1);
                break;

            case TYPE_UCHAR:
                *(unsigned char*)field_ptr = lua_tointeger(L, -1);
                break;

            case TYPE_LINKED_LIST:
                lua_to_linked_list(L, -1, (void**)field_ptr, meta[i].list_meta);
                break;

            case TYPE_STRUCT:
                lua_to_struct(L, -1, field_ptr, meta[i].sub_meta);
                break;

            case TYPE_STRUCT_PTR:
                if (lua_istable(L, -1)) {
                    *(void**)field_ptr = safe_malloc(sizeof(meta[i].sub_meta));  // Allocate memory for the struct
                    lua_to_struct(L, -1, *(void**)field_ptr, meta[i].sub_meta);
                }
                break;

            default:
                luaL_error(L, "Unsupported field type for '%s'", meta[i].name);
                break;
        }

        lua_pop(L, 1);  // Remove the field value from the stack
    }
}

