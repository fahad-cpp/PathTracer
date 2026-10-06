#define NOMINMAX
#include "PathTracer.h"
#include "Timer.h"
#include <fstream>
#include <iostream>
#include <random>
#include <thread>


// Day sky
// Colourf skycolor1 = { 1, 1, 1 };
// Colourf skycolor2 = { 0.5f, 0.7f, 1 };
// Night sky
Colourf skycolor1 = { 0, 0, 0 };
Colourf skycolor2 = { 0, 0, 0 };
// Conversion Utils
static Vector canvasToViewport(float x, float y, FS::RenderState &renderState) {
    constexpr float d = 1.f;
    const Vector viewport{ renderState.width / float(renderState.height), 1 };
    return { x * (viewport.x / float(renderState.width)), y * (viewport.y / float(renderState.height)), d };
}
static Colourf linearToGamma(const Colourf color) {
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
// Random
static float cubeRandom() {
    // get random number between [0.f,1.f]
    static std::random_device device;
    static std::mt19937 gen(device());
    static std::uniform_real_distribution<float> dist(-1.f, 1.f);
    return dist(gen);
}
static Vector unitRandom() {
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
// static Vector hemisphereRandom(const Vector normal) {
//     Vector randvec = unitRandom();
//     if (dot(randvec, normal) > 0.f) {
//         return randvec;
//     } else {
//         return -randvec;
//     }
// }
// Math helpers
static Vector reflectRay(const Vector R, const Vector N) {
    return (2 * (N * dot(R, N)) - R);
}
template <typename T>
static T lerp(const T &a, const T &b, float t) {
    return (a + (t * (b - a)));
}
PathTracer::PathTracer(const std::string &name, const uint32_t width, const uint32_t height) {
    mWindow = new FS::Window(name.c_str(), width, height);
    mWindow->swapBuffers();
    mWindow->processMessages();
}
PathTracer::~PathTracer() {
    delete mWindow;
    bvh.cleanup();
}
void PathTracer::pathTrace() {
    Timer timer;
    FS::Window &window = *this->mWindow;
    FS::RenderState &renderState = window.getRenderState();
    uint32_t segments = std::sqrt(std::thread::hardware_concurrency());
    if(segments < std::sqrt(std::thread::hardware_concurrency())){
        segments++;
    }
    const uint32_t threadCount = segments * segments;
    std::vector<std::atomic_bool> finished(threadCount);

    std::vector<std::thread> threads(threadCount);
    const float segmentwidth = static_cast<float>(renderState.width / float(segments));
    const float segmentHeight = static_cast<float>(renderState.height / float(segments));
    for (uint32_t i = 0; i < threadCount; i++) {
        int ysegment = (i / segments);
        int xsegment = (i % segments);
        Vector2 tileSize = { segmentwidth, segmentHeight };
        Vector2 tileOffset = { xsegment * segmentwidth,ysegment * segmentHeight };
        threads[i] = std::thread(&PathTracer::pathTraceTile, this, tileOffset, tileSize, std::ref(finished[i]), config.sampleCount, config.bounceCount);
    }
    for (uint32_t i = 0; i < threadCount; i++) {
        while (!finished[i]) {
            window.swapBuffers();
            window.processMessages();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        threads[i].join();
    }
    timer.stop();
    std::cout << "Rendered, took : " << timer.diff << " ms\n";
}
void PathTracer::update() {
    handleInput();
    mWindow->swapBuffers();
    mWindow->processMessages();
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
}
bool PathTracer::isRunning() {
    return mWindow->isOpen();
}

void PathTracer::setConfig(PathTracerConfig config) {
    this->config = config;
}

void PathTracer::handleInput() {
    FS::Input &input = mWindow->getInput();
    FS::RenderState &renderState = mWindow->getRenderState();
    if (isDown(FS::Buttons::BUTTON_R)) {
        clearScreen(0x00000000);
        pathTrace();
    }
    if (isDown(FS::Buttons::BUTTON_ESC)) {
        mWindow->close();
    }
    if (isDown(FS::Buttons::BUTTON_P)) {
        exportToPPM((uint32_t *)renderState.screenBuffer, renderState.width, renderState.height, "output.ppm");
    }
}
void PathTracer::setScene(Scene &scene) {
    mScene = scene;
    bvh.build(mScene);
}

IntersectionData PathTracer::closestIntersection(const Vector origin, const Vector direction, float min, float max) {
    IntersectionData intersection = bvh.lookup(origin, direction, min, max, bvh.root);
    return intersection;
}
Colourf PathTracer::traceRay(const Vector origin, const Vector direction, const int bounceCount) {
    IntersectionData intersectData = closestIntersection(origin, direction, 0.001f, FLT_MAX);
    if (intersectData.intersection == FLT_MAX) {
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
    return (traceRay(intersectData.point, newDirection, bounceCount - 1) + illum) * color;
}
void PathTracer::pathTraceTile(const Vector2 offset, const Vector2 tileSize, std::atomic_bool &finished, const int SAMPLE_COUNT, const int BOUNCE) {
    FS::RenderState &renderState = mWindow->getRenderState();
    const Vector origin = { 0, 0, -0.5f };
    for (int y = offset.y; y < (offset.y + tileSize.y); y++) {
        for (int x = offset.x; x < (offset.x + tileSize.x); x++) {
            FS::Colourf colourf;
            for (int i = 0; i < SAMPLE_COUNT; i++) {
                float samplex = x + (cubeRandom() - 0.5f);
                float sampley = y + (cubeRandom() - 0.5f);
                Vector direction = canvasToViewport(samplex - (renderState.width / 2.f), sampley - (renderState.height / 2.f), renderState);
                Colourf color = traceRay(origin, direction, BOUNCE);
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
void PathTracer::exportToPPM(uint32_t *buffer, uint32_t width, uint32_t height, const std::string &file) {
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
void PathTracer::clearScreen(uint32_t color) {
    FS::RenderState &renderState = mWindow->getRenderState();
    for (uint32_t y = 0; y < renderState.height; y++) {
        for (uint32_t x = 0; x < renderState.width; x++) {
            uint32_t index = x + (y * renderState.width);
            ((uint32_t *)renderState.screenBuffer)[index] = color;
        }
    }
}