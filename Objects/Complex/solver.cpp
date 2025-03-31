#include <Objects/Complex/solver.hpp>

#include <Objects/Basic/assert.hpp>
#include <Objects/Basic/time.hpp>

#include <set>

bool compare(double old_score, double cur_score, double temp, Randomizer &rnd) {
    return cur_score >= old_score || rnd.get_d() < std::exp(-((old_score - cur_score) / old_score) / temp);
}

Answer simulate(const std::vector<uint32_t> &monsters_order, const TestData &test_data) {

    Answer answer;
    answer.x = test_data.start_x;
    answer.y = test_data.start_y;
    answer.monsters_order = monsters_order;

    /*std::set<uint32_t> S;
    for (uint32_t monster_id: monsters_order) {
        S.insert(monster_id);
    }
    ASSERT(S.size() == monsters_order.size(), "invalid monsters order");*/

    ASSERT(test_data.monsters.size() == monsters_order.size(), "invalid monsters order");

    for (uint32_t monster_it = 0; monster_it < monsters_order.size(); monster_it++) {
        answer.last_monster_i = monster_it;

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
                len = std::min(speed * 1.0, len);
                px = answer.x + dx * len;
                py = answer.y + dy * len;

                constexpr uint32_t K = 2;

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

                        // из этой точки мы можем ударить монстра
                        if (get_dist(to_x, to_y, monster.x, monster.y) <= range * range) {

                            // из best_to мы тоже можем ударить монстра
                            if (get_dist(best_to_x, best_to_y, monster.x, monster.y) <= range * range) {

                                // хотим быть ближе к некст монстру
                                const auto &next_monster = test_data.monsters[monsters_order[monster_it + 1]];
                                if (get_dist(to_x, to_y, next_monster.x, next_monster.y) < get_dist(best_to_x, best_to_y, next_monster.x, next_monster.y)) {
                                    best_to_x = to_x;
                                    best_to_y = to_y;
                                }

                            } else {
                                best_to_x = to_x;
                                best_to_y = to_y;
                            }

                        } else {
                            if (get_dist(to_x, to_y, monster.x, monster.y) < get_dist(best_to_x, best_to_y, monster.x, monster.y)) {
                                best_to_x = to_x;
                                best_to_y = to_y;
                            }
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

            //answer.score += monster.exp * 10 / std::sqrt(answer.actions.size());

            // обновим уровень
            while (true) {
                uint32_t new_lvl_exp_need = 1000 + answer.level * (answer.level + 1) * 50;
                if (answer.exp >= new_lvl_exp_need) {
                    answer.exp -= new_lvl_exp_need;
                    answer.level++;
                    //answer.score += answer.level * 100.0 / std::sqrt(answer.actions.size());
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
    Answer new_answer = answer;
    uint32_t a = rnd.get(0, new_answer.monsters_order.size() - 1);
    uint32_t b = rnd.get(0, new_answer.monsters_order.size() - 1);

    if (a == b) {
        return false;
    }

    std::swap(new_answer.monsters_order[a], new_answer.monsters_order[b]);

    new_answer = simulate(new_answer.monsters_order, test_data);

    if (compare(answer.score, new_answer.score, temp, rnd)) {
        answer = std::move(new_answer);
        return true;
    } else {
        return false;
    }
}

bool Solver::try_throw(Randomizer &rnd) {
    Answer new_answer = answer;
    uint32_t a = rnd.get(0, new_answer.monsters_order.size() - 2);

    std::swap(new_answer.monsters_order[a], new_answer.monsters_order.back());

    new_answer = simulate(new_answer.monsters_order, test_data);

    if (compare(answer.score, new_answer.score, temp, rnd)) {
        answer = std::move(new_answer);
        return true;
    } else {
        return false;
    }
}

bool Solver::try_insert(Randomizer &rnd) {
    Answer new_answer = answer;

    uint32_t a = rnd.get(0, new_answer.monsters_order.size() - 1);
    uint32_t monster_id = new_answer.monsters_order[a];
    new_answer.monsters_order.erase(new_answer.monsters_order.begin() + a);

    new_answer.monsters_order.insert(new_answer.monsters_order.begin() + rnd.get(0, new_answer.monsters_order.size()), monster_id);

    new_answer = simulate(new_answer.monsters_order, test_data);

    if (compare(answer.score, new_answer.score, temp, rnd)) {
        answer = std::move(new_answer);
        return true;
    } else {
        return false;
    }
}

bool Solver::try_insert_smart(Randomizer &rnd) {
    Answer new_answer = answer;

    {
        uint32_t old_num_turns = test_data.num_turns;
        test_data.num_turns = rnd.get(1, test_data.num_turns - 1);
        Answer incomplete_answer = simulate(new_answer.monsters_order, test_data);
        test_data.num_turns = old_num_turns;

        for (uint32_t K = rnd.get(1, 10); K > 0 && incomplete_answer.last_monster_i < new_answer.monsters_order.size(); K--) {

            // найдем самого близкого монстра
            uint32_t best_m = new_answer.monsters_order[incomplete_answer.last_monster_i];
            for (uint32_t i = incomplete_answer.last_monster_i; i < new_answer.monsters_order.size(); i++) {
                auto &best_monster = test_data.monsters[best_m];
                auto &monster = test_data.monsters[new_answer.monsters_order[i]];

                if (get_dist(incomplete_answer.x, incomplete_answer.y, best_monster.x, best_monster.y) >
                    get_dist(incomplete_answer.x, incomplete_answer.y, monster.x, monster.y)) {

                    best_m = new_answer.monsters_order[i];
                }
            }
            new_answer.monsters_order.erase(std::find(new_answer.monsters_order.begin(), new_answer.monsters_order.end(), best_m));
            new_answer.monsters_order.insert(new_answer.monsters_order.begin() + incomplete_answer.last_monster_i, best_m);

            incomplete_answer.last_monster_i++;
        }
    }

    new_answer = simulate(new_answer.monsters_order, test_data);

    if (compare(answer.score, new_answer.score, temp, rnd)) {
        answer = std::move(new_answer);
        return true;
    } else {
        return false;
    }
}

bool Solver::try_insert_segment(Randomizer &rnd) {
    Answer new_answer = answer;

    {
        uint32_t l = rnd.get(0, new_answer.monsters_order.size() - 1);
        uint32_t r = rnd.get(0, new_answer.monsters_order.size() - 1);
        if (l > r) {
            std::swap(l, r);
        }
        std::vector<uint32_t> monsters;
        for (uint32_t i = l; i <= r; i++) {
            monsters.push_back(new_answer.monsters_order[i]);
        }

        new_answer.monsters_order.erase(new_answer.monsters_order.begin() + l, new_answer.monsters_order.begin() + r + 1);

        new_answer.monsters_order.insert(new_answer.monsters_order.begin() + rnd.get(0, new_answer.monsters_order.size()), monsters.begin(), monsters.end());
    }

    new_answer = simulate(new_answer.monsters_order, test_data);

    if (compare(answer.score, new_answer.score, temp, rnd)) {
        answer = std::move(new_answer);
        return true;
    } else {
        return false;
    }
}

bool Solver::try_reverse(Randomizer &rnd) {
    Answer new_answer = answer;
    uint32_t l = rnd.get(0, new_answer.monsters_order.size() - 1);
    uint32_t r = rnd.get(0, new_answer.monsters_order.size() - 1);

    if (l > r) {
        std::swap(l, r);
    }
    if (l == r) {
        return false;
    }

    ASSERT(0 <= l && l < r && r < new_answer.monsters_order.size(), "invalid segment");

    std::reverse(new_answer.monsters_order.begin() + l, new_answer.monsters_order.begin() + r);

    new_answer = simulate(new_answer.monsters_order, test_data);

    if (compare(answer.score, new_answer.score, temp, rnd)) {
        answer = std::move(new_answer);
        return true;
    } else {
        return false;
    }
}

Solver::Solver(std::vector<uint32_t> copy_monsters_order, TestData copy_test_data) : test_data(std::move(copy_test_data)) {
    ASSERT(!test_data.monsters.empty(), "monsters is empty");
    answer.monsters_order = std::move(copy_monsters_order);
    answer = simulate(answer.monsters_order, test_data);
}

Solver::Solver(TestData copy_test_data) : test_data(std::move(copy_test_data)) {
    ASSERT(!test_data.monsters.empty(), "monsters is empty");
    answer.monsters_order.resize(test_data.monsters.size());
    std::iota(answer.monsters_order.begin(), answer.monsters_order.end(), 0);
    answer = simulate(answer.monsters_order, test_data);
}

Answer Solver::solve(uint64_t random_seed) {
    Randomizer rnd(random_seed);

    ETimer timer;

    Answer best_answer = answer;

    // test: 3
    // gold: 197819, score: 237690, step: 1000000, time: 26.2046s, temp: 1.0025e-05
    // ========================================================================================
    // test: 20
    // gold: 305000, score: 321254, step: 1000000, time: 65.0669s, temp: 1.0005e-05
    // gold: 327000, score: 343848, step: 1000000, time: 61.2345s, temp: 1.21647e-05
    // gold: 378000, score: 395827, step: 1000000, time: 70.1689s, temp: 2.67983e-05
    // gold: 386001, score: 404707, step: 1000000, time: 63.5259s, temp: 1.38461e-05
    // gold: 345000, score: 361502, step: 1000000, time: 65.1844s, temp: 2.88849e-05
    // gold: 353001, score: 370223, step: 1000000, time: 69.0404s, temp: 0.000343505
    // gold: 431013, score: 451379, step: 2000000, time: 70.4348s, temp: 1.36999e-90
    // gold: 557049, score: 557049, step: 2000000, time: 62.5673s, temp: 1.36999e-90
    // gold: 637011, score: 637011, step: 2000000, time: 58.5722s, temp: 2.19746e-05
    // gold: 702002, score: 702002, step: 2000000, time: 86.2481s, temp: 2.00938e-05
    // ========================================================================================
    // test: 25
    // gold: 16548, score: 16548, step: 2000000, time: 15.1092s, temp: 2.62239e-05
    // gold: 16786, score: 16786, step: 2000000, time: 14.5175s, temp: 2.62239e-05
    uint32_t step = 0;
    for (; //step <= 2'000'000
         ; step++) {
        if (step % 1'000 == 0 && timer.get_ms() > 60'000) {
            break;
        }

        double old_score = answer.score;

        double p = rnd.get_d();
        // try_swap(rnd); // 319002
        // try_insert(rnd); // 393031
        // try_insert_smart(rnd); // 428112 -> 518113
        // try_reverse(rnd);// 263057

        if (p < 0.5) {
            try_insert_smart(rnd);
        } else if (p < 0.8) {
            try_insert(rnd);
        } else {
            try_reverse(rnd);
        }

        /*if (p < 0.2) {
            try_swap(rnd);
        }
        //else if (p < 0.6) {try_throw(rnd);}
        else if (p < 0.7) {
            try_insert_smart(rnd);
        } else if (p < 0.9) {
            try_insert(rnd);
        } else {
            try_reverse(rnd);
        }*/

        if (answer.score > best_answer.score) {
            best_answer = answer;
        }

        temp *= 0.9999;
        if (old_score < answer.score) {
            temp = std::min(temp * 0.9999, 0.00001);
        } else {
            temp = std::min(temp * 1.0005, 0.002);
        }

        /*if (old_score < answer.score) {
            temp = std::max(temp * 0.99, 0.000001);
        } else {
            temp = std::min(temp * 1.00001, 0.002);
        }*/

        if (step % 1'000 == 0) {
            //std::cout << "gold: " << answer.gold << ", score: " << answer.score << ", step: " << step << ", time: " << timer << ", temp: " << temp << '\n';
        }
    }
    //std::cout << "best:\n";
    //std::cout << "gold: " << best_answer.gold << ", score: " << best_answer.score << ", step: " << step << ", time: " << timer << ", temp: " << temp << '\n';
    return best_answer;
}
