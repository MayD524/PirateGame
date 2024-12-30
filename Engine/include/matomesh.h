#ifndef MATOMESH_H
#define MATOMESH_H

#include "raylib.h"
#include <stdbool.h>

// ------------------------
// Collision Types Enum
// ------------------------
typedef enum {
    COLLISION_NONE,
    COLLISION_AABB,
    COLLISION_SPHERE,
    COLLISION_MATO_MESH
} CollisionType;

// ------------------------
// Triangle Structure
// ------------------------
typedef struct {
    Vector3 p1;
    Vector3 p2;
    Vector3 p3;
} Triangle;

// Forward declaration of BVHNode
typedef struct BVHNode BVHNode;

// ------------------------
// BVH Node Structure
// ------------------------
struct BVHNode {
    BoundingBox box;
    BVHNode* left;
    BVHNode* right;
    Triangle* triangles;
    int triangleCount;
};

// ------------------------
// MatoMesh Structure
// ------------------------
typedef struct {
    Triangle* triangles;
    int triangleCount;
    BVHNode* bvhRoot;
} MatoMesh;

// ------------------------
// Function Declarations
// ------------------------

// MatoMesh Management
MatoMesh ExtractMatoMesh(Mesh raylibMesh);
void DestroyMatoMesh(MatoMesh* matoMesh);

// BVH Construction
BVHNode* CreateBVHNode(Triangle* triangles, int triangleCount);
void DestroyBVH(BVHNode* node);

// Collision Detection
bool CheckBoundingBoxIntersection(BoundingBox a, BoundingBox b);
bool CheckTriangleIntersection(Triangle t1, Triangle t2);
bool TraverseBVH(BVHNode* a, BVHNode* b);

// Helper Functions
BoundingBox GetMeshAABB(MatoMesh* matoMesh, Vector3 position);

// Initialization Functions

#endif // MATOMESH_H
