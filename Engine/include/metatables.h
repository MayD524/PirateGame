#pragma once

#ifndef BH_METATABLES_H
#define BH_METATABLES_H

#include <scene.h>
#include <stddef.h>
#include <raylib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include <util.h>
#include <extended_memory.h>
#include <entity.h>

typedef enum {
    TYPE_STRING,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_DOUBLE,
    TYPE_CHAR,
    TYPE_FUNC_PTR,
    TYPE_ENUM,
    TYPE_VOID_PTR,
    TYPE_STRUCT,
    TYPE_UINT,
    TYPE_UCHAR,
    TYPE_LINKED_LIST,

    // Added pointer types
    TYPE_INT_PTR,
    TYPE_FLOAT_PTR,
    TYPE_DOUBLE_PTR,
    TYPE_UCHAR_PTR,
    TYPE_USHORT_PTR,
    TYPE_UINT_PTR,
    TYPE_STRUCT_PTR,

    // Added array types
    TYPE_STRING_ARRAY,
    TYPE_FLOAT_ARRAY,
    TYPE_INT_ARRAY
} FieldType;


typedef struct FieldMeta {
    const char* name;
    FieldType type;
    size_t offset;
    struct FieldMeta* sub_meta;  // For nested structs
    const char** enum_strings;   // For enum types
    struct FieldMeta* list_meta; // For linked lists
    size_t next_offset;          // Offset of the next pointer for linked lists
    size_t prev_offset;          // Offset of the prev pointer for doubly linked lists
} FieldMeta;

typedef void (*callback_t)(void);

void push_struct_to_lua(lua_State* L, void* struct_ptr, FieldMeta* meta);
int lua_to_enum(lua_State* L, int index, const char** enum_strings);
void lua_to_struct(lua_State* L, int index, void* struct_ptr, FieldMeta* meta);

#define FIELD_UINT_PTR(struct_type, field) { #field, TYPE_UINT_PTR, offsetof(struct_type, field), NULL, NULL, NULL, 0, 0 }
#define FIELD_STRUCT_PTR(struct_type, field, sub_meta) { #field, TYPE_STRUCT_PTR, offsetof(struct_type, field), sub_meta, NULL, NULL, 0, 0 }
#define FIELD_STRING_ARRAY(struct_type, field, size) { #field, TYPE_STRING_ARRAY, offsetof(struct_type, field), NULL, NULL, NULL, size, 0 }
#define FIELD_FLOAT_ARRAY(struct_type, field, size) { #field, TYPE_FLOAT_ARRAY, offsetof(struct_type, field), NULL, NULL, NULL, size, 0 }
#define FIELD_INT_ARRAY(struct_type, field, size) { #field, TYPE_INT_ARRAY, offsetof(struct_type, field), NULL, NULL, NULL, size, 0 }
#define FIELD_INT_PTR(struct_type, field) { #field, TYPE_INT_PTR, offsetof(struct_type, field), NULL, NULL, NULL, 0, 0 }
#define FIELD_FLOAT_PTR(struct_type, field) { #field, TYPE_FLOAT_PTR, offsetof(struct_type, field), NULL, NULL, NULL, 0, 0 }
#define FIELD_DOUBLE_PTR(struct_type, field) { #field, TYPE_DOUBLE_PTR, offsetof(struct_type, field), NULL, NULL, NULL, 0, 0 }
#define FIELD_UCHAR_PTR(struct_type, field) { #field, TYPE_UCHAR_PTR, offsetof(struct_type, field), NULL, NULL, NULL, 0, 0 }
#define FIELD_USHORT_PTR(struct_type, field) { #field, TYPE_USHORT_PTR, offsetof(struct_type, field), NULL, NULL, NULL, 0, 0 }
#define FIELD_UINT_PTR(struct_type, field) { #field, TYPE_UINT_PTR, offsetof(struct_type, field), NULL, NULL, NULL, 0, 0 }
#define FIELD_STRING(struct_type, field) { #field, TYPE_STRING, offsetof(struct_type, field), NULL, NULL, NULL, 0, 0 }
#define FIELD_INT(struct_type, field) { #field, TYPE_INT, offsetof(struct_type, field), NULL, NULL, NULL, 0, 0 }
#define FIELD_FLOAT(struct_type, field) { #field, TYPE_FLOAT, offsetof(struct_type, field), NULL, NULL, NULL, 0, 0 }
#define FIELD_BOOL(struct_type, field) { #field, TYPE_BOOL, offsetof(struct_type, field), NULL, NULL, NULL, 0, 0 }
#define FIELD_DOUBLE(struct_type, field) { #field, TYPE_DOUBLE, offsetof(struct_type, field), NULL, NULL, NULL, 0, 0 }
#define FIELD_CHAR(struct_type, field) { #field, TYPE_CHAR, offsetof(struct_type, field), NULL, NULL, NULL, 0, 0 }
#define FIELD_FUNC_PTR(struct_type, field) { #field, TYPE_FUNC_PTR, offsetof(struct_type, field), NULL, NULL, NULL, 0, 0 }
#define FIELD_ENUM(struct_type, field, enum_strings) { #field, TYPE_ENUM, offsetof(struct_type, field), NULL, enum_strings, NULL, 0, 0 }
#define FIELD_VOID_PTR(struct_type, field) { #field, TYPE_VOID_PTR, offsetof(struct_type, field), NULL, NULL, NULL, 0, 0 }
#define FIELD_STRUCT(struct_type, field, sub_meta) { #field, TYPE_STRUCT, offsetof(struct_type, field), sub_meta, NULL, NULL, 0, 0 }
#define FIELD_UINT(struct_type, field) { #field, TYPE_UINT, offsetof(struct_type, field), NULL, NULL, NULL, 0, 0 }
#define FIELD_UCHAR(struct_type, field) { #field, TYPE_UCHAR, offsetof(struct_type, field), NULL, NULL, NULL, 0, 0 }
#define FIELD_LINKED_LIST(struct_type, field, sub_meta, next_field, prev_field) { #field, TYPE_LINKED_LIST, offsetof(struct_type, field), NULL, NULL, sub_meta, offsetof(struct_type, next_field), offsetof(struct_type, prev_field) }

