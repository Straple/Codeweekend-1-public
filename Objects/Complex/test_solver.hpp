#pragma once

#include <Objects/Basic/randomizer.hpp>
#include <Objects/Complex/answer.hpp>
#include <Objects/Complex/test_data.hpp>

// хранит хорошее множество ответов и улучшает их
class TestSolver {

    std::string dirname;

    TestData test_data;

    struct Item {
        Answer answer;
        uint32_t cnt_failed_improve = 0;
    };

    std::vector<Item> items;

    void mow();

    void bubble_sort(uint32_t i);

public:
    TestSolver() = default;

    explicit TestSolver(TestData test_data, std::string dirname);

    void improve(Randomizer &rnd);

    void add(Randomizer &rnd);

    void print_state(std::ostream& output) const;

    void write(const std::string& dirname) const;

    [[nodiscard]] Answer get_best() const;
};
