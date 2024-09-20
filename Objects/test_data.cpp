#include "test_data.hpp"
#include "../assert.hpp"
#include <climits>

std::istream &operator>>(std::istream &input, TestData &data) {
    ASSERT(input, "unable to read");

    data.height = readInt(input);

    data.hero.base_power = readInt(input);
    data.hero.base_range = readInt(input);
    data.hero.base_speed = readInt(input);
    data.hero.level_power_coeff = readInt(input);
    data.hero.level_range_coeff = readInt(input);
    data.hero.level_speed_coeff = readInt(input);

    data.num_turns = readInt(input);
    data.start_x = readInt(input);
    data.start_y = readInt(input);
    data.width = readInt(input);

    while (true) {
        Monster monster;
        if (!input) {
            break;
        }

        monster.attack = readInt(input);
        if (monster.attack == LLONG_MAX) {
            break;
        }
        monster.exp = readInt(input);
        monster.gold = readInt(input);
        monster.hp = readInt(input);
        monster.range = readInt(input);
        monster.x = readInt(input);
        monster.y = readInt(input);

        data.monsters.push_back(monster);
    }

    return input;
}

std::ostream &operator<<(std::ostream &output, const TestData &data) {

    output << "{\n";

    output << "\t\"height\": " << data.height << ",\n";

    output << "\t\"hero\": {\n";
    output << "\t\t\"base_power\": " << data.hero.base_power << ",\n";
    output << "\t\t\"base_range\": " << data.hero.base_range << ",\n";
    output << "\t\t\"base_speed\": " << data.hero.base_speed << ",\n";
    output << "\t\t\"level_power_coeff\": " << data.hero.level_power_coeff << ",\n";
    output << "\t\t\"level_range_coeff\": " << data.hero.level_range_coeff << ",\n";
    output << "\t\t\"level_speed_coeff\": " << data.hero.level_speed_coeff << "\n";
    output << "\t},\n";

    output << "\t\"num_turns\": " << data.num_turns << ",\n";
    output << "\t\"start_x\": " << data.start_x << ",\n";
    output << "\t\"start_y\": " << data.start_y << ",\n";
    output << "\t\"width\": " << data.width << ",\n";

    output << "\t\"monsters\": [\n";
    for (int i = 0; i < data.monsters.size(); i++) {
        auto monster = data.monsters[i];
        output << "\t\t{\n";
        output << "\t\t\t\"attack\": " << monster.attack << ",\n";
        output << "\t\t\t\"exp\": " << monster.exp << ",\n";
        output << "\t\t\t\"gold\": " << monster.gold << ",\n";
        output << "\t\t\t\"hp\": " << monster.hp << ",\n";
        output << "\t\t\t\"range\": " << monster.range << ",\n";
        output << "\t\t\t\"x\": " << monster.x << ",\n";
        output << "\t\t\t\"y\": " << monster.y << "\n";
        output << "\t\t}";
        if (i + 1 < data.monsters.size()) {
            output << ",";
        }
        output << "\n";
    }
    output << "\t]\n";

    output << "}\n";

    return output;
}

long long readInt(std::istream &input) {
    while (input) {
        char c;
        input >> c;
        if (c == '-' || c == '+' || ('0' <= c && c <= '9')) {
            // int!
            long long x = 0;
            if ('0' <= c && c <= '9') {
                x = c - '0';
            }
            char dig;
            while ((input >> dig) && ('0' <= dig && dig <= '9')) {
                x *= 10;
                x += dig - '0';
            }
            if (c == '-') {
                x *= -1;
            }
            return x;
        }
    }
    return LLONG_MAX;
}