static FieldMeta Vector2Meta[] = {
    FIELD_FLOAT(Vector2, x),
    FIELD_FLOAT(Vector2, y),
    { NULL, 0, 0, NULL, NULL, NULL, 0, 0 }
};

static FieldMeta Vector3Meta[] = {
    FIELD_FLOAT(Vector3, x),
    FIELD_FLOAT(Vector3, y),
    FIELD_FLOAT(Vector3, z),
    { NULL, 0, 0, NULL, NULL, NULL, 0, 0 }
};

static FieldMeta Vector4Meta[] = {
    FIELD_FLOAT(Vector4, x),
    FIELD_FLOAT(Vector4, y),
    FIELD_FLOAT(Vector4, z),
    FIELD_FLOAT(Vector4, w),
    { NULL, 0, 0, NULL, NULL, NULL, 0, 0 }
};

static FieldMeta TagMeta[] = {
    FIELD_STRING(t_Tag, tag),
    FIELD_UINT(t_Tag, tag_id),
    { NULL, 0, 0, NULL, NULL, NULL, 0, 0 }
};

static FieldMeta MatrixMeta[] = {
    FIELD_FLOAT(Matrix, m0),
    FIELD_FLOAT(Matrix, m4),
    FIELD_FLOAT(Matrix, m8),
    FIELD_FLOAT(Matrix, m12),

    FIELD_FLOAT(Matrix, m1),
    FIELD_FLOAT(Matrix, m5),
    FIELD_FLOAT(Matrix, m9),
    FIELD_FLOAT(Matrix, m13),

    FIELD_FLOAT(Matrix, m2),
    FIELD_FLOAT(Matrix, m6),
    FIELD_FLOAT(Matrix, m10),
    FIELD_FLOAT(Matrix, m14),

    FIELD_FLOAT(Matrix, m3),
    FIELD_FLOAT(Matrix, m7),
    FIELD_FLOAT(Matrix, m11),
    FIELD_FLOAT(Matrix, m15),

    { NULL, 0, 0, NULL, NULL, NULL, 0, 0 }
};

static FieldMeta BoneInfoMeta[] = {
    FIELD_STRING_ARRAY(BoneInfo, name, 32),  // Bone name (fixed-size string array)
    FIELD_INT(BoneInfo, parent),            // Parent bone index
    { NULL, 0, 0, NULL, NULL, NULL, 0, 0 }
};


static FieldMeta MeshMeta[] = {
    FIELD_INT(Mesh, vertexCount),
    FIELD_INT(Mesh, triangleCount),

    // Vertex attributes
    FIELD_FLOAT_PTR(Mesh, vertices),
    FIELD_FLOAT_PTR(Mesh, texcoords),
    FIELD_FLOAT_PTR(Mesh, texcoords2),
    FIELD_FLOAT_PTR(Mesh, normals),
    FIELD_FLOAT_PTR(Mesh, tangents),
    FIELD_UCHAR_PTR(Mesh, colors),
    FIELD_USHORT_PTR(Mesh, indices),

    // Animation vertex data
    FIELD_FLOAT_PTR(Mesh, animVertices),
    FIELD_FLOAT_PTR(Mesh, animNormals),
    FIELD_UCHAR_PTR(Mesh, boneIds),
    FIELD_FLOAT_PTR(Mesh, boneWeights),
    FIELD_STRUCT_PTR(Mesh, boneMatrices, MatrixMeta),
    FIELD_INT(Mesh, boneCount),

    // OpenGL identifiers
    FIELD_UINT(Mesh, vaoId),
    FIELD_UINT_PTR(Mesh, vboId),

    { NULL, 0, 0, NULL, NULL, NULL, 0, 0 }
};

