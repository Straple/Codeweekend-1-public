#pragma once

#include <Objects/Basic/randomizer.hpp>
#include <Objects/Complex/answer.hpp>
#include <Objects/Complex/test_data.hpp>

Answer simulate(Answer answer, const TestData &test_data);

class Solver {

    TestData test_data;

    Answer answer;

    double temp = 1;

    bool try_swap(Randomizer &rnd);

    bool try_insert(Randomizer &rnd);

    bool try_insert_smart(Randomizer &rnd);

    bool try_insert_segment(Randomizer &rnd);

    bool try_move(Randomizer &rnd);

    bool try_reverse(Randomizer &rnd);

    bool try_change_settings(Randomizer &rnd);

public:

    Solver(Answer answer, TestData test_data);

    Solver(std::vector<uint32_t> monsters_order, TestData test_data);

    explicit Solver(TestData test_data);

    Answer solve(uint64_t random_seed);
};
