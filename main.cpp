#include <Objects/Basic/randomizer.hpp>
#include <Objects/Basic/time.hpp>
#include <Objects/Complex/answer.hpp>
#include <Objects/Complex/solver.hpp>
#include <Objects/Complex/test_data.hpp>
#include <Objects/Complex/test_solver.hpp>
#include <Objects/Tools/tools.hpp>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <set>

const std::vector<uint32_t> MAX_RAW_SCORES = {
        0,
        700,     // 1
        2665,    // 2
        245608,  // 3
        129770,  // 4
        6706160, // 5
        12595524,// 6
        2874331, // 7
        366423,  // 8
        482073,  // 9
        1869111, // 10
        1740182, // 11
        3050,    // 12
        3732,    // 13
        794646,  // 14
        2364185, // 15
        1129413, // 16
        18330001,// 17
        1140206, // 18
        1027001, // 19
        830000,  // 20
        703000,  // 21
        29824,   // 22
        47579,   // 23
        22501,   // 24
        19447,   // 25
        1174,    // 26
        3976,    // 27
        132001,  // 28
        106357,  // 29
        14881104,// 30
        10134289,// 31
        372587,  // 32
        368591,  // 33
        2679006, // 34
        2714850, // 35
        1718213, // 36
        2709867, // 37
        196138,  // 38
        209491,  // 39
        116093,  // 40
        3671,    // 41
        4176,    // 42
        8065,    // 43
        6476,    // 44
        14089,   // 45
        25116,   // 46
        45213,   // 47
        21559,   // 48
        18647,   // 49
        142221,  // 50
};

void run_solver(const std::string &dirname) {

    std::filesystem::create_directories(dirname);

    std::vector<uint32_t> tests = {

            //1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
            26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50};

    std::ofstream logger("log.csv");

    logger << "message,changes,thr,test,gold,solve time,total time" << std::endl;

    Timer timer;
    std::vector<TestData> tests_data(51);
    for (uint32_t test = 1; test <= 50; test++) {
        Timer timer;
        std::ifstream input("Tests/test_" + std::to_string(test) + ".json");
        input >> tests_data[test];
        std::cout << "reading tests data(" << test << "): " << timer << std::endl;
    }
    std::cout << "Total reading tests data: " << timer << std::endl;

    std::vector<std::atomic<bool>> is_free(51);
    for (auto &i: is_free) {
        i = true;
    }

    Timer total_timer;

    std::mutex mutex;

    auto lock = [&](uint32_t test) {
        while (true) {
            bool expected = true;
            if (is_free[test].compare_exchange_strong(expected, false)) {
                break;
            }
        }
    };

    auto unlock = [&](uint32_t test) {
        is_free[test] = true;
    };

    launch_threads(THREADS, [&](uint32_t thr) {
        Randomizer rnd(thr * 6124231 + RANDOM_SEED);
        while (!std::filesystem::exists("exit")) {
            uint32_t test = rnd.get(tests);

            Timer timer;

            std::string filename = dirname + "/test_" + std::to_string(test) + ".json";

            std::vector<uint32_t> monsters_order;
            uint32_t old_gold = 0;
            if (std::filesystem::exists(filename) && rnd.get_d() < 0.5) {
                Answer old_answer;
                lock(test);
                std::ifstream input(filename);
                input >> old_answer;
                unlock(test);
                old_gold = old_answer.gold;

                monsters_order = old_answer.monsters_order;

                // случайно пошафлим подотрезок монстров
                uint32_t k = rnd.get(0, 1);
                for (uint32_t i = 0; i < k; i++) {
                    uint32_t l = rnd.get(0, monsters_order.size() - 1);
                    uint32_t r = rnd.get(0, monsters_order.size() - 1);

                    if (l > r) {
                        std::swap(l, r);
                    }

                    std::shuffle(monsters_order.begin() + l, monsters_order.begin() + r, rnd.generator);
                }

            } else {
                monsters_order = tests_data[test].monsters_order;
                std::shuffle(monsters_order.begin(), monsters_order.end(), rnd.generator);
            }

            Solver solver(monsters_order, tests_data[test]);
            Answer answer = solver.solve(rnd.get());

            lock(test);

            auto is_improve = [&]() {
                if (!std::filesystem::exists(filename)) {
                    return true;
                }
                Answer old_answer;
                std::ifstream input(filename);
                input >> old_answer;
                return answer.gold > old_answer.gold;
            };

            if (is_improve()) {
                std::ofstream output(filename);
                output << answer;

                std::unique_lock locker(mutex);
                logger << "improve," << old_gold * 1000ULL / MAX_RAW_SCORES[test] << " -> " << answer.gold * 1000ULL / MAX_RAW_SCORES[test] << "," << thr << ',' << test << ',' << answer.gold << ',' << timer.get_ms() / 1000.0 << ',' << total_timer.get_ms() / 1000.0 << std::endl;
            } else {
                std::unique_lock locker(mutex);
                logger << "failed," << old_gold * 1000ULL / MAX_RAW_SCORES[test] << " -> " << answer.gold * 1000ULL / MAX_RAW_SCORES[test] << "," << thr << ',' << test << ',' << answer.gold << ',' << timer.get_ms() / 1000.0 << ',' << total_timer.get_ms() / 1000.0 << std::endl;
            }

            unlock(test);
        }
    });
}

