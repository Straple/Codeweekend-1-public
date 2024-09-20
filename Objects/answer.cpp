#include "answer.hpp"
#include "../assert.hpp"

bool operator==(Action lhs, Action rhs) {
    return lhs.type == rhs.type &&
           lhs.x == rhs.x &&
           lhs.y == rhs.y &&
           lhs.target_id == rhs.target_id;
}

bool operator<(const Answer &lhs, const Answer &rhs) {
    return lhs.score < rhs.score;
}

std::ostream &operator<<(std::ostream &output, const Answer &answer) {
    output << "score: " << answer.score << '\n';
    output << "{\n";
    output << "\t\"moves\": [\n";
    for (int i = 0; i < answer.actions.size(); i++) {
        auto &action = answer.actions[i];
        output << "\t\t{\n";
        output << "\t\t\t\"type\": \"" << (action.type == Action::Action_t::MOVE ? "move" : "attack") << "\",\n";
        if (action.type == Action::Action_t::MOVE) {
            output << "\t\t\t\"target_x\": " << action.x << ",\n";
            output << "\t\t\t\"target_y\": " << action.y << "\n";
        } else {
            output << "\t\t\t\"target_id\": " << action.target_id << "\n";
        }
        output << "\t\t}";
        if (i + 1 < answer.actions.size()) {
            output << ",";
        }
        output << "\n";
    }
    output << "\t]\n";
    output << "}\n";
    return output;
}

std::istream &operator>>(std::istream &input, Answer &answer) {
    ASSERT(input, "unable to read");
    answer.score = readInt(input);

    while (true) {
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
            action.x = readInt(input);
            action.y = readInt(input);
        } else {
            action.target_id = readInt(input);
        }

        answer.actions.push_back(action);
    }

    return input;
}