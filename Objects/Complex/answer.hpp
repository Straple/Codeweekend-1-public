#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

struct Action {
    enum Action_t {
        MOVE,
        ATTACK,
    } type;

    int64_t x;
    int64_t y;

    int64_t target_id;
};

bool operator==(const Action &lhs, const Action &rhs);

struct Answer {
    int64_t score = 0;
    std::vector<Action> actions;
};

bool operator<(const Answer &lhs, const Answer &rhs);

std::ostream &operator<<(std::ostream &output, const Answer &answer);

std::istream &operator>>(std::istream &input, Answer &answer);
