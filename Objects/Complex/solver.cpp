#include <Objects/Complex/solver.hpp>

#include <Objects/Basic/assert.hpp>
#include <Objects/Basic/time.hpp>

bool compare(double old_score, double cur_score, double temp, Randomizer &rnd) {
    return cur_score >= old_score || rnd.get_d() < std::exp(-((old_score - cur_score) / old_score) / temp);
}

Answer simulate(const std::vector<uint32_t> &monsters_order, const TestData &test_data) {

    Answer answer;
    answer.x = test_data.start_x;
    answer.y = test_data.start_y;

    for (uint32_t monster_it = 0; monster_it < monsters_order.size(); monster_it++) {
        uint32_t monster_id = monsters_order[monster_it];
        const auto &monster = test_data.monsters[monster_id];

        uint32_t speed = (test_data.hero.base_speed * (100 + answer.level * test_data.hero.level_speed_coeff)) / 100;
        uint32_t power = (test_data.hero.base_power * (100 + answer.level * test_data.hero.level_power_coeff)) / 100;
        uint32_t range = (test_data.hero.base_range * (100 + answer.level * test_data.hero.level_range_coeff)) / 100;

        if (answer.actions.size() == test_data.num_turns) {
            break;
        }

        // дойдем до монстра
        while (answer.actions.size() < test_data.num_turns && get_dist(answer.x, answer.y, monster.x, monster.y) > range * range) {
            auto get_move_to_triv = [&]() -> std::pair<uint32_t, uint32_t> {
                // TODO: оптимизировать

                uint32_t best_to_x = answer.x;
                uint32_t best_to_y = answer.y;

                uint32_t left_x = answer.x < speed ? 0 : answer.x - speed;
                uint32_t right_x = std::min(answer.x + speed, test_data.width);

                uint32_t left_y = answer.y < speed ? 0 : answer.y - speed;
                uint32_t right_y = std::min(answer.y + speed, test_data.height);

                for (uint32_t to_x = left_x; to_x <= right_x; to_x++) {
                    for (uint32_t to_y = left_y; to_y <= right_y; to_y++) {
                        // не можем допрыгнуть туда
                        if (get_dist(answer.x, answer.y, to_x, to_y) > speed * speed) {
                            continue;
                        }

                        if (get_dist(to_x, to_y, monster.x, monster.y) < get_dist(best_to_x, best_to_y, monster.x, monster.y)) {
                            best_to_x = to_x;
                            best_to_y = to_y;
                        }
                    }
                }
                ASSERT(!(best_to_x == answer.x && best_to_y == answer.y), "unable to find to pos");
                return {best_to_x, best_to_y};
            };
            auto get_move_to = [&]() -> std::pair<uint32_t, uint32_t> {
                uint32_t best_to_x = answer.x;
                uint32_t best_to_y = answer.y;

                uint32_t px = 0;
                uint32_t py = 0;

                double dx = static_cast<int>(monster.x) - static_cast<int>(answer.x);
                double dy = static_cast<int>(monster.y) - static_cast<int>(answer.y);
                double len = std::sqrt(dx * dx + dy * dy);
                dx /= len;
                dy /= len;
                px = answer.x + dx * speed;
                py = answer.y + dy * speed;

                uint32_t K = 1;

                uint32_t left_x = px < K ? 0 : px - K;
                uint32_t right_x = std::min(px + K, test_data.width);

                uint32_t left_y = py < K ? 0 : py - K;
                uint32_t right_y = std::min(py + K, test_data.height);

                for (uint32_t to_x = left_x; to_x <= right_x; to_x++) {
                    for (uint32_t to_y = left_y; to_y <= right_y; to_y++) {
                        // не можем допрыгнуть туда
                        if (get_dist(answer.x, answer.y, to_x, to_y) > speed * speed) {
                            continue;
                        }

                        // при прыжке в to мы сможем ударить монстра
                        /*if (get_dist(to_x, to_y, monster.x, monster.y) <= range * range) {

                            // тогда хотелось бы прыгнуть поближе к следующему монстру
                            if (monster_it + 1 < monsters_order.size()) {
                                const auto &next_monster = test_data.monsters[monsters_order[monster_it + 1]];

                                if ((best_to_x == answer.x && best_to_y == answer.y) || get_dist(to_x, to_y, next_monster.x, next_monster.y) < get_dist(best_to_x, best_to_y, next_monster.x, next_monster.y)) {
                                    best_to_x = to_x;
                                    best_to_y = to_y;
                                }
                            }
                            else{
                                // следующего монстра нет
                                if ((best_to_x == answer.x && best_to_y == answer.y) || get_dist(to_x, to_y, monster.x, monster.y) < get_dist(best_to_x, best_to_y, monster.x, monster.y)) {
                                    best_to_x = to_x;
                                    best_to_y = to_y;
                                }
                            }
                        } else*/
                        if (get_dist(to_x, to_y, monster.x, monster.y) < get_dist(best_to_x, best_to_y, monster.x, monster.y)) {
                            best_to_x = to_x;
                            best_to_y = to_y;
                        }
                    }
                }
                ASSERT(!(best_to_x == answer.x && best_to_y == answer.y), "unable to find to pos");

                // TODO: он все еще берет точку, которая не очень близкая
                /*auto [triv_to_x, triv_to_y] = get_move_to_triv();
                uint32_t best_dist = get_dist(best_to_x, best_to_y, monster.x, monster.y);
                uint32_t triv_dist = get_dist(triv_to_x, triv_to_y, monster.x, monster.y);
                if(best_dist > triv_dist){
                    std::cout << "here\n";
                    // best_to: (90, 519)
                    // triv_to: (88, 520)
                }
                ASSERT(get_dist(best_to_x, best_to_y, monster.x, monster.y) <= get_dist(triv_to_x, triv_to_y, monster.x, monster.y), "invalid best to");*/
                return {best_to_x, best_to_y};
            };

            auto [to_x, to_y] = get_move_to();
            answer.actions.push_back({Action::Action_t::MOVE, to_x, to_y, 0});
            answer.x = to_x;
            answer.y = to_y;
        }

        uint32_t total_damage = 0;

        // побьем монстра
        while (answer.actions.size() < test_data.num_turns && total_damage < monster.hp) {
            ASSERT(get_dist(answer.x, answer.y, monster.x, monster.y) <= range * range, "failed to attack");

            answer.actions.push_back({Action::Action_t::ATTACK, 0, 0, monster_id});
            total_damage += power;
        }

        // убили монстра
        if (total_damage >= monster.hp) {
            uint32_t add_gold = (monster.gold * 1000) / (1000 + answer.fatigue);
            answer.exp += monster.exp;
            answer.gold += add_gold;

            answer.score += monster.exp * 10 / std::sqrt(answer.actions.size());

            // обновим уровень
            while (true) {
                uint32_t new_lvl_exp_need = 1000 + answer.level * (answer.level + 1) * 50;
                if (answer.exp >= new_lvl_exp_need) {
                    answer.exp -= new_lvl_exp_need;
                    answer.level++;
                    answer.score += answer.level * 100.0 / std::sqrt(answer.actions.size());
                } else {
                    break;
                }
            }
        }
    }

    answer.score += answer.gold;

    return answer;
}

