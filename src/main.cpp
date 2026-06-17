#include "Colour.h"
#include "Vector.h"
#include "Vector2.h"
#include <FSWindow.h>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <vector>

using Vector = FS::Vector;
using Vector2 = FS::Vector2;

struct Material {
    FS::Colour color;
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
void pathTrace(const Scene &scene, FS::RenderState &renderState) {
    constexpr float d = 1.f;
    const Vector origin = { 0, 0, 0 };
    const Vector2 viewportSize = { 1.f, 1.f };

    for (int y = 0; y < renderState.height; y++) {
        for (int x = 0; x < renderState.width; x++) {
            Vector direction = { (x - renderState.width / 2.f) * viewportSize.x / float(renderState.width), (y - renderState.height / 2.f) * viewportSize.y / float(renderState.height), d };
            IntersectionData intersectData = closestIntersection(origin, direction, scene);
            if (intersectData.intersection != INT_MAX) {
                uint32_t index = x + (y * renderState.width);
                float light = rayTracedLight(intersectData.point, intersectData.normal, scene.lights);
                ((uint32_t *)renderState.screenBuffer)[index] = FS::rgbtoHex(intersectData.material.color * light);
            }
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
    scene.spheres.emplace_back(Vector{ 0, 0, 5 }, 0.2f, Material{ { 0, 255, 0 } });

    scene.lights.reserve(32);
    scene.lights.emplace_back(Vector{ -3, 4, 0 }, Vector{ 0, 0, 0 }, 0.6f, LightType::LT_POINT);
    scene.lights.emplace_back(Vector{ 0, 0, 0 }, Vector{ 0, 0, 0 }, 0.3f, LightType::LT_AMBIENT);
    scene.lights.emplace_back(Vector{ 0, 0, 0 }, Vector{ -2, 0, 0 }, 0.6f, LightType::LT_DIRECTION);

    while (window.isOpen()) {
        clearScreen(0x00000000, renderState);
        pathTrace(scene, renderState);
        window.processMessages();
        window.swapBuffers();
    }
}