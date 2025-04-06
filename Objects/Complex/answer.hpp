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

struct Answer {
    // информация о герое
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t exp = 0;
    uint32_t gold = 0;
    uint32_t level = 0;
    uint64_t fatigue = 0;

    // настройки симуляции
    uint32_t window_len = 2;// размер окна, в котором мы ищем точку прыжка
    int64_t fatigue_weight = 200;
    bool enable_stop = true;
    uint64_t random_seed = 0;// unused

    double score;

    std::vector<Action> actions;

    // последний указатель на монстра в monsters_order, которого мы убили
    uint32_t last_monster_i = 0;

    std::vector<uint32_t> monsters_order;
};

std::ostream &operator<<(std::ostream &output, const Answer &answer);

std::istream &operator>>(std::istream &input, Answer &answer);