bool Solver::try_swap(Randomizer &rnd) {
    uint32_t a = rnd.get(0, monsters_order.size() - 1);
    uint32_t b = rnd.get(0, monsters_order.size() - 1);

    if (a == b) {
        return false;
    }

    std::swap(monsters_order[a], monsters_order[b]);

    Answer new_answer = simulate(monsters_order, test_data);

    if (compare(answer.score, new_answer.score, temp, rnd)) {
        answer = std::move(new_answer);
        return true;
    } else {
        std::swap(monsters_order[a], monsters_order[b]);
        return false;
    }
}

bool Solver::try_reverse(Randomizer &rnd) {
    uint32_t l = rnd.get(0, monsters_order.size() - 1);
    uint32_t r = rnd.get(0, monsters_order.size() - 1);

    if (l > r) {
        std::swap(l, r);
    }
    if (l == r) {
        return false;
    }

    std::reverse(monsters_order.begin() + l, monsters_order.begin() + r);

    Answer new_answer = simulate(monsters_order, test_data);

    if (compare(answer.score, new_answer.score, temp, rnd)) {
        answer = std::move(new_answer);
        return true;
    } else {
        std::reverse(monsters_order.begin() + l, monsters_order.begin() + r);
        return false;
    }
}

Solver::Solver(TestData copy_test_data, uint64_t random_seed) : test_data(std::move(copy_test_data)) {
    ASSERT(!test_data.monsters.empty(), "monsters is empty");

    Randomizer rnd(random_seed);
    monsters_order.resize(test_data.monsters.size());
    std::iota(monsters_order.begin(), monsters_order.end(), 0);
    std::shuffle(monsters_order.begin(), monsters_order.end(), rnd.generator);
    answer = simulate(monsters_order, test_data);
}

Answer Solver::solve(uint64_t random_seed) {
    Randomizer rnd(random_seed);

    ETimer timer;

    // gold: 197819, score: 237690, step: 1000000, time: 26.2046s, temp: 1.0025e-05
    for (uint32_t step = 0; step <= 1'000'000; step++) {
        bool verdict = false;
        if (rnd.get_d() < 0.5) {
            verdict = try_swap(rnd);
        } else {
            verdict = try_reverse(rnd);
        }

        if (verdict) {
            temp *= 0.999;
            temp = std::max(temp, 0.00001);
        } else {
            temp *= 1.0005;
            temp = std::min(temp, 0.001);
        }

        if (step % 1'000 == 0) {
            std::cout << "gold: " << answer.gold << ", score: " << answer.score << ", step: " << step << ", time: " << timer << ", temp: " << temp << '\n';
        }
    }
    return answer;
}
