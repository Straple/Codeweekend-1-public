#pragma once

#include <random>

struct Randomizer {
    std::mt19937_64 generator;

public:
    Randomizer();

    explicit Randomizer(uint64_t seed);

    // uint64_t
    uint64_t get();

    // int64_t [left, right]
    int64_t get(int64_t left, int64_t right);

    // double [0, 1]
    double get_d();

    // double [left, right]
    double get_d(double left, double right);
};
