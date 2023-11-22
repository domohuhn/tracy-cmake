#include "instrumentation.hpp"
#include <thread>
#include <cstdio>
#include <chrono>



void fun2() {
    ZoneScopedS(10);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}


void fun3() {
    MEASURE_SCOPE;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    fun2();
}

void fun1() {
    MEASURE_SCOPE;
    fun2();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    fun3();
}

int main() {
    while(true) {
        MEASURE_SCOPE;
        auto start = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(27));
        fun1();
        auto end = std::chrono::steady_clock::now();
        printf("duration %f s\n", std::chrono::duration<double>(end-start).count());
        FRAME_MARKER;
    }
    return 0;
}
