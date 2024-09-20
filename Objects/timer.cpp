#include "timer.hpp"

#include <iomanip>

#ifdef _WIN32

#include <windows.h>
// windows.h defined min and max macros
// this is bad
#undef min
#undef max

uint64_t calc_performance_frequency() {
    LARGE_INTEGER perf;
    //ASSERT(QueryPerformanceFrequency(&perf), "call to QueryPerformanceFrequency fails");
    QueryPerformanceFrequency(&perf) return perf.QuadPart;
}

uint64_t get_ticks() {
    LARGE_INTEGER ticks;
    //ASSERT(QueryPerformanceCounter(&ticks), "call to QueryPerformanceCounter fails");
    QueryPerformanceCounter(&ticks) return ticks.QuadPart;
}

#elif defined(__linux__) || defined(__APPLE__)

#include <sys/time.h>

uint64_t calc_performance_frequency() {
    return 1'000'000;// колво микросекунд в секунде
}

uint64_t get_ticks() {
    timeval ticks;

    // вернет -1 в случае ошибки
    //ASSERT(gettimeofday(&ticks, NULL) == 0, "call to gettimeofday fails");
    gettimeofday(&ticks, NULL);
    return ticks.tv_sec * 1'000'000 + ticks.tv_usec;
}

#else
static_assert(false, "not supported operating system");
#endif

const double get_performance_frequency() {
    static const double performance_frequency = static_cast<double>(calc_performance_frequency());
    return performance_frequency;
}

Timer::Timer()
    : start_tick(get_ticks()) {
}

[[nodiscard]] double Timer::get() const {
    return static_cast<double>(get_ticks() - start_tick) / get_performance_frequency();
}

void Timer::reset() {
    start_tick = get_ticks();
}

[[nodiscard]] uint64_t Timer::get_tick() const {
    return start_tick;
}

std::ostream &operator<<(std::ostream &output, const Timer &time) {
    double t = time.get();
    if (t >= 1) {
        output << t << "s";
    } else if (t >= 1e-3) {
        output << t * 1e3 << "ms";
    } else if (t >= 1e-6) {
        output << t * 1e6 << "us";
    } else if (t >= 1e-9) {
        output << t * 1e9 << "ns";
    } else {
        output << "0s";
    }
    return output;
}