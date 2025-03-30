#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

struct Action {
    enum Action_t {
        MOVE,
        ATTACK,
    } type;

    uint32_t x;
    uint32_t y;

    uint32_t attack_id;
};

bool operator==(const Action &lhs, const Action &rhs);

struct Answer {
    // информация о герое
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t exp = 0;
    uint32_t gold = 0;
    uint32_t level = 0;
    uint32_t fatigue = 0;

    double score = 0;

    std::vector<Action> actions;
};

std::ostream &operator<<(std::ostream &output, const Answer &answer);

std::istream &operator>>(std::istream &input, Answer &answer);
