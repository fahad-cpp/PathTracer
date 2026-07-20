#include "Colour.h"
#include "Input.h"
#include "Vector.h"
#include "Vector2.h"
#include <FSWindow.h>
#include <atomic>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

/*
    TODO:
    -Glass material (refractive)
    -Focus blur
    -BVH
*/

using Vector = FS::Vector;
using Vector2 = FS::Vector2;
using Colour = FS::Colour;
using Colourf = FS::Colourf;

// Day sky
Colourf skycolor1 = { 1, 1, 1 };
Colourf skycolor2 = { 0.5f, 0.7f, 1 };
// Night sky
//  Colourf skycolor1 = { 0, 0, 0 };
//  Colourf skycolor2 = { 0, 0, 0 };

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

// Conversion Utils
Vector canvasToViewport(float x, float y, FS::RenderState &renderState) {
    constexpr float d = 1.f;
    const Vector viewport{ renderState.width / float(renderState.height), 1 };
    return { x * (viewport.x / float(renderState.width)), y * (viewport.y / float(renderState.height)), d };
}
Colourf linearToGamma(const Colourf &color) {
    Colourf gammacolor;
    if (color.R > 0) {
        gammacolor.R = std::sqrt(color.R);
    }
    if (color.G > 0) {
        gammacolor.G = std::sqrt(color.G);
    }
    if (color.B > 0) {
        gammacolor.B = std::sqrt(color.B);
    }
    return gammacolor;
}
Vector normalize(const Vector &vec) {
    return vec / length(vec);
}
// Random
float cubeRandom() {
    // get random number between [0.f,1.f]
    static std::random_device device;
    thread_local std::mt19937 gen(device());
    static std::uniform_real_distribution<float> dist(-1.f, 1.f);
    return dist(gen);
}
Vector unitRandom() {
    Vector randvec;
    // TODO:Very inefficient , fix later
    while (true) {
        randvec = { cubeRandom(), cubeRandom(), cubeRandom() };
        if (length(randvec) <= 1.f) {
            break;
        }
    }
    return normalize(randvec);
}
Vector hemisphereRandom(const Vector &normal) {
    Vector randvec = unitRandom();
    if (dot(randvec, normal) > 0.f) {
        return randvec;
    } else {
        return -randvec;
    }
}
// Math helpers
Vector reflectRay(const Vector &R, const Vector &N) {
    return (2 * (N * dot(R, N)) - R);
}
template <typename T>
T lerp(const T &a, const T &b, float t) {
    return (a + (t * (b - a)));
}
// Core raytracing
IntersectionData closestIntersection(const Vector &origin, const Vector &direction, float min, float max, const Scene &scene) {
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

        if ((t < minT) && (t > min) && (t < max)) {
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

Colourf traceRay(const Vector &origin, const Vector &direction, const int bounceCount, const Scene &scene) {
    IntersectionData intersectData = closestIntersection(origin, direction, 0.001f, INT_MAX, scene);
    if (intersectData.intersection == INT_MAX) {
        return lerp(skycolor2, skycolor1, (direction.y + 0.5f));
    }
    Colourf color = intersectData.material.color;
    if (bounceCount <= 0) {
        return { 0, 0, 0 };
    }
    Vector newDirection;
    if (intersectData.material.metalness <= 0.f) {
        newDirection = normalize(intersectData.normal + unitRandom());
    } else {
        Vector reflected = normalize(reflectRay(normalize(-direction), intersectData.normal));
        newDirection = reflected + (unitRandom() * (1 - intersectData.material.metalness));
    }
    float illum = intersectData.material.illumination;
    return (traceRay(intersectData.point, newDirection, bounceCount - 1, scene) + illum) * color;
}
void pathTraceTile(const Vector2 offset, const Vector2 tileSize, FS::Window &window, const Scene &scene, std::atomic_bool &finished, const int SAMPLE_COUNT, const int BOUNCE) {
    FS::RenderState &renderState = window.getRenderState();
    const Vector origin = { 0, 0, -0.5f };
    for (int y = offset.y; y < (offset.y + tileSize.y); y++) {
        for (int x = offset.x; x < (offset.x + tileSize.x); x++) {
            FS::Colourf colourf;
            for (int i = 0; i < SAMPLE_COUNT; i++) {
                float samplex = x + (cubeRandom() - 0.5f);
                float sampley = y + (cubeRandom() - 0.5f);
                Vector direction = canvasToViewport(samplex - (renderState.width / 2.f), sampley - (renderState.height / 2.f), renderState);
                Colourf color = traceRay(origin, direction, BOUNCE, scene);
                colourf.R += (color.R);
                colourf.G += (color.G);
                colourf.B += (color.B);
            }
            colourf = colourf / SAMPLE_COUNT;
            uint32_t index = x + (y * renderState.width);
            ((uint32_t *)renderState.screenBuffer)[index] = FS::rgbtoHex(linearToGamma(colourf));
        }
    }
    finished = true;
}
void pathTrace(const Scene &scene, FS::Window &window) {
    constexpr int SAMPLE_COUNT = 1024;
    constexpr int BOUNCE = 20;

    FS::RenderState renderState = window.getRenderState();
    const uint32_t threadCount = std::thread::hardware_concurrency();
    std::vector<std::atomic_bool> finished(threadCount);
    uint32_t remainingLines = renderState.height % threadCount;
    uint32_t linesPerThread = renderState.height / threadCount;
    std::vector<std::thread> threads(threadCount);
    uint32_t start = 0;
    for (uint32_t i = 0; i < threadCount; i++) {
        int tileY = linesPerThread + ((i < remainingLines) ? 1 : 0);
        Vector2 tileSize = { float(renderState.width), float(tileY) };
        Vector2 tileOffset = { float(0), float(start) };
        threads[i] = std::thread(pathTraceTile, tileOffset, tileSize, std::ref(window), std::cref(scene), std::ref(finished[i]), SAMPLE_COUNT, BOUNCE);
        start += tileY;
    }
    for (uint32_t i = 0; i < threadCount; i++) {
        while (!finished[i]) {
            window.swapBuffers();
            window.processMessages();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        threads[i].join();
    }
    std::cout << "Rendered.\n";
}
// Export Given buffer to PPM P3 File
void exportToPPM(uint32_t *buffer, uint32_t width, uint32_t height, const std::string &file) {
    if (!buffer) {
        std::cerr << "Invalid buffer :exportToPPM()\n";
        return;
    }
    std::ofstream ofs(file);
    if (!ofs.is_open()) {
        std::cerr << "Failed to open file " << file << " :exportToPPM()\n";
        return;
    }
    ofs << "P3\n";
    ofs << width << ' ' << height << "\n";
    ofs << "255\n";
    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            uint32_t index = x + (y * width);
            Colour color = FS::hexToRGB(buffer[index]);
            ofs << int(color.R) << ' ' << int(color.G) << ' ' << int(color.B) << '\n';
        }
    }
    ofs.close();
}
void clearScreen(uint32_t color, FS::RenderState &renderState) {
    for (int y = 0; y < renderState.height; y++) {
        for (int x = 0; x < renderState.width; x++) {
            uint32_t index = x + (y * renderState.width);
            ((uint32_t *)renderState.screenBuffer)[index] = color;
        }
    }
}
Scene createScene() {
    Scene scene;
    scene.spheres.reserve(32);
    scene.spheres.push_back({
        .center = Vector{ -0.5f, 0, 1 },
        .radius = 0.2f,
        .material = {
            .color = { 1, 0.1f, 0.1f },
            .illumination = 0.f,
            .metalness = 1.f,
        },
    });
    scene.spheres.push_back({
        .center = Vector{ 0, 0, 1 },
        .radius = 0.2f,
        .material = {
            .color = { 0.1f, 1, 0.1f },
            .illumination = 0.f,
            .metalness = 0.f,
        },
    });
    scene.spheres.push_back({
        .center = Vector{ 0.5f, 0, 1 },
        .radius = 0.2f,
        .material = {
            .color = { 0.1f, 0.1f, 1.f },
            .illumination = 0.f,
            .metalness = 0.5f,
        },
    });
    scene.spheres.push_back({
        .center = Vector{ 0, 100.2f, 0 },
        .radius = 100.f,
        .material = {
            .color = { 1, 1, 0.1f },
            .illumination = 0.f,
            .metalness = 0.f,
        },
    });
    return scene;
}
void handleInput(FS::Input &input, Scene &scene, FS::RenderState &renderState, FS::Window &window) {
    if (isDown(FS::Buttons::BUTTON_R)) {
        clearScreen(0x00000000, renderState);
        pathTrace(scene, window);
    }
    if (isDown(FS::Buttons::BUTTON_ESC)) {
        window.close();
    }
    if (isDown(FS::Buttons::BUTTON_P)) {
        exportToPPM((uint32_t *)renderState.screenBuffer, renderState.width, renderState.height, "output.ppm");
    }
}
int main() {
    constexpr int width = 720;
    constexpr int height = 720;

    FS::Window window("Path Tracer", width, height);
    FS::RenderState renderState = window.getRenderState();
    Scene scene = createScene();

    pathTrace(scene, window);

    while (window.isOpen()) {
        FS::Input input = window.getInput();
        handleInput(input, scene, renderState, window);
        window.swapBuffers();
        window.processMessages();
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
}