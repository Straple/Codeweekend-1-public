#include <Objects/Complex/answer.hpp>

#include <Objects/Basic/assert.hpp>
#include <Objects/nlohmann/json.hpp>

using json = nlohmann::json;

bool operator==(const Action &lhs, const Action &rhs) {
    return lhs.type == rhs.type &&
           lhs.x == rhs.x &&
           lhs.y == rhs.y &&
           lhs.attack_id == rhs.attack_id;
}

std::ostream &operator<<(std::ostream &output, const Answer &answer) {
    json json;

    json["x"] = answer.x;
    json["y"] = answer.y;
    json["exp"] = answer.exp;
    json["gold"] = answer.gold;
    json["level"] = answer.level;
    json["fatigue"] = answer.fatigue;

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
    output << json;
    /*output << "{\n";
    output << "\t\"moves\": [\n";
    for (int i = 0; i < answer.actions.size(); i++) {
        auto &action = answer.actions[i];
        output << "\t\t{\n";
        output << "\t\t\t\"type\": \"" << (action.type == Action::Action_t::MOVE ? "move" : "attack") << "\",\n";
        if (action.type == Action::Action_t::MOVE) {
            output << "\t\t\t\"target_x\": " << action.x << ",\n";
            output << "\t\t\t\"target_y\": " << action.y << "\n";
        } else {
            output << "\t\t\t\"target_id\": " << action.attack_id << "\n";
        }
        output << "\t\t}";
        if (i + 1 < answer.actions.size()) {
            output << ",";
        }
        output << "\n";
    }
    output << "\t]\n";
    output << "}\n";*/
    return output;
}

std::istream &operator>>(std::istream &input, Answer &answer) {
    ASSERT(input, "unable to read");
    FAILED_ASSERT("TODO");
    //answer.score = readInt(input);

    /*while (true) {
        Action action;
        std::string s;
        bool ok = false;
        while (input >> s) {
            if (s == "\"move\",") {
                ok = true;
                action.type = Action::Action_t::MOVE;
                break;
            } else if (s == "\"attack\",") {
                ok = true;
                action.type = Action::Action_t::ATTACK;
                break;
            }
        }

        if (!ok) {
            break;
        }

        if (action.type == Action::Action_t::MOVE) {
            //action.x = readInt(input);
            //action.y = readInt(input);
        } else {
            //action.target_id = readInt(input);
        }

        answer.actions.push_back(action);
    }*/

    return input;
}