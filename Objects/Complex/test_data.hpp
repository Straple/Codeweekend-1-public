#pragma once

#include <iostream>
#include <vector>
#include <cstdint>

struct Hero {
    uint32_t base_speed = 0;
    uint32_t base_power = 0;
    uint32_t base_range = 0;

    uint32_t level_speed_coeff = 0;
    uint32_t level_power_coeff = 0;
    uint32_t level_range_coeff = 0;
};

struct Monster {
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t hp = 0;
    uint32_t exp = 0;
    uint32_t gold = 0;
    uint32_t range = 0;
    uint32_t attack = 0;
};

struct TestData {
    uint32_t num_turns = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t start_x = 0;
    uint32_t start_y = 0;

    Hero hero;

    std::vector<Monster> monsters;
};

std::istream &operator>>(std::istream &input, TestData &data);

std::ostream &operator<<(std::ostream &output, const TestData &data);

uint32_t get_dist(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1);
