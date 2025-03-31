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

    std::vector<uint32_t> tests = {
            //1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25
            26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50
    };

    std::ofstream logger("log.csv");

    logger << "message,thr,test,gold,solve time,total time" << std::endl;

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

    auto do_work = [&](uint32_t thr) {
        Randomizer rnd(thr * 6124231 + RANDOM_SEED);
        while (!std::filesystem::exists("exit")) {
            uint32_t test = rnd.get(tests);

            ETimer timer;

            std::string filename = "Solutions/test_" + std::to_string(test) + ".json";

            std::vector<uint32_t> monsters_order;
            if (std::filesystem::exists(filename) && rnd.get_d() < 0.3) {
                Answer old_answer;
                lock(test);
                std::ifstream input(filename);
                input >> old_answer;
                unlock(test);

                monsters_order = old_answer.monsters_order;

                // случайно пореверсим порядок монстров
                if (rnd.get_d() < 0.3) {
                    uint32_t k = rnd.get(1, 5);
                    for (uint32_t i = 0; i < k; i++) {
                        uint32_t l = rnd.get(0, monsters_order.size() - 1);
                        uint32_t r = rnd.get(0, monsters_order.size() - 1);

                        if (l > r) {
                            std::swap(l, r);
                        }

                        std::reverse(monsters_order.begin() + l, monsters_order.begin() + r);
                    }
                }
            } else {
                monsters_order.resize(tests_data[test].monsters.size());
                std::iota(monsters_order.begin(), monsters_order.end(), 0);
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
                logger << "improve," << thr << ',' << test << ',' << answer.gold << ',' << timer.get_ms() / 1000.0 << ',' << total_timer.get_ms() / 1000.0 << std::endl;
            } else {
                std::unique_lock locker(mutex);
                logger << "failed," << thr << ',' << test << ',' << answer.gold << ',' << timer.get_ms() / 1000.0 << ',' << total_timer.get_ms() / 1000.0 << std::endl;
            }

            unlock(test);
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
    run_solver();
    return 0;

    uint32_t test = 28;
    TestData test_data;
    std::ifstream input("Tests/test_" + std::to_string(test) + ".json");
    input >> test_data;

    Solver solver(test_data);
    Answer answer = solver.solve(303);
    std::ofstream output("test_" + std::to_string(test) + ".json");
    output << answer;
}
