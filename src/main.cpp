#include "PathTracer.h"

/*
    TODO:
    -Glass material (refractive)
    -Focus blur
    -BVH
*/
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
            .metalness = 0.7f,
        },
    });
    scene.spheres.push_back({
        .center = Vector{ 0, -100.2f, 0 },
        .radius = 100.f,
        .material = {
            .color = { 1, 1, 0.1f },
            .illumination = 0.f,
            .metalness = 0.f,
        },
    });
    scene.spheres.push_back({
        .center = Vector{ 0, 100.f, -100.f },
        .radius = 50.f,
        .material = {
            .color = { 1, 1, 1 },
            .illumination = 1.f,
            .metalness = 0.f,
        },
    });
    return scene;
}
int main() {
    constexpr int width = 1920;
    constexpr int height = 1080;

    PathTracer pathTracer("Path Tracer", width, height);
    Scene scene = createScene();
    pathTracer.setScene(scene);

    pathTracer.setConfig({
        .sampleCount = 1024,
        .bounceCount = 20
    });

    pathTracer.pathTrace();

    while (pathTracer.isRunning()) {
        pathTracer.update();
    }
}