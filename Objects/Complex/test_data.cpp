#include <Objects/Complex/test_data.hpp>

#include <Objects/Basic/assert.hpp>
#include <Objects/nlohmann/json.hpp>

using json = nlohmann::json;

void TestData::build() {
    monsters_attack.assign((height + 1) * (width + 1), {});
    for (uint32_t y = 0; y <= height; y++) {
        for (uint32_t x = 0; x <= width; x++) {
            for (uint32_t m = 0; m < monsters.size(); m++) {
                const auto &monster = monsters[m];

                if (monster.attack == 0 ||
                    get_dist(x, y, monster.x, monster.y) > monster.range * monster.range) {
                    continue;
                }

                monsters_attack[y * (width + 1) + x].push_back(m);
            }
        }
    }
}

std::istream &operator>>(std::istream &input, TestData &data) {
    ASSERT(input, "unable to read");

    try {
        json json = json::parse(input);

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