static FieldMeta QuaternionMeta[] = {
    FIELD_FLOAT(Quaternion, x),
    FIELD_FLOAT(Quaternion, y),
    FIELD_FLOAT(Quaternion, z),
    FIELD_FLOAT(Quaternion, w),
    { NULL, 0, 0, NULL, NULL, NULL, 0, 0 }
};

static FieldMeta Texture2DMeta[] = {
    FIELD_UINT(Texture2D, id),
    FIELD_INT(Texture2D, width),
    FIELD_INT(Texture2D, height),
    FIELD_INT(Texture2D, mipmaps),
    FIELD_INT(Texture2D, format),
    { NULL, 0, 0, NULL, NULL, NULL, 0, 0 }
};

static FieldMeta ShaderMeta[] = {
    FIELD_UINT(Shader, id),
    FIELD_INT_PTR(Shader, locs),
    { NULL, 0, 0, NULL, NULL, NULL, 0, 0 }
};

static FieldMeta ColorMeta[] = {
    FIELD_INT(Color, r),
    FIELD_INT(Color, g),
    FIELD_INT(Color, b),
    FIELD_INT(Color, a),
    { NULL, 0, 0, NULL, NULL, NULL, 0, 0 }
};

static FieldMeta MaterialMapMeta[] = {
    FIELD_STRUCT(MaterialMap, texture, Texture2DMeta),
    FIELD_STRUCT(MaterialMap, color, ColorMeta),
    FIELD_FLOAT(MaterialMap, value),
    { NULL, 0, 0, NULL, NULL, NULL, 0, 0 }
};

static FieldMeta MaterialMeta[] = {
    FIELD_STRUCT(Material, shader, ShaderMeta),
    FIELD_STRUCT_PTR(Material, maps, MaterialMapMeta),
    FIELD_FLOAT_ARRAY(Material, params, 4),
    { NULL, 0, 0, NULL, NULL, NULL, 0, 0 }
};

static FieldMeta TransformMeta[] = {
    FIELD_STRUCT(Transform, translation, Vector3Meta),
    FIELD_STRUCT(Transform, rotation, QuaternionMeta),
    FIELD_STRUCT(Transform, scale, Vector3Meta),
    { NULL, 0, 0, NULL, NULL, NULL, 0, 0 }
};

static FieldMeta ModelMeta[] = {
    FIELD_STRUCT(Model, transform, MatrixMeta),
    
    FIELD_INT(Model, meshCount),
    FIELD_INT(Model, materialCount),
    FIELD_STRUCT_PTR(Model, meshes, MeshMeta),
    FIELD_STRUCT_PTR(Model, materials, MaterialMeta),
    FIELD_INT_PTR(Model, meshMaterial),

    // Animation data
    FIELD_INT(Model, boneCount),
    FIELD_STRUCT_PTR(Model, bones, BoneInfoMeta),
    FIELD_STRUCT_PTR(Model, bindPose, TransformMeta),
    { NULL, 0, 0, NULL, NULL, NULL, 0, 0 }
};

static FieldMeta RectangleMeta[] = {
    FIELD_FLOAT(Rectangle, x),
    FIELD_FLOAT(Rectangle, y),
    FIELD_FLOAT(Rectangle, width),
    FIELD_FLOAT(Rectangle, height),
    { NULL, 0, 0, NULL, NULL, NULL, 0, 0 }
};

static const char* PriorityRankStrings[] = {
    "NONE",
    "LOW",
    "MEDIUM",
    "HIGH",    
    "CRITICAL",
    "PLAYER",
    NULL
};

static FieldMeta EntityMeta[] = {
    FIELD_STRING(t_Entity, entity_name),
    FIELD_UINT(t_Entity, num_tags),
    FIELD_UINT(t_Entity, layer),

    FIELD_VOID_PTR(t_Entity, entity_data),

    FIELD_BOOL(t_Entity, is_active),
    FIELD_FLOAT(t_Entity, health),
    FIELD_FLOAT(t_Entity, max_life_time),
    FIELD_FLOAT(t_Entity, current_life_time),

    FIELD_ENUM(t_Entity, priority_rank, PriorityRankStrings),

    FIELD_STRUCT(t_Entity, entity3D.position, Vector3Meta),
    FIELD_STRUCT(t_Entity, entity3D.velocity, Vector3Meta),
    FIELD_STRUCT(t_Entity, entity3D.rotation, Vector3Meta),
    FIELD_STRUCT(t_Entity, entity3D.scale, Vector3Meta),
    FIELD_INT(t_Entity, entity3D.model_id),

    { NULL, 0, 0, NULL, NULL, NULL, 0, 0 }
};

#endif // BH_METATABLES_H