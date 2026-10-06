#ifndef PATH_TRACER_H
#define PATH_TRACER_H
#include "BVH.h"
#include <FSWindow.h>
#include <string>
struct PathTracerConfig{
    uint32_t sampleCount;
    uint32_t bounceCount;
};
class PathTracer {
    BVH bvh ={};
    FS::Window* mWindow=nullptr;
    Scene mScene;
    //default config
    PathTracerConfig config = {
        .sampleCount = 20,
        .bounceCount = 20
    };
  public:
    PathTracer(const std::string &name, const uint32_t width, const uint32_t height);
    ~PathTracer();

    void setScene(Scene& scene);
    void pathTrace();
    void update();
    bool isRunning();
    void setConfig(PathTracerConfig config);

  private:
    void handleInput();
    //core raytracing
    IntersectionData closestIntersection(const Vector origin, const Vector direction, float min, float max);
    Colourf traceRay(const Vector origin, const Vector direction, const int bounceCount);
    void pathTraceTile(const Vector2 offset, const Vector2 tileSize, std::atomic_bool &finished, const int SAMPLE_COUNT, const int BOUNCE);
    void exportToPPM(uint32_t *buffer, uint32_t width, uint32_t height, const std::string &file);
    void clearScreen(uint32_t color);
};
#endif