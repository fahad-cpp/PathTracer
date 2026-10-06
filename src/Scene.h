#ifndef SCENE_H
#define SCENE_H
#include <FSWindow.h>
#include <vector>
#include "Utils.h"
struct Material {
    Colourf color;
    float illumination;
    float metalness;
};

struct Sphere {
    Vector center;
    float radius;
    Material material;
};

struct Scene {
    std::vector<Sphere> spheres;
};

struct IntersectionData {
    Vector point;
    Vector normal;
    Material material;
    float intersection;
};
#endif