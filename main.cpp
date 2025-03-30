#include <Objects/Basic/randomizer.hpp>
#include <Objects/Basic/time.hpp>
#include <Objects/Complex/answer.hpp>
#include <Objects/Complex/solver.hpp>
#include <Objects/Complex/test_data.hpp>

#include <chrono>
#include <fstream>
#include <iostream>
#include <set>
#include <thread>

#include <atomic>
#include <filesystem>
#include <mutex>
#include <thread>

void run_solver() {

    std::filesystem::create_directories("Solutions");

    std::vector<uint32_t> tests = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25};

    std::ofstream logger("log.csv");

    logger << "thr,test,gold,solve time,total time\n";

    std::vector<TestData> tests_data(51);
    for (uint32_t test = 1; test <= 50; test++) {
        std::ifstream input("Tests/test_" + std::to_string(test) + ".json");
        input >> tests_data[test];
    }

    std::vector<std::atomic<bool>> is_free(51);
    for (auto &i: is_free) {
        i = true;
    }

    ETimer total_timer;

    std::mutex mutex;

    auto do_work = [&](uint32_t thr) {
        Randomizer rnd(thr * 6124231 + RANDOM_SEED);
        while (!std::filesystem::exists("exit")) {
            uint32_t test = rnd.get(tests);
            bool expected = true;
            if (!is_free[test].compare_exchange_strong(expected, false)) {
                continue;
            }

            ETimer timer;

            std::string filename = "Solutions/test_" + std::to_string(test) + ".json";

            std::vector<uint32_t> monsters_order;
            if (std::filesystem::exists(filename)) {
                Answer old_answer;
                std::ifstream input(filename);
                input >> old_answer;
                monsters_order = old_answer.monsters_order;
            } else {
                monsters_order.resize(tests_data[test].monsters.size());
                std::iota(monsters_order.begin(), monsters_order.end(), 0);
                std::shuffle(monsters_order.begin(), monsters_order.end(), rnd.generator);
            }

            Solver solver(monsters_order, tests_data[test]);
            Answer answer = solver.solve(rnd.get());

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
                logger << thr << ',' << test << ',' << answer.gold << ',' << timer.get_ms() / 1000.0 << ',' << total_timer.get_ms() / 1000.0 << std::endl;
            }

            is_free[test] = true;
        }
    };

    std::vector<std::thread> threads(THREADS);
    for (uint32_t thr = 0; thr < THREADS; thr++) {
        threads[thr] = std::thread(do_work, thr);
    }
    for (uint32_t thr = 0; thr < THREADS; thr++) {
        threads[thr].join();
    }
}

int main() {
    //run_solver();

    uint32_t test = 20;
    TestData test_data;
    std::ifstream input("Tests/test_" + std::to_string(test) + ".json");
    input >> test_data;

    Solver solver(test_data);
    Answer answer = solver.solve(303);
    std::ofstream output("Solutions/test_" + std::to_string(test) + ".json");
    output << answer;
}