void run_test_solver(const std::string &dirname) {

    std::filesystem::create_directories(dirname);

    std::vector<uint32_t> tests = {
            1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
            26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50};

    std::ofstream logger("log.csv");

    logger << "message,changes,thr,test,gold,solve time,total time" << std::endl;

    Timer timer;
    std::vector<TestSolver> test_solvers(51);
    {
        std::vector<TestData> tests_data(51);
        for (uint32_t test = 1; test <= 50; test++) {
            Timer timer;
            std::ifstream input("Tests/test_" + std::to_string(test) + ".json");
            input >> tests_data[test];
            std::cout << "reading tests data(" << test << "): " << timer << std::endl;
        }
        std::cout << "Total reading tests data: " << timer << std::endl;

        for (uint32_t test = 1; test <= 50; test++) {
            test_solvers[test] = TestSolver(tests_data[test], dirname + "/" + std::to_string(test));
        }
    }

    std::vector<std::atomic<bool>> is_free(51);
    for (auto &i: is_free) {
        i = true;
    }

    Timer total_timer;

    std::mutex mutex;

    auto lock = [&](uint32_t test) {
        bool expected = true;
        if (is_free[test].compare_exchange_strong(expected, false)) {
            return true;
        } else {
            return false;
        }
    };

    auto unlock = [&](uint32_t test) {
        is_free[test] = true;
    };

    launch_threads(THREADS, [&](uint32_t thr) {
        Randomizer rnd(thr * 6124231ULL + RANDOM_SEED);
        while (!std::filesystem::exists("exit")) {
            uint32_t test = rnd.get(tests);

            if (!lock(test)) {
                continue;
            }

            Timer timer;

            std::string filename = dirname + "/test_" + std::to_string(test) + ".json";

            auto &solver = test_solvers[test];

            auto old_answer = solver.get_best();

            if (rnd.get_d() < 0.5) {
                solver.add(rnd);
            } else {
                solver.improve(rnd);
            }

            auto cur_answer = solver.get_best();

            if (old_answer.gold < cur_answer.gold) {
                std::ofstream output(filename);
                output << cur_answer;

                std::unique_lock locker(mutex);
                logger << "improve," << old_answer.gold * 1000ULL / MAX_RAW_SCORES[test] << " -> " << cur_answer.gold * 1000ULL / MAX_RAW_SCORES[test] << "," << thr << ',' << test << ',' << cur_answer.gold << ',' << timer.get_ms() / 1000.0 << ',' << total_timer.get_ms() / 1000.0 << std::endl;
            } else {
                std::unique_lock locker(mutex);
                logger << "failed," << old_answer.gold * 1000ULL / MAX_RAW_SCORES[test] << " -> " << cur_answer.gold * 1000ULL / MAX_RAW_SCORES[test] << "," << thr << ',' << test << ',' << cur_answer.gold << ',' << timer.get_ms() / 1000.0 << ',' << total_timer.get_ms() / 1000.0 << std::endl;
            }

            unlock(test);
        }
    });
}

void print_compare_simulates(const std::string &solutions_dir, uint32_t left_test, uint32_t right_test) {
    double total_p = 0;
    for (uint32_t test = left_test; test <= right_test; test++) {
        TestData test_data;
        {
            std::ifstream input("Tests/test_" + std::to_string(test) + ".json");
            input >> test_data;
        }

        Answer answer;
        {
            std::ifstream input(solutions_dir + "/test_" + std::to_string(test) + ".json");
            input >> answer;
        }

        Answer new_answer = simulate(answer, test_data);

        double p = ((int) new_answer.gold - (int) answer.gold) * 100.0 / answer.gold;
        total_p += p;
        std::cout << test << ": " << answer.gold << " -> " << new_answer.gold << " " << p << "%" << std::endl;
    }
    // Total p: -140.589% -> -46.8513%
    std::cout << "Total p: " << total_p << "%" << std::endl;
}

