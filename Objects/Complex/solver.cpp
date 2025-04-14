#include <Objects/Complex/solver.hpp>

#include <Objects/Basic/assert.hpp>
#include <Objects/Basic/time.hpp>

#include <tuple>

bool compare(double old_score, double cur_score, double temp, Randomizer &rnd) {
    return cur_score >= old_score || rnd.get_d() < std::exp(-((old_score - cur_score) / old_score) / (temp * 0.001));
}

Answer simulate(Answer answer, const TestData &test_data) {
    answer.x = test_data.start_x;
    answer.y = test_data.start_y;
    answer.exp = 0;
    answer.gold = 0;
    answer.level = 0;
    answer.fatigue = 0;
    answer.last_monster_i = 0;
    answer.score = 0;
    answer.actions.clear();

    std::vector<bool> is_killed(test_data.monsters.size());

    ASSERT(test_data.monsters_order.size() == answer.monsters_order.size(), "invalid monsters order");

    if (test_data.test_id == 36 || test_data.test_id == 37) {
        uint32_t speed = (test_data.hero.base_speed * (100 + answer.level * test_data.hero.level_speed_coeff)) / 100;
        while (answer.y + 2 * speed <= test_data.height) {
            answer.actions.push_back({Action::Action_t::MOVE, answer.x, answer.y + speed, 0});
            answer.y += speed;
        }
    }

    for (uint32_t monster_it = 0; monster_it < answer.monsters_order.size(); monster_it++) {
        uint32_t monster_id = answer.monsters_order[monster_it];
        const auto &monster = test_data.monsters[monster_id];

        uint32_t speed = (test_data.hero.base_speed * (100 + answer.level * test_data.hero.level_speed_coeff)) / 100;
        uint32_t power = (test_data.hero.base_power * (100 + answer.level * test_data.hero.level_power_coeff)) / 100;
        uint32_t range = (test_data.hero.base_range * (100 + answer.level * test_data.hero.level_range_coeff)) / 100;

        if (answer.actions.size() == test_data.num_turns) {
            break;
        }

        //if ((monster.hp + power - 1) / power > 30) {
        //    continue;// слишком жирный
        //}

        auto update_fatigue = [&](uint32_t cnt) {
            uint32_t pos = answer.y * (test_data.width + 1) + answer.x;
            for (uint32_t m: test_data.monsters_attack[pos]) {
                if (!is_killed[m]) {
                    answer.fatigue += test_data.monsters[m].attack * cnt;
                }
            }
        };

        // дойдем до монстра
        while (answer.actions.size() < test_data.num_turns && get_dist(answer.x, answer.y, monster.x, monster.y) > range * range) {
            auto get_move_to = [&]() -> std::pair<uint32_t, uint32_t> {
                uint32_t best_to_x = answer.x;
                uint32_t best_to_y = answer.y;

                double dx = static_cast<int>(monster.x) - static_cast<int>(answer.x);
                double dy = static_cast<int>(monster.y) - static_cast<int>(answer.y);
                double len = std::sqrt(dx * dx + dy * dy);
                dx /= len;
                dy /= len;
                len = std::min(speed * 1.0 - 1, len - 1);
                if (answer.enable_stop && monster.range < range) {
                    len = std::min(speed * 1.0, len - monster.range * 1.0);
                }
                uint32_t px = answer.x + dx * len;
                uint32_t py = answer.y + dy * len;

                uint32_t left_x = px < answer.window_len ? 0 : px - answer.window_len;
                uint32_t right_x = std::min(px + answer.window_len, test_data.width);

                uint32_t left_y = py < answer.window_len ? 0 : py - answer.window_len;
                uint32_t right_y = std::min(py + answer.window_len, test_data.height);

                auto get_dot_score = [&](uint32_t to_x, uint32_t to_y) {
                    int64_t score = 0;
                    score -= get_dist(to_x, to_y, monster.x, monster.y);

                    if (get_dist(to_x, to_y, monster.x, monster.y) <= range * range) {
                        score = 1'000'000;

                        for (uint32_t it = monster_it + 1; it < answer.monsters_order.size(); it++) {
                            const auto &monster = test_data.monsters[answer.monsters_order[it]];

                            if (get_dist(to_x, to_y, monster.x, monster.y) <= range * range) {
                                score += 10'000;
                            } else {
                                score -= get_dist(to_x, to_y, monster.x, monster.y);
                                break;
                            }
                        }
                    }

                    uint64_t fatigue = 0;
                    {
                        uint32_t pos = to_y * (test_data.width + 1) + to_x;
                        for (uint32_t m: test_data.monsters_attack[pos]) {
                            if (!is_killed[m]) {
                                fatigue += test_data.monsters[m].attack;
                            }
                        }
                        fatigue = std::min((uint64_t) 1'000'000'000, fatigue);
                    }
                    score -= fatigue * answer.fatigue_weight;
                    if (fatigue > 1'000'000) {
                        score = -1e18;
                    }
                    return score;
                };

                int64_t best_score = -1e17;

                for (uint32_t to_y = left_y; to_y <= right_y; to_y++) {
                    for (uint32_t to_x = left_x; to_x <= right_x; to_x++) {
                        // не можем допрыгнуть туда
                        if (get_dist(answer.x, answer.y, to_x, to_y) > speed * speed) {
                            continue;
                        }

                        int64_t score = get_dot_score(to_x, to_y);

                        if (score > best_score) {
                            best_score = score;
                            best_to_x = to_x;
                            best_to_y = to_y;
                        }
                    }
                }

                return {best_to_x, best_to_y};
            };

            auto [to_x, to_y] = get_move_to();
            ASSERT(get_dist(answer.x, answer.y, to_x, to_y) <= speed * speed, "too far to move");

            answer.actions.push_back({Action::Action_t::MOVE, to_x, to_y, 0});
            answer.x = to_x;
            answer.y = to_y;
            update_fatigue(1);
        }

        uint32_t total_damage = 0;

        // побьем монстра
        while (answer.actions.size() < test_data.num_turns && total_damage < monster.hp) {
            ASSERT(get_dist(answer.x, answer.y, monster.x, monster.y) <= range * range, "to far to attack");

            answer.actions.push_back({Action::Action_t::ATTACK, 0, 0, monster_id});
            total_damage += power;
        }

        update_fatigue(total_damage / power);

        // убили монстра
        if (total_damage >= monster.hp) {
            answer.last_monster_i = monster_it;
            is_killed[monster_id] = true;

            // монстр не атаковал нас после смерти
            if (get_dist(answer.x, answer.y, monster.x, monster.y) <= monster.range * monster.range) {
                answer.fatigue -= monster.attack;
            }

            uint32_t add_gold = (monster.gold * 1000) / (1000 + answer.fatigue);
            answer.exp += monster.exp;
            answer.gold += add_gold;

            // обновим уровень
            while (true) {
                uint32_t new_lvl_exp_need = 1000 + answer.level * (answer.level + 1) * 50;
                if (answer.exp >= new_lvl_exp_need) {
                    answer.exp -= new_lvl_exp_need;
                    answer.level++;
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
    uint32_t a = rnd.get(0, answer.monsters_order.size() - 1);
    uint32_t b = 0;
    if (rnd.get_d() < 0.5) {
        b = rnd.get(0, answer.monsters_order.size() - 1);
    } else {
        b = std::min(a + (uint32_t) rnd.get(-10, 10), (uint32_t) answer.monsters_order.size() - 1);
    }

    if (a == b) {
        return false;
    }

    Answer new_answer = answer;
    std::swap(new_answer.monsters_order[a], new_answer.monsters_order[b]);
    new_answer = simulate(new_answer, test_data);

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

    new_answer = simulate(new_answer, test_data);

    if (compare(answer.score, new_answer.score, temp, rnd)) {
        answer = std::move(new_answer);
        return true;
    } else {
        return false;
    }
}

bool Solver::try_move(Randomizer &rnd) {
    Answer new_answer = answer;

    uint32_t a = rnd.get(0, new_answer.monsters_order.size() - 1);
    uint32_t monster_id = new_answer.monsters_order[a];
    new_answer.monsters_order.erase(new_answer.monsters_order.begin() + a);

    uint32_t left = a >= 10 ? a - 10 : 0;
    uint32_t right = std::min(a + 10, static_cast<uint32_t>(new_answer.monsters_order.size()));

    new_answer.monsters_order.insert(new_answer.monsters_order.begin() + rnd.get(left, right), monster_id);

    new_answer = simulate(new_answer, test_data);

    if (compare(answer.score, new_answer.score, temp, rnd)) {
        answer = std::move(new_answer);
        return true;
    } else {
        return false;
    }
}

bool Solver::try_insert_smart(Randomizer &rnd) {
    Answer new_answer = answer;

    if (rnd.get_d() < 0.5) {
        uint32_t k = rnd.get(0, new_answer.last_monster_i);

        uint32_t z = rnd.get(1, 10);
        if (rnd.get_d() < 0.5) {
            z = rnd.get(10, 100);
        }

        for (; z > 0 && k + 1 < new_answer.monsters_order.size(); z--) {
            const auto &cur_monster = test_data.monsters[new_answer.monsters_order[k]];
            // найдем ближайшего монстра к cur_monster

            auto get_score = [&](uint32_t m) {
                const auto &monster = test_data.monsters[m];
                int64_t score = 0;
                score -= get_dist(cur_monster.x, cur_monster.y, monster.x, monster.y);
                score += monster.gold;
                return score;
            };

            uint32_t best_m = new_answer.monsters_order[k + 1];
            int64_t best_score = get_score(best_m);
            for (uint32_t i = k + 1; i < new_answer.monsters_order.size(); i++) {

                int64_t score = get_score(new_answer.monsters_order[i]);
                if (best_score < score) {
                    best_score = score;
                    best_m = new_answer.monsters_order[i];
                }
            }

            new_answer.monsters_order.erase(std::find(new_answer.monsters_order.begin(), new_answer.monsters_order.end(), best_m));
            new_answer.monsters_order.insert(new_answer.monsters_order.begin() + k + 1, best_m);
            k++;
        }
    } else {
        uint32_t old_num_turns = test_data.num_turns;
        test_data.num_turns = rnd.get(0, test_data.num_turns);
        new_answer = simulate(new_answer, test_data);
        test_data.num_turns = old_num_turns;

        uint32_t k = new_answer.last_monster_i;
        std::vector<std::pair<uint32_t, uint32_t>> available_monsters;
        for (uint32_t i = k + 1; i < new_answer.monsters_order.size(); i++) {
            uint32_t m = new_answer.monsters_order[i];
            auto &monster = test_data.monsters[m];
            available_monsters.emplace_back(get_dist(new_answer.x, new_answer.y, monster.x, monster.y), m);
        }

        std::sort(available_monsters.begin(), available_monsters.end(), std::greater<>());
        while (!available_monsters.empty()) {
            auto [dist, m] = available_monsters.back();
            available_monsters.pop_back();

            if (rnd.get_d() < 0.5) {
                continue;
            }

            new_answer.monsters_order.erase(std::find(new_answer.monsters_order.begin(), new_answer.monsters_order.end(), m));
            new_answer.monsters_order.insert(new_answer.monsters_order.begin() + k + 1, m);
            k++;

            if (rnd.get_d() < 0.1) {
                break;
            }
        }
    }

    new_answer = simulate(new_answer, test_data);

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

    new_answer = simulate(new_answer, test_data);

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
    uint32_t r = 0;
    if (rnd.get_d() < 0.5) {
        r = rnd.get(0, new_answer.monsters_order.size() - 1);
    } else {
        r = std::min(l + (uint32_t) rnd.get(3, 30), (uint32_t) new_answer.monsters_order.size() - 1);
    }

    if (l > r) {
        std::swap(l, r);
    }
    if (l == r) {
        return false;
    }

    ASSERT(0 <= l && l < r && r < new_answer.monsters_order.size(), "invalid segment");

    std::reverse(new_answer.monsters_order.begin() + l, new_answer.monsters_order.begin() + r);

    new_answer = simulate(new_answer, test_data);

    if (compare(answer.score, new_answer.score, temp, rnd)) {
        answer = std::move(new_answer);
        return true;
    } else {
        return false;
    }
}

bool Solver::try_change_settings(Randomizer &rnd) {
    Answer new_answer = answer;

    double p = rnd.get_d();

    if (false && p < 0.4) {
        new_answer.window_len = rnd.get(1, 15);
    } else if (p < 0.8) {
        new_answer.fatigue_weight = rnd.get(1, 30'000);
    } else {
        new_answer.enable_stop = rnd.get(0, 1);
    }

    new_answer = simulate(new_answer, test_data);

    if (compare(answer.score, new_answer.score, temp, rnd)) {
        answer = std::move(new_answer);
        return true;
    } else {
        return false;
    }
}

Solver::Solver(Answer copy_answer, TestData copy_test_data) : answer(std::move(copy_answer)), test_data(std::move(copy_test_data)) {
    ASSERT(!test_data.monsters.empty(), "monsters is empty");
}

Solver::Solver(std::vector<uint32_t> copy_monsters_order, TestData copy_test_data) : test_data(std::move(copy_test_data)) {
    ASSERT(!test_data.monsters.empty(), "monsters is empty");
    answer.monsters_order = std::move(copy_monsters_order);
    answer = simulate(answer, test_data);
}

Solver::Solver(TestData copy_test_data) : test_data(std::move(copy_test_data)) {
    ASSERT(!test_data.monsters.empty(), "monsters is empty");
    answer.monsters_order = test_data.monsters_order;
    answer = simulate(answer, test_data);
}

Answer Solver::solve(uint64_t random_seed) {
    Randomizer rnd(random_seed);

    Timer timer;

    Answer best_answer = answer;

    double temp_raw = 0.3;
    double temp_mult = 1;
    //uint32_t cnt_failed_improve = 0;

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
    //
    // gold: 693000, score: 693000, step: 2000000, time: 121.553s, temp: 0.000125784
    // gold: 646023, score: 646023, step: 500000, time: 122.553s, max_temp: 0.299328, temp_raw: 0.00673771, temp_mult: 15.1948
    // gold: 674002, score: 674002, step: 500000, time: 119.056s, max_temp: 0.0887038, temp_raw: 0.00673771, temp_mult: 13.1717
    // gold: 673002, score: 673002, step: 2000000, time: 142.103s, temp: 0.3
    // ========================================================================================
    // test: 21
    // gold: 402238, score: 402238, step: 2000000, time: 82.9996s, temp: 1.0288e-05, K=1
    // gold: 447182, score: 447182, step: 2000000, time: 107.967s, temp: 3.70016e-05, K=2
    // gold: 484134, score: 484134, step: 2000000, time: 133.054s, temp: 1.5303e-05, K=3
    // gold: 489119, score: 489119, step: 2000000, time: 181.425s, temp: 1.01856e-05, K=4
    //
    //
    // gold: 484134, score: 484134, step: 2000000, time: 132.86s, temp: 1.5303e-05
    // ========================================================================================
    // test: 25
    // gold: 16548, score: 16548, step: 2000000, time: 15.1092s, temp: 2.62239e-05
    // gold: 16786, score: 16786, step: 2000000, time: 14.5175s, temp: 2.62239e-05
    // gold: 16786, score: 16786, step: 2000000, time: 18.4639s, temp: 2.62239e-05
    // gold: 16841, score: 16841, step: 2000000, time: 105.821s, temp: 3.39805e-05
    // gold: 16926, score: 16926, step: 2000000, time: 104.715s, temp: 0.0858919
    // ========================================================================================
    // test: 28
    // gold: 40840, score: 40840, step: 2000000, time: 262.758s, temp: 1.20002e-05
    // gold: 38272, score: 38272, step: 108000, time: 60.3998s, temp: 7.19768e-05
    uint32_t step = 0;
    double max_temp = 0;
    //uint32_t prev_step_updated = 0;
    for (;
         //step <= 100'000
         ; step++) {
        if (step % 10 == 0 && timer.get_ms() > 10'000) {
            break;
        }

        temp = temp_raw * temp_mult;
        max_temp = std::max(temp, max_temp);
        //double old_score = answer.score;

        double p = rnd.get_d();

        if (p < 0.1) {
            try_change_settings(rnd);
        } else if (p < 0.6) {
            try_insert_smart(rnd);
        } else if (p < 0.8) {
            try_insert(rnd);
        } else if (p < 0.9) {
            try_swap(rnd);
        } else {
            try_reverse(rnd);
        }

        if (answer.score > best_answer.score) {
            best_answer = answer;
            //prev_step_updated = step;
        }

        temp_raw = std::max(temp_raw * 0.99999, 0.15);

        if (step % 1'000 == 0) {
            //std::cout << "gold: " << answer.gold << ", score: " << answer.score << ", step: " << step << ", time: " << timer << ", max_temp: " << max_temp << ", temp_raw: " << temp_raw << ", temp_mult: " << temp_mult << '\n';
            max_temp = 0;
        }

        //if(step - prev_step_updated > 10'000){
        //    break;
        //}
    }
    //std::cout << "best:\n";
    //std::cout << "gold: " << best_answer.gold << ", score: " << best_answer.score << ", step: " << step - 1 << ", time: " << timer << ", temp: " << temp << '\n';
    return best_answer;
}
