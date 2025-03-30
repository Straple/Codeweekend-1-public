#include <Objects/Complex/answer.hpp>

#include <Objects/Basic/assert.hpp>
#include <Objects/nlohmann/json.hpp>

using json = nlohmann::json;

std::ostream &operator<<(std::ostream &output, const Answer &answer) {
    json json;

    json["x"] = answer.x;
    json["y"] = answer.y;
    json["exp"] = answer.exp;
    json["gold"] = answer.gold;
    json["level"] = answer.level;
    json["fatigue"] = answer.fatigue;
    json["monsters_order"] = answer.monsters_order;

    for (auto action: answer.actions) {
        if (action.type == Action::Action_t::ATTACK) {
            json["moves"].push_back({
                    {"type", "attack"},
                    {"target_id", action.attack_id},
            });
        } else {
            json["moves"].push_back({
                    {"type", "move"},
                    {"target_x", action.x},
                    {"target_y", action.y},
            });
        }
    }
    output << std::setfill('\t') << std::setw(1) << json;
    return output;
}

std::istream &operator>>(std::istream &input, Answer &answer) {
    ASSERT(input, "unable to read");

    try {
        json json = json::parse(input);

        answer.x = json["x"];
        answer.y = json["y"];
        answer.exp = json["exp"];
        answer.gold = json["gold"];
        answer.level = json["level"];
        answer.fatigue = json["fatigue"];
        answer.monsters_order = std::vector<uint32_t>(json["monsters_order"]);

        for (auto &action: json["moves"]) {
            if (action["type"] == "attack") {
                answer.actions.push_back({Action::Action_t::ATTACK, 0, 0, action["target_id"]});
            } else if (action["type"] == "move") {
                answer.actions.push_back({Action::Action_t::MOVE, action["target_x"], action["target_y"], 0});
            } else {
                FAILED_ASSERT("invalid action type");
            }
        }

    } catch (const json::parse_error &error) {
        FAILED_ASSERT("TestData read failed, message: >" + std::string(error.what()) + "<");
    }

    return input;
}
