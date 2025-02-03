#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>

namespace utils {

#define TERMINAL_COLOR_RESET   "\x1B[0m"
#define TERMINAL_COLOR_RED     "\x1B[31m"
#define TERMINAL_COLOR_GREEN   "\x1B[32m"
#define TERMINAL_COLOR_YELLOW  "\x1B[33m"
#define TERMINAL_COLOR_BLUE    "\x1B[34m"
#define TERMINAL_COLOR_MAGENTA "\x1B[35m"
#define TERMINAL_COLOR_CYAN    "\x1B[36m"
#define TERMINAL_COLOR_WHITE   "\x1B[37m"

#define TAB "    "

#define INVALID_UINT32 static_cast<uint32_t>(-1)

#define GET_INSTANCE_PROC(instance, name) ((name)vkGetInstanceProcAddr(instance,  #name + 4))

    std::vector<char> readFile(std::string const& file_name);

    class FPSCounter {
    public:
        FPSCounter() : frameCount(0), fps(0) {}

        void start() {
            startTime = std::chrono::steady_clock::now();
        }

        void endFrame() {
            frameCount++;
            auto endTime = std::chrono::steady_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            if (elapsedTime >= 1000) {
                fps = static_cast<double>(frameCount) * 1000.0 / elapsedTime;
                frameCount = 0;
                startTime = endTime;
            }
        }

        double getFPS() const {
            return fps;
        }

    private:
        int frameCount;
        double fps;
        std::chrono::steady_clock::time_point startTime;
    };

}