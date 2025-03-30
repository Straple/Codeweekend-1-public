#pragma once

#include <Objects/Basic/randomizer.hpp>
#include <Objects/Complex/answer.hpp>
#include <Objects/Complex/test_data.hpp>

Answer simulate(const std::vector<uint32_t> &monsters_order, const TestData &test_data);

class Solver {

    TestData test_data;

    std::vector<uint32_t> monsters_order;

    Answer answer;

    bool try_swap(Randomizer &rnd);

public:
    Solver(TestData copy_test_data);

    Answer solve(uint64_t random_seed);
};
