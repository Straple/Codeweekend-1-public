#pragma once

#include "test_data.hpp"
#include <iostream>
#include <vector>

struct Action {
    enum Action_t {
        MOVE,
        ATTACK,
    } type;

    cnt_t x;
    cnt_t y;

    cnt_t target_id;

    friend bool operator==(Action lhs, Action rhs);
};

struct Answer {
    cnt_t score = 0;
    std::vector<Action> actions;
};

bool operator<(const Answer &lhs, const Answer &rhs);

std::ostream &operator<<(std::ostream &output, const Answer &answer);

std::istream &operator>>(std::istream &input, Answer &answer);