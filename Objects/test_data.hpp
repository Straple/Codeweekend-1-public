#pragma once

#include <iostream>
#include <vector>

using cnt_t = long long;

struct Hero {
    cnt_t base_speed = 0;
    cnt_t base_power = 0;
    cnt_t base_range = 0;

    cnt_t level_speed_coeff = 0;
    cnt_t level_power_coeff = 0;
    cnt_t level_range_coeff = 0;
};

struct Monster {
    cnt_t x = 0;
    cnt_t y = 0;
    cnt_t hp = 0;
    cnt_t exp = 0;
    cnt_t gold = 0;
    cnt_t range = 0;
    cnt_t attack = 0;
};

struct TestData {
    cnt_t num_turns = 0;
    cnt_t width = 0;
    cnt_t height = 0;
    cnt_t start_x = 0;
    cnt_t start_y = 0;

    Hero hero;

    std::vector<Monster> monsters;
};

std::istream &operator>>(std::istream &input, TestData &data);

std::ostream &operator<<(std::ostream &output, const TestData &data);

long long readInt(std::istream &input);