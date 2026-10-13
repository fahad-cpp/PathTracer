#define NOMINMAX
#include "BVH.h"
#include "Utils.h"
#include <assert.h>

#define LAST_NODE_COUNT 1
constexpr float epsilon = std::numeric_limits<float>::epsilon();
void BVH::build(Scene &scene) {
    if (visualize) {
        visualizationAABB.reserve(2 * scene.spheres.size() - 1);
    }
    // build root node
    root = new BVHNode;
    root->spheres.reserve(scene.spheres.size());
    for (const Sphere &sphere : scene.spheres) {
        root->spheres.push_back(sphere);
    }
    updateAABB(root);
    subdivide(root);
}
IntersectionData BVH::lookup(const Vector origin, const Vector direction, float min, float max, BVHNode *node) {
    static const IntersectionData noIntersection = {
        .point = {},
        .normal = {},
        .material = {},
        .intersection = FLT_MAX
    };

    if (!RayIntersectAABB(origin, direction, node->aabb)) {
        return noIntersection;
    }
    // leaf node
    if (node->left == nullptr && node->right == nullptr) {
        IntersectionData intersection = noIntersection;
        for (const Sphere &sphere : node->spheres) {
            IntersectionData raySphere = RayIntersectSphere(origin, direction, sphere);
            if (
                raySphere.intersection < intersection.intersection &&
                (raySphere.intersection >= min) &&
                (raySphere.intersection <= max)) {
                intersection = raySphere;
            }
        }
        return intersection;
    }
    IntersectionData left = noIntersection;
    IntersectionData right = noIntersection;
    left = lookup(origin, direction, min, max, node->left);
    right = lookup(origin, direction, min, max, node->right);

    return (left.intersection < right.intersection) ? left : right;
}
void BVH::updateAABB(BVHNode *node) {
    Vector min = { FLT_MAX, FLT_MAX, FLT_MAX };
    Vector max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
    for (const Sphere &sphere : node->spheres) {
        Vector sphereMin = sphere.center - sphere.radius;
        Vector sphereMax = sphere.center + sphere.radius;
        if (sphereMin.x < min.x) {
            min.x = sphereMin.x;
        }
        if (sphereMin.y < min.y) {
            min.y = sphereMin.y;
        }
        if (sphereMin.z < min.z) {
            min.z = sphereMin.z;
        }

        if (sphereMax.x > max.x) {
            max.x = sphereMax.x;
        }
        if (sphereMax.y > max.y) {
            max.y = sphereMax.y;
        }
        if (sphereMax.z > max.z) {
            max.z = sphereMax.z;
        }
    }
    if (visualize) {
        visualizationAABB.push_back(node->aabb);
    }
    node->aabb = { min, max };
}

void BVH::subdivide(BVHNode *node) {
    assert(LAST_NODE_COUNT > 0);
    if (node->spheres.size() <= LAST_NODE_COUNT) {
        return;
    }
    Vector dist = node->aabb.max - node->aabb.min;
    std::vector<Sphere> leftSpheres = {};
    std::vector<Sphere> rightSpheres = {};

    if ((dist.x > dist.y) && (dist.x > dist.z)) {
        // x is the longest axis
        float middle = node->aabb.min.x + (dist.x / 2.f);
        for (const Sphere &sphere : node->spheres) {
            if (sphere.center.x < middle) {
                leftSpheres.push_back(sphere);
            } else {
                rightSpheres.push_back(sphere);
            }
        }
    } else if (dist.y > dist.z) {
        // y is the longest axis
        float middle = node->aabb.min.y + (dist.y / 2.f);
        for (const Sphere &sphere : node->spheres) {
            if (sphere.center.y < middle) {
                leftSpheres.push_back(sphere);
            } else {
                rightSpheres.push_back(sphere);
            }
        }
    } else {
        // z is the longest axis
        float middle = node->aabb.min.z + (dist.z / 2.f);
        for (const Sphere &sphere : node->spheres) {
            if (sphere.center.z < middle) {
                leftSpheres.push_back(sphere);
            } else {
                rightSpheres.push_back(sphere);
            }
        }
    }
    // nothing changed , dont subdivide further
    if (
        leftSpheres.size() == node->spheres.size() ||
        rightSpheres.size() == node->spheres.size()) {
        //return;
    }

    if (leftSpheres.size()) {
        BVHNode *left = new BVHNode;
        left->left = nullptr;
        left->right = nullptr;
        left->spheres = leftSpheres;
        updateAABB(left);
        node->left = left;
        subdivide(left);
    }
    if (rightSpheres.size()) {
        BVHNode *right = new BVHNode;
        right->left = nullptr;
        right->right = nullptr;
        right->spheres = rightSpheres;
        updateAABB(right);
        node->right = right;
        subdivide(right);
    }
}
void BVH::cleanup() {
    cleanupNode(root);
}
void BVH::cleanupNode(BVHNode *node) {
    if (node->left != nullptr) {
        cleanupNode(node->left);
        node->left = nullptr;
    }
    if (node->right != nullptr) {
        cleanupNode(node->right);
        node->right = nullptr;
    }
    delete node;
}

bool RayIntersectAABB(const Vector O, const Vector D, const AABB box) {
    const Vector ND = D / length(D);
    const float origin[3] = { O.x, O.y, O.z };
    const float direction[3] = { ND.x, ND.y, ND.z };
    const float bbMin[3] = { box.min.x, box.min.y, box.min.z };
    const float bbMax[3] = { box.max.x, box.max.y, box.max.z };

    float enterDist = 0.f;
    float exitDist = FLT_MAX;
    for (uint32_t i = 0; i < 3; ++i) {
        if (abs(direction[i]) <= epsilon) {
            if (origin[i] < bbMin[i] || origin[i] > bbMax[i]) {
                return false;
            } else {
                // Ray is inside the slab so skip this axis
                continue;
            }
        }
        float invDir = 1.f / direction[i];
        float nearT = (bbMin[i] - origin[i]) * invDir;
        float farT = (bbMax[i] - origin[i]) * invDir;
        if (nearT > farT) {
            std::swap(nearT, farT);
        }
        enterDist = std::max(enterDist, nearT);
        exitDist = std::min(exitDist, farT);

        if (enterDist > exitDist) {
            return false;
        }
    }
    return true;
}

IntersectionData RayIntersectSphere(const Vector origin, const Vector direction, const Sphere sphere) {
    // Ray Sphere intersection
    Vector CO = origin - sphere.center;
    float a = dot(direction, direction);
    float b = dot(CO, direction) * 2;
    float c = dot(CO, CO) - (sphere.radius * sphere.radius);

    float discriminant = (b * b) - (4 * a * c);
    float t = FLT_MAX;

    if (discriminant >= 0) {
        t = (-b - sqrt(discriminant)) / (2 * a);
    }
    Vector hitPoint = origin + (t * direction);
    IntersectionData intersection = {
        .point = hitPoint,
        .normal = normalize(hitPoint - sphere.center),
        .material = sphere.material,
        .intersection = t
    };
    return intersection;
}