void print_compare_scores(const std::string &solutions_dir, uint32_t left_test, uint32_t right_test) {
    double total = 0;
    for (uint32_t test = left_test; test <= right_test; test++) {
        TestData test_data;
        {
            std::ifstream input("Tests/test_" + std::to_string(test) + ".json");
            input >> test_data;
        }

        Answer answer;
        {
            std::ifstream input(solutions_dir + "/test_" + std::to_string(test) + ".json");
            if (input) {
                input >> answer;
            }
        }

        //Answer expected = simulate(answer.monsters_order, test_data);
        //ASSERT(answer.gold == expected.gold, "invalid answer");

        double score = answer.gold * 1000.0 / MAX_RAW_SCORES[test];
        total += score;
        std::cout << test << ": " << answer.gold << "/" << MAX_RAW_SCORES[test] << " " << score << std::endl;
    }
    std::cout << "Total: " << total << std::endl;
}

void launch_tests(const std::string &solutions_dir, uint32_t left_test, uint32_t right_test) {

    std::filesystem::create_directory(solutions_dir);

    Timer total_timer;

    std::vector<TestData> tests_data(51);
    for (uint32_t test = left_test; test <= right_test; test++) {
        Timer timer;
        std::ifstream input("Tests/test_" + std::to_string(test) + ".json");
        input >> tests_data[test];
        std::cout << "reading tests data(" << test << "): " << timer << std::endl;
    }
    std::cout << "Total reading tests data: " << total_timer << std::endl;

    std::vector<std::atomic<bool>> is_free(51);
    for (auto &i: is_free) {
        i = true;
    }

    std::ofstream logger("log.csv");

    logger << "thr,test,gold,relative score,solve time,total time" << std::endl;

    std::mutex mutex;

    uint64_t total_relative_score = 0;
    uint64_t total_gold = 0;

    launch_threads(THREADS, [&](uint32_t thr) {
        for (uint32_t test = left_test; test <= right_test; test++) {
            bool expected = true;
            if (!is_free[test].compare_exchange_strong(expected, false)) {
                continue;// уже занят
            }

            Timer timer;

            std::string filename = solutions_dir + "/test_" + std::to_string(test) + ".json";

            Solver solver(tests_data[test]);
            Answer answer = solver.solve(0);

            {
                std::ofstream output(filename);
                ASSERT(output, "unable to open file for writing");
                output << answer;
            }

            std::unique_lock locker(mutex);
            uint64_t relative_score = answer.gold * 1000.0 / MAX_RAW_SCORES[test];
            total_relative_score += relative_score;
            total_gold += answer.gold;
            logger << thr << ',' << test << ',' << answer.gold << ',' << relative_score << ',' << timer.get_ms() / 1000.0 << ',' << total_timer.get_ms() / 1000.0 << std::endl;
        }
    });

    logger << "-1" << ',' << "0" << ',' << total_gold << ',' << total_relative_score << ',' << total_timer.get_ms() / 1000.0 << ',' << total_timer.get_ms() / 1000.0 << std::endl;
    std::cout << total_relative_score << std::endl;
}

int main() {

    //run_test_solver("Solutions4");
    //run_solver("Solutions3");

    // в ответах у меня: 16680
    // сейчас получаю: 8123 -> 9114 -> 9929 -> 10256 -> 10573 -> 11263 -> 11780 -> 12812 -> 13731
    // подольше запустить: 15320.5
    // launch_tests("Solutions3", 26, 50);

    // Total: 23387.1 + 19669.9 (Solutions4)
    // Total: 23463.9 + 20042.1 (Solutions_nice)
    //print_compare_scores("Solutions4", 1, 25);

    // print_compare_simulates("Solutions3", 1, 25);
    // return 0;

    /*uint32_t test = 48;
    TestData test_data;
    {
        std::ifstream input("Tests/test_" + std::to_string(test) + ".json");
        input >> test_data;
    }*/

    /*TestSolver test_solver(test_data, "Solutions3");
    Randomizer rnd;
    std::ofstream output("log");
    while (true) {
        if (rnd.get_d() < 0.5) {
            test_solver.add(rnd);
        } else {
            test_solver.improve(rnd);
        }
        test_solver.print_state(output);

        //test_solver.write("Solutions3");
    }*/

    /*Answer from_ans;
    {
        std::ifstream input("Solutions4/test_" + std::to_string(test) + ".json");
        input >> from_ans;
    }

    Randomizer rnd(42);
    uint32_t best = 0;
    // 15710 ->
    while (true) {
        std::shuffle(from_ans.monsters_order.begin(), from_ans.monsters_order.end(), rnd.generator);
        Solver solver(from_ans.monsters_order, test_data);
        Answer answer = solver.solve(rnd.get());
        best = std::max(best, answer.gold);
        std::cout << "best: " << best << std::endl;
    }*/
    //std::ofstream output("test_" + std::to_string(test) + ".json");
    //output << answer;
}
