#pragma once

#include <cstdint>
#include <thread>
#include <vector>

template<typename foo_t>
void launch_threads(const uint32_t threads_num, foo_t &&foo) {
    std::vector<std::thread> threads(threads_num);
    for (uint32_t thr = 0; thr < threads.size(); thr++) {
        threads[thr] = std::thread(foo, thr);
    }
    for (uint32_t thr = 0; thr < threads.size(); thr++) {
        threads[thr].join();
    }
}
