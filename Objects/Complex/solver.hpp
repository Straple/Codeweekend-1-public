#pragma once

#include <Objects/Basic/randomizer.hpp>
#include <Objects/Complex/answer.hpp>
#include <Objects/Complex/test_data.hpp>

Answer simulate(const std::vector<uint32_t> &monsters_order, const TestData &test_data);

class Solver {

    TestData test_data;

    Answer answer;

    double temp = 0.001;

    // TODO: добавить более умную операцию
    // которая симулирует игру, а затем в какой-то момент берет монстров жадно

    bool try_swap(Randomizer &rnd);

    bool try_throw(Randomizer &rnd);

    // TODO: операция try_insert оочень хороша
    // нужно бы сделать, чтобы она не один элемент добавляла, а некоторый маленький отрезок
    bool try_insert(Randomizer &rnd);

    bool try_reverse(Randomizer &rnd);

public:
    Solver(std::vector<uint32_t> monsters_order, TestData test_data);

    explicit Solver(TestData test_data);

    Answer solve(uint64_t random_seed);
};
