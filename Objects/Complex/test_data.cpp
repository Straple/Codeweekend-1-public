#include <Objects/Complex/test_data.hpp>

#include <Objects/Basic/assert.hpp>
#include <Objects/nlohmann/json.hpp>

using json = nlohmann::json;

void TestData::build() {
    // build monsters_order
    {
        // std::cout << "Test id: " << test_id << '\n';
        // (hp, exp, gold)
        // std::map<std::tuple<uint32_t, uint32_t, uint32_t>, uint32_t> map;

        for (uint32_t m = 0; m < monsters.size(); m++) {
            auto &monster = monsters[m];

            // skip bad monsters
            if (test_id == 17 && monster.gold == 1) {
                continue;
            } else if (test_id == 18 && monster.gold == 1) {
                continue;
            } else if (test_id == 19 && monster.gold == 1) {
                continue;
            } else if (test_id == 20 && monster.gold == 1) {
                continue;
            } else if (test_id == 21 && monster.gold == 1) {
                continue;
            } else if (test_id == 36 && monster.hp == 7777777) {
                continue;
            } else if (test_id == 37 && monster.hp == 7777777) {
                continue;
            } else if (monster.hp / hero.base_power >= 50) {
                continue;// слишком жирный
            }

            monsters_order.push_back(m);

            // map[{monster.hp, monster.exp, monster.gold}]++;
        }
        //std::cout << monsters.size() << " -> " << monsters_order.size() << '\n';
        /*for (auto [params, cnt]: map) {
            if (cnt != 1) {
                auto [hp, exp, gold] = params;
                std::cout << hp << ' ' << exp << ' ' << gold << ":  " << cnt << '\n';
            }
        }*/
    }

    monsters_attack.assign((height + 1) * (width + 1), {});

    for (uint32_t m = 0; m < monsters.size(); m++) {
        const auto &monster = monsters[m];
        if (monster.attack == 0) {
            continue;
        }
        uint32_t left = monster.x >= monster.range ? monster.x - monster.range : 0;
        uint32_t right = std::min(width, monster.x + monster.range);
        uint32_t top = monster.y >= monster.range ? monster.y - monster.range : 0;
        uint32_t bottom = std::min(height, monster.y + monster.range);
        for (uint32_t y = top; y <= bottom; y++) {
            for (uint32_t x = left; x <= right; x++) {
                if (get_dist(x, y, monster.x, monster.y) <= monster.range * monster.range) {
                    monsters_attack[y * (width + 1) + x].push_back(m);
                }
            }
        }
    }
}

std::istream &operator>>(std::istream &input, TestData &data) {
    ASSERT(input, "unable to read");

    try {
        json json = json::parse(input);

        data.test_id = json["test_id"];

        data.height = json["height"];
        data.width = json["width"];

        data.start_x = json["start_x"];
        data.start_y = json["start_y"];

        data.num_turns = json["num_turns"];

        data.hero.base_power = json["hero"]["base_power"];
        data.hero.base_range = json["hero"]["base_range"];
        data.hero.base_speed = json["hero"]["base_speed"];
        data.hero.level_power_coeff = json["hero"]["level_power_coeff"];
        data.hero.level_range_coeff = json["hero"]["level_range_coeff"];
        data.hero.level_speed_coeff = json["hero"]["level_speed_coeff"];

        for (auto &json_monster: json["monsters"]) {
            Monster monster;
            monster.x = json_monster["x"];
            monster.y = json_monster["y"];
            monster.hp = json_monster["hp"];
            monster.attack = json_monster["attack"];
            monster.exp = json_monster["exp"];
            monster.gold = json_monster["gold"];
            monster.range = json_monster["range"];
            data.monsters.emplace_back(monster);
        }

    } catch (const json::parse_error &error) {
        FAILED_ASSERT("TestData read failed, message: >" + std::string(error.what()) + "<");
    }

    data.build();

    return input;
}

std::ostream &operator<<(std::ostream &output, const TestData &data) {
    FAILED_ASSERT("TODO");
    /*output << "{\n";

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

    output << "}\n";*/

    return output;
}

uint32_t get_dist(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1) {
    return (x0 - x1) * (x0 - x1) + (y0 - y1) * (y0 - y1);
}
