#include "Colour.h"
#include "Vector.h"
#include "Vector2.h"
#include <FSWindow.h>
#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

/*
    TODO:
    -Anti aliasing by offsetting rays
    -diffuse lighting samples
    -Metal material (reflective)
    -Glass material (refractive)
    -Focus blur
*/
using Vector = FS::Vector;
using Vector2 = FS::Vector2;
using Colour = FS::Colour;
Colour backgroundColor = { 64, 64, 64 };

struct Material {
    Colour color;
    float reflectiveness;
};

struct Sphere {
    Vector center;
    float radius;
    Material material;
};

enum class LightType : uint8_t {
    LT_POINT = 0,
    LT_DIRECTION,
    LT_AMBIENT
};

struct Light {
    Vector pos;
    Vector direction;
    float intensity;
    LightType type;
};

struct Scene {
    std::vector<Sphere> spheres;
    std::vector<Light> lights;
};

struct IntersectionData {
    Vector point;
    Vector normal;
    Material material;
    float intersection;
};

void clearScreen(uint32_t color, FS::RenderState &renderState) {
    for (int y = 0; y < renderState.height; y++) {
        for (int x = 0; x < renderState.width; x++) {
            uint32_t index = x + (y * renderState.width);
            ((uint32_t *)renderState.screenBuffer)[index] = color;
        }
    }
}
Vector normalize(const Vector &vec) {
    return vec / length(vec);
}
// vv You're not supposed to use this for path tracing , its already done when tracing paths
float rayTracedLight(const Vector &point, const Vector &normal, const std::vector<Light> &lights) {
    float intensity = 0.f;
    for (const Light &light : lights) {
        Vector L = {};
        if (light.type == LightType::LT_AMBIENT) {
            intensity += light.intensity;
        } else {
            if (light.type == LightType::LT_DIRECTION) {
                L = normalize(-light.direction);
            } else if (light.type == LightType::LT_POINT) {
                L = normalize(light.pos - point);
            }
            float diffuse = std::max(0.f, dot(normal, L));
            intensity += (diffuse * light.intensity);
        }
    }

    return std::clamp(intensity, 0.f, 1.f);
}
IntersectionData closestIntersection(const Vector &origin, const Vector &direction, const Scene &scene) {
    float minT = INT_MAX;
    Material hitMaterial = {};
    Vector hitNormal = {};
    Vector hitPoint = {};
    for (const Sphere &sphere : scene.spheres) {
        // Ray Sphere intersection
        Vector CO = origin - sphere.center;
        float a = dot(direction, direction);
        float b = dot(CO, direction) * 2;
        float c = dot(CO, CO) - (sphere.radius * sphere.radius);

        float discriminant = (b * b) - (4 * a * c);
        float t = INT_MAX;

        if (discriminant >= 0) {
            t = (-b - sqrt(discriminant)) / (2 * a);
        }

        if ((t > 0) && (t < minT)) {
            minT = t;
            hitPoint = origin + (t * direction);
            hitMaterial = sphere.material;
            hitNormal = normalize(hitPoint - sphere.center);
        }
    }

    IntersectionData intData = {
        .point = hitPoint,
        .normal = hitNormal,
        .material = hitMaterial,
        .intersection = minT
    };

    return intData;
}
Vector reflectRay(const Vector &R, const Vector &N) {
    return (2 * (N * dot(R, N)) - R);
}
// get random number between [0.f,1.f]
float unitRandom() {
    float randnum = (rand() / (RAND_MAX + 1.f));
    return randnum;
}
Vector hemisphereRandom(const Vector &normal) {
    Vector randvec = {};
    // Very inefficient , fix later
    while (true) {
        randvec = { unitRandom(), unitRandom(), unitRandom() };
        if (length(randvec) < 1) {
            break;
        }
    }
    normalize(randvec);
    if (dot(randvec, normal) > 0.f) {
        return randvec;
    } else {
        return -randvec;
    }
}
Colour traceRay(const Vector &origin, const Vector &direction, const int bounceCount, const Scene &scene) {
    IntersectionData intersectData = closestIntersection(origin, direction, scene);
    float reflectiveness = intersectData.material.reflectiveness;
    if (intersectData.intersection == INT_MAX) {
        return backgroundColor;
    }
    Colour color = intersectData.material.color;
    if (bounceCount == 0 || reflectiveness <= 0.f) {
        return color;
    }
    Vector newDirection = hemisphereRandom(intersectData.normal); // reflectRay(-direction, intersectData.normal);
    return traceRay(intersectData.point, newDirection, bounceCount - 1, scene) * 0.5f;
}
Vector canvasToViewport(float x, float y, FS::RenderState &renderState) {
    constexpr float d = 1.f;
    const Vector viewport{ renderState.width / float(renderState.height), 1 };
    return { x * (viewport.x / float(renderState.width)), y * (viewport.y / float(renderState.height)), d };
}
void pathTrace(const Scene &scene, FS::RenderState &renderState) {
    constexpr int SAMPLE_COUNT = 10;
    constexpr int BOUNCE = 50;

    const Vector origin = { 0, 0, 0 };
    for (int y = 0; y < renderState.height; y++) {
        std::clog << "\rScanlines done: " << y + 1 << "/" << renderState.height << std::flush;
        for (int x = 0; x < renderState.width; x++) {
            // TODO: Randomize direction and use multiple samples
            Vector fcolor;
            for (int i = 0; i < SAMPLE_COUNT; i++) {
                float samplex = x + (unitRandom() - 0.5f);
                float sampley = y + (unitRandom() - 0.5f);
                Vector direction = canvasToViewport(samplex - (renderState.width / 2.f), sampley - (renderState.height / 2.f), renderState);
                Colour color = traceRay(origin, direction, BOUNCE, scene);
                fcolor.x += (color.R / 255.f);
                fcolor.y += (color.G / 255.f);
                fcolor.z += (color.B / 255.f);
            }
            fcolor = fcolor / SAMPLE_COUNT;
            Colour pixelColor = {
                uint8_t(std::clamp(fcolor.x * 255.f, 0.f, 255.f)),
                uint8_t(std::clamp(fcolor.y * 255.f, 0.f, 255.f)),
                uint8_t(std::clamp(fcolor.z * 255.f, 0.f, 255.f))
            };
            uint32_t index = x + (y * renderState.width);
            ((uint32_t *)renderState.screenBuffer)[index] = FS::rgbtoHex(pixelColor);
        }
    }
}
int main() {
    constexpr int width = 720;
    constexpr int height = 720;

    FS::Window window("Path Tracer", width, height);
    FS::RenderState &renderState = *(window.getRenderState());

    Scene scene;
    scene.spheres.reserve(32);
    scene.spheres.emplace_back(Vector{ -0.5f, 0, 1 }, 0.2f, Material{ { 255, 0, 0 }, 0.4f });
    scene.spheres.emplace_back(Vector{ 0, 0, 1 }, 0.2f, Material{ { 0, 255, 0 }, 0.3f });
    scene.spheres.emplace_back(Vector{ 0.5f, 0, 1 }, 0.2f, Material{ { 0, 0, 255 }, 0.4f });
    scene.spheres.emplace_back(Vector{ 0, 100.2f, 0 }, 100.f, Material{ { 255, 255, 255 }, 0.4f });

    scene.lights.reserve(32);
    scene.lights.emplace_back(Vector{ 0, 0, 0 }, Vector{ -2, 0, 0 }, 0.6f, LightType::LT_DIRECTION);

    pathTrace(scene, renderState);
    window.swapBuffers();

    while (window.isOpen()) {
        window.processMessages();
    }
}