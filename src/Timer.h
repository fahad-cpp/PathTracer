#ifndef TIMER_H
#define TIMER_H
#include <chrono>
class Timer{
    public:
    float diff;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    Timer(){
        startTime = std::chrono::high_resolution_clock::now();
    }
    void start(){
        startTime = std::chrono::high_resolution_clock::now();
    }
    void stop(){
        std::chrono::time_point<std::chrono::high_resolution_clock> endTime = std::chrono::high_resolution_clock::now();

        diff = std::chrono::duration<float,std::chrono::milliseconds::period>(endTime - startTime).count();
    }
};
#endif