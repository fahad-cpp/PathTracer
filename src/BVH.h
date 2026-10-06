#ifndef BVH_H
#define BVH_H
#include "Scene.h"
struct AABB {
    Vector min, max;
};
struct BVHNode {
    BVHNode *left = nullptr;
    BVHNode *right = nullptr;
    AABB aabb;
    std::vector<Sphere> spheres;
};
class BVH {
  public:
    BVHNode *root = nullptr;
    void build(Scene &scene);
    IntersectionData lookup(const Vector origin, const Vector direction, float min, float max, BVHNode *node);
    void cleanup();

  private:
    void updateAABB(BVHNode *node);
    void subdivide(BVHNode *node);
    void cleanupNode(BVHNode *node);
    void drawBB();
};

bool RayIntersectAABB(const Vector origin, const Vector direction, const AABB box);
IntersectionData RayIntersectSphere(const Vector origin, const Vector direction, const Sphere sphere);
#endif