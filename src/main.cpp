#include "PathTracer.h"
#include <random>
/*
    TODO:
    -Glass material (refractive)
    -Focus blur
    -BVH
*/
Scene createScene() {
    Scene scene;
    constexpr uint32_t NUM_SPHERES = 1024;
    scene.spheres.reserve(NUM_SPHERES);
    scene.cameraPos = {0.f,0.f,-100.f};
    // scene.spheres.push_back({
    //     .center = Vector{ -0.5f, 0, 1 },
    //     .radius = 0.2f,
    //     .material = {
    //         .color = { 1, 0.1f, 0.1f },
    //         .illumination = 0.f,
    //         .metalness = 1.f,
    //     },
    // });
    // scene.spheres.push_back({
    //     .center = Vector{ 0, 0, 1 },
    //     .radius = 0.2f,
    //     .material = {
    //         .color = { 0.1f, 1, 0.1f },
    //         .illumination = 0.f,
    //         .metalness = 0.f,
    //     },
    // });
    // scene.spheres.push_back({
    //     .center = Vector{ 0.5f, 0, 1 },
    //     .radius = 0.2f,
    //     .material = {
    //         .color = { 0.1f, 0.1f, 1.f },
    //         .illumination = 0.f,
    //         .metalness = 0.7f,
    //     },
    // });
    // scene.spheres.push_back({
    //     .center = Vector{ 0, -100.2f, 0 },
    //     .radius = 100.f,
    //     .material = {
    //         .color = { 1, 1, 0.1f },
    //         .illumination = 0.f,
    //         .metalness = 0.f,
    //     },
    // });
    // scene.spheres.push_back({
    //     .center = Vector{ 0, 100.f, 100.f },
    //     .radius = 50.f,
    //     .material = {
    //         .color = { 1, 1, 1 },
    //         .illumination = 1.f,
    //         .metalness = 0.f,
    //     },
    // });
    static std::random_device device;
    static std::mt19937 gen(device());
    static std::uniform_real_distribution<float> signedHundredDist(-100.f, 100.f);
    static std::uniform_real_distribution<float> unitDist(0.f,1.f);
    static std::uniform_real_distribution<float> oneToTenDist(1.f, 10.f);
    for (uint32_t i = 0; i < NUM_SPHERES; ++i) {
        Vector center = {signedHundredDist(gen),signedHundredDist(gen),signedHundredDist(gen)};
        while(length(center - scene.cameraPos) < 10.f){
            center = {signedHundredDist(gen),signedHundredDist(gen),signedHundredDist(gen)};
        }
        const Material material = {
            .color = {unitDist(gen),unitDist(gen),unitDist(gen)},
            .illumination = 0.f,
            .metalness = unitDist(gen)
        };
        const float radius = oneToTenDist(gen);
        const Sphere sphere = {
            .center = center,
            .radius = radius,
            .material = material
        };
        scene.spheres.push_back(sphere);
    }
    return scene;
}
int main() {
    constexpr int width = 720;
    constexpr int height = 720;

    PathTracer pathTracer("Path Tracer", width, height);
    Scene scene = createScene();
    pathTracer.setScene(scene);

    pathTracer.setConfig({ .sampleCount = 1024,
                           .bounceCount = 20 });

    pathTracer.pathTrace();

    while (pathTracer.isRunning()) {
        pathTracer.update();
    }
}