#include <Objects/Complex/solver.hpp>

#include <Objects/Basic/assert.hpp>
#include <Objects/Basic/time.hpp>

#include <set>
#include <tuple>

bool compare(double old_score, double cur_score, double temp, Randomizer &rnd) {
    return cur_score >= old_score || rnd.get_d() < std::exp(-((old_score - cur_score) / old_score) / (temp * 0.001));
}

Answer simulate(const std::vector<uint32_t> &monsters_order, uint64_t random_seed, const TestData &test_data) {

    Answer answer;
    answer.x = test_data.start_x;
    answer.y = test_data.start_y;
    answer.monsters_order = monsters_order;

    Randomizer rnd(random_seed);

    std::vector<bool> is_killed(test_data.monsters.size());

    ASSERT(test_data.monsters_order.size() == monsters_order.size(), "invalid monsters order");

    for (uint32_t monster_it = 0; monster_it < monsters_order.size(); monster_it++) {
        uint32_t monster_id = monsters_order[monster_it];
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
            ASSERT(test_data.monsters_attack[pos].empty(), "is not empty");
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

                uint32_t px = 0;
                uint32_t py = 0;

                double dx = static_cast<int>(monster.x) - static_cast<int>(answer.x);
                double dy = static_cast<int>(monster.y) - static_cast<int>(answer.y);
                double len = std::sqrt(dx * dx + dy * dy);
                dx /= len;
                dy /= len;
                len = std::min(speed * 1.0 - 1, len - 1);
                px = answer.x + dx * len;
                py = answer.y + dy * len;

                uint32_t left_x = px < answer.window_len ? 0 : px - answer.window_len;
                uint32_t right_x = std::min(px + answer.window_len, test_data.width);

                uint32_t left_y = py < answer.window_len ? 0 : py - answer.window_len;
                uint32_t right_y = std::min(py + answer.window_len, test_data.height);

                auto get_dot_score = [&](uint32_t to_x, uint32_t to_y) {
                    // 22008
                    int64_t score;
                    score -= get_dist(to_x, to_y, monster.x, monster.y);

                    // 22217
                    if (get_dist(to_x, to_y, monster.x, monster.y) <= range * range) {
                        score = 1'000'000;

                        uint32_t it = monster_it + 1;
                        if (it < answer.monsters_order.size()) {
                            const auto &monster = test_data.monsters[answer.monsters_order[it]];
                            score -= get_dist(to_x, to_y, monster.x, monster.y);
                        }

                        /*for (uint32_t it = monster_it; it < answer.monsters_order.size(); it++) {
                            const auto &monster = test_data.monsters[answer.monsters_order[it]];
                            if (get_dist(to_x, to_y, monster.x, monster.y) > range * range) {
                                score -= get_dist(to_x, to_y, monster.x, monster.y);
                                break;
                            }
                            score += 100'000;
                        }*/
                    }
                    return score;
                };

                int64_t best_score = -1e18;

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
                // умный выбор точки, но медленный
                /*std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> dots;

                    for (uint32_t to_y = left_y; to_y <= right_y; to_y++) {
                        for (uint32_t to_x = left_x; to_x <= right_x; to_x++) {
                            // не можем допрыгнуть туда
                            if (get_dist(answer.x, answer.y, to_x, to_y) > speed * speed) {
                                continue;
                            }

                            uint32_t dist = get_dist(to_x, to_y, monster.x, monster.y);
                            dots.emplace_back(dist, to_x, to_y);
                        }
                    }

                    {
                        uint32_t monster_iter = monster_it;
                        while (monster_iter < monsters_order.size()) {
                            const auto &cur_monster = test_data.monsters[monsters_order[monster_iter]];
                            for (auto &[dist, to_x, to_y]: dots) {
                                dist = get_dist(to_x, to_y, cur_monster.x, cur_monster.y);
                            }
                            std::sort(dots.begin(), dots.end());

                            if (std::get<0>(dots[0]) > range * range) {
                                break;
                            }

                            while (std::get<0>(dots.back()) > range * range) {
                                dots.pop_back();
                            }
                            monster_iter++;
                        }

                        std::tie(best_dist, best_to_x, best_to_y) = dots[0];
                    }*/

                return {best_to_x, best_to_y};
            };

            auto [to_x, to_y] = get_move_to();
            ASSERT(get_dist(answer.x, answer.y, to_x, to_y) <= speed * speed, "too far to move");
            ASSERT(!(to_x == answer.x && to_y == answer.y), "invalid to");

            answer.actions.push_back({Action::Action_t::MOVE, to_x, to_y, 0});
            answer.x = to_x;
            answer.y = to_y;
            update_fatigue(1);
        }

        uint32_t total_damage = 0;

        // побьем монстра
        while (answer.actions.size() < test_data.num_turns && total_damage < monster.hp) {
            ASSERT(get_dist(answer.x, answer.y, monster.x, monster.y) <= range * range, "failed to attack");

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

/*Answer simulate(const std::vector<uint32_t> &monsters_order, uint64_t random_seed, const TestData &test_data) {

    Answer answer;
    answer.x = test_data.start_x;
    answer.y = test_data.start_y;
    answer.monsters_order = monsters_order;

    Randomizer rnd(random_seed);

    std::vector<bool> is_killed(monsters_order.size());

    uint64_t dist_w = rnd.get(1, 100000);

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

        if ((monster.hp + power - 1) / power > 20) {
            continue;// слишком жирный
        }

        //answer.score -= answer.fatigue * 5.0 / test_data.num_turns * (1 - answer.actions.size() * 1.0 / test_data.num_turns);

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
            auto get_move_to_smart = [&]() -> std::pair<uint32_t, uint32_t> {
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

                constexpr uint32_t K = 1;

                uint32_t left_x = px < K ? 0 : px - K;
                uint32_t right_x = std::min(px + K, test_data.width);

                uint32_t left_y = py < K ? 0 : py - K;
                uint32_t right_y = std::min(py + K, test_data.height);

                std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> dots;

                for (uint32_t to_y = left_y; to_y <= right_y; to_y++) {
                    for (uint32_t to_x = left_x; to_x <= right_x; to_x++) {
                        // не можем допрыгнуть туда
                        if (get_dist(answer.x, answer.y, to_x, to_y) > speed * speed) {
                            continue;
                        }

                        best_to_x = to_x;
                        best_to_y = to_y;
                    }
                }

                ASSERT(!(best_to_x == answer.x && best_to_y == answer.y), "unable to find to pos");

                // взяли неплохую точку
                // но давайте улучшим ее

                auto get_metric = [&](uint32_t to_x, uint32_t to_y) {
                    uint64_t fatigue = 0;
                    {
                        uint32_t pos = to_y * (test_data.width + 1) + to_x;
                        for (uint32_t m: test_data.monsters_attack[pos]) {
                            if (!is_killed[m]) {
                                fatigue += test_data.monsters[m].attack;
                            }
                        }
                    }
                    int64_t metric = 0;
                    if (get_dist(to_x, to_y, monster.x, monster.y) > range * range) {
                        metric += get_dist(to_x, to_y, monster.x, monster.y) * dist_w;
                    }
                    metric += fatigue;
                    return metric;
                };

                auto best_metric = get_metric(best_to_x, best_to_y);

                // (metric, to_x, to_y)
                std::set<std::tuple<int64_t, uint32_t, uint32_t>> S;
                S.insert({best_metric, best_to_x, best_to_y});

                std::set<std::tuple<uint32_t, uint32_t>> visited;
                visited.insert({best_to_x, best_to_y});

                for (uint32_t step = 0; !S.empty(); step++) {
                    auto [metric, to_x, to_y] = *S.begin();
                    S.erase(S.begin());

                    if (metric < best_metric && !(to_x == answer.x && to_y == answer.y)) {
                        best_metric = metric;
                        best_to_x = to_x;
                        best_to_y = to_y;
                    }

                    if (step > 30) {
                        continue;
                    }

                    auto do_step = [&](int32_t dx, int32_t dy) {
                        int32_t x = dx + static_cast<int32_t>(to_x);
                        int32_t y = dy + static_cast<int32_t>(to_y);

                        if (x < 0 || y < 0) {
                            return;
                        }
                        if (test_data.width < x || test_data.height < y) {
                            return;
                        }
                        // не можем допрыгнуть туда
                        if (get_dist(answer.x, answer.y, x, y) > speed * speed) {
                            return;
                        }
                        if (visited.count({x, y})) {
                            return;
                        }

                        visited.insert({x, y});
                        S.insert({get_metric(x, y), x, y});
                    };

                    do_step(0, +1);
                    do_step(0, -1);
                    do_step(+1, 0);
                    do_step(-1, 0);
                }
                return {best_to_x, best_to_y};
            };

            auto get_move_to = [&]() -> std::pair<uint32_t, uint32_t> {
                return get_move_to_smart();

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

                constexpr uint32_t K = 3;

                uint32_t left_x = px < K ? 0 : px - K;
                uint32_t right_x = std::min(px + K, test_data.width);

                uint32_t left_y = py < K ? 0 : py - K;
                uint32_t right_y = std::min(py + K, test_data.height);

                std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> dots;

                for (uint32_t to_y = left_y; to_y <= right_y; to_y++) {
                    for (uint32_t to_x = left_x; to_x <= right_x; to_x++) {
                        // не можем допрыгнуть туда
                        if (get_dist(answer.x, answer.y, to_x, to_y) > speed * speed) {
                            continue;
                        }

                        uint32_t metric = get_dist(to_x, to_y, monster.x, monster.y);// + fatigue / 40;
                        dots.push_back({metric, to_x, to_y});
                    }
                }
                std::sort(dots.begin(), dots.end());
                for (auto [metric, to_x, to_y]: dots) {
                    best_to_x = to_x;
                    best_to_y = to_y;
                    if (rnd.get_d() < 0.5) {
                        break;
                    }
                }

                ASSERT(!(best_to_x == answer.x && best_to_y == answer.y), "unable to find to pos");
                return {best_to_x, best_to_y};
            };

            auto [to_x, to_y] = get_move_to();
            ASSERT(get_dist(answer.x, answer.y, to_x, to_y) <= speed * speed, "too far to move");
            ASSERT(!(to_x == answer.x && to_y == answer.y), "invalid to");

            answer.actions.push_back({Action::Action_t::MOVE, to_x, to_y, 0});
            answer.x = to_x;
            answer.y = to_y;
            update_fatigue(1);
        }

        uint32_t total_damage = 0;

        // побьем монстра
        while (answer.actions.size() < test_data.num_turns && total_damage < monster.hp) {
            ASSERT(get_dist(answer.x, answer.y, monster.x, monster.y) <= range * range, "failed to attack");

            answer.actions.push_back({Action::Action_t::ATTACK, 0, 0, monster_id});
            total_damage += power;
        }

        update_fatigue(total_damage / power);

        // убили монстра
        if (total_damage >= monster.hp) {
            is_killed[monster_id] = true;

            if (get_dist(answer.x, answer.y, monster.x, monster.y) <= monster.range * monster.range) {
                answer.fatigue -= monster.attack;
            }

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
}*/

bool Solver::try_swap(Randomizer &rnd) {
    uint32_t a = rnd.get(0, answer.last_monster_i);
    uint32_t b = a + rnd.get(-10, 10);
    if (a == b || b >= answer.monsters_order.size()) {
        return false;
    }

    Answer new_answer = answer;
    std::swap(new_answer.monsters_order[a], new_answer.monsters_order[b]);
    new_answer = simulate(new_answer.monsters_order, new_answer.random_seed, test_data);

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

    new_answer = simulate(new_answer.monsters_order, new_answer.random_seed, test_data);

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

    new_answer = simulate(new_answer.monsters_order, new_answer.random_seed, test_data);

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
        uint32_t k = rnd.get(0, new_answer.last_monster_i);

        for (uint32_t z = rnd.get(1, 5); z > 0 && k + 1 < new_answer.monsters_order.size(); z--) {
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
    }

    new_answer = simulate(new_answer.monsters_order, new_answer.random_seed, test_data);

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

    new_answer = simulate(new_answer.monsters_order, new_answer.random_seed, test_data);

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
    uint32_t r = rnd.get(0, new_answer.monsters_order.size() - 1);//uint32_t r = std::min(l + (uint32_t) rnd.get(3, 30), (uint32_t) new_answer.monsters_order.size() - 1);

    if (l > r) {
        std::swap(l, r);
    }
    if (l == r) {
        return false;
    }

    ASSERT(0 <= l && l < r && r < new_answer.monsters_order.size(), "invalid segment");

    std::reverse(new_answer.monsters_order.begin() + l, new_answer.monsters_order.begin() + r);
    //std::shuffle(new_answer.monsters_order.begin() + l, new_answer.monsters_order.begin() + r, rnd.generator);

    new_answer = simulate(new_answer.monsters_order, new_answer.random_seed, test_data);

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

    if (p < 5) {
        //new_answer.random_seed = rnd.get();
        new_answer.window_len = rnd.get(1, 6);
    } else {
        // TODO
    }

    new_answer = simulate(new_answer.monsters_order, new_answer.random_seed, test_data);

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
    answer = simulate(answer.monsters_order, answer.random_seed, test_data);
}

Solver::Solver(TestData copy_test_data) : test_data(std::move(copy_test_data)) {
    ASSERT(!test_data.monsters.empty(), "monsters is empty");
    answer.monsters_order = test_data.monsters_order;
    //answer.monsters_order.resize(test_data.monsters.size());
    //std::iota(answer.monsters_order.begin(), answer.monsters_order.end(), 0);
    answer = simulate(answer.monsters_order, answer.random_seed, test_data);
}

Answer Solver::solve(uint64_t random_seed) {
    Randomizer rnd(random_seed);

    Timer timer;

    Answer best_answer = answer;

    double raw_temp = 0.2;
    double temp_mult = 1;

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
    // gold: 646023, score: 646023, step: 500000, time: 122.553s, max_temp: 0.299328, raw_temp: 0.00673771, temp_mult: 15.1948
    // gold: 674002, score: 674002, step: 500000, time: 119.056s, max_temp: 0.0887038, raw_temp: 0.00673771, temp_mult: 13.1717
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
    for (;
         //step <= 2'000'000
         ; step++) {
        if (step % 10 == 0 && timer.get_ms() > 120'000) {
            break;
        }

        temp = raw_temp * temp_mult;
        max_temp = std::max(temp, max_temp);
        double old_score = answer.score;

        double p = rnd.get_d();
        // try_swap(rnd); // 319002
        // try_insert(rnd); // 393031
        // try_insert_smart(rnd); // 518113
        // try_reverse(rnd);// 263057


        // ~20200
        /*if (p < 0.5) {
            try_insert_smart(rnd);
        } else if (p < 0.8) {
            try_insert(rnd);
        } else {
            try_reverse(rnd);
        }*/

        // 16155
        // try_insert_smart(rnd);

        // 16556
        // try_insert(rnd);

        // 20639 -> 21305
        /*if (p < 0.5) {
            try_insert_smart(rnd);
        } else if (p < 0.8) {
            try_insert(rnd);
        } else {
            try_swap(rnd);
        }*/

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

        /*
        if (p < 0.33) {
            try_swap(rnd);
        } else if (p < 0.66) {
            try_move(rnd);
        } else {
            try_insert(rnd);
        }
         */

        if (answer.score > best_answer.score) {
            best_answer = answer;
        }

        raw_temp = std::max(raw_temp * 0.99999, 0.00001);
        if (old_score < answer.score) {
            // improve
            temp_mult = (temp_mult + 1) / 2;
        } else {
            // failed
            temp_mult = std::min(temp_mult * 1.0005, 60'000.0);
        }

        if (step % 1'000 == 0) {
            //std::cout << "gold: " << answer.gold << ", score: " << answer.score << ", step: " << step << ", time: " << timer << ", max_temp: " << max_temp << ", raw_temp: " << raw_temp << ", temp_mult: " << temp_mult << '\n';
            max_temp = 0;
        }
    }
    //std::cout << "best:\n";
    //std::cout << "gold: " << best_answer.gold << ", score: " << best_answer.score << ", step: " << step - 1 << ", time: " << timer << ", temp: " << temp << '\n';
    return best_answer;
}
