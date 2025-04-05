#include <Objects/Basic/randomizer.hpp>
#include <Objects/Basic/time.hpp>
#include <Objects/Complex/answer.hpp>
#include <Objects/Complex/solver.hpp>
#include <Objects/Complex/test_data.hpp>
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

            1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
            //26, 27, 28, 29, 30, 31, 32, 33, 34, 35, /*36, 37,*/ 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50
    };

    std::ofstream logger("log.csv");

    logger << "message,thr,test,gold,solve time,total time" << std::endl;

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
                logger << "improve " << old_gold * 1000 / MAX_RAW_SCORES[test] << " -> " << answer.gold * 1000 / MAX_RAW_SCORES[test] << "," << thr << ',' << test << ',' << answer.gold << ',' << timer.get_ms() / 1000.0 << ',' << total_timer.get_ms() / 1000.0 << std::endl;
            } else {
                std::unique_lock locker(mutex);
                logger << "failed " << old_gold * 1000 / MAX_RAW_SCORES[test] << " -> " << answer.gold * 1000 / MAX_RAW_SCORES[test] << "," << thr << ',' << test << ',' << answer.gold << ',' << timer.get_ms() / 1000.0 << ',' << total_timer.get_ms() / 1000.0 << std::endl;
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

        Answer new_answer = simulate(answer.monsters_order, answer.random_seed, test_data);

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

    run_solver("Solutions");
    //return 0;

    // launch_tests("Solutions3", 1, 25);
    // print_compare_scores("Solutions_kek", 1, 25);

    // print_compare_simulates("Solutions3", 1, 25);
    return 0;

    uint32_t test = 20;
    TestData test_data;
    {
        std::ifstream input("Tests/test_" + std::to_string(test) + ".json");
        input >> test_data;
    }

    Solver solver(test_data);
    Answer answer = solver.solve(303);
    std::ofstream output("test_" + std::to_string(test) + ".json");
    output << answer;
}

/*
test relative_score raw_score

в Solution2 находятся решения сервера на 21 час в 32 ядра и в log2.csv находится их лог
1: 700/700 1000
2: 2580/2665 968.105
3: 226259/245608 921.22
4: 121444/129770 935.84
5: 6402021/6706160 954.648
6: 11825964/12595524 938.902
7: 2762177/2874331 960.981
8: 355863/366423 971.181
9: 458703/482073 951.522
10: 1763429/1869111 943.459
11: 1647422/1740182 946.695
12: 3020/3050 990.164
13: 3623/3732 970.793
14: 755149/794646 950.296
15: 2290431/2364185 968.804
16: 1112222/1129413 984.779
17: 16300041/18330001 889.255
18: 973115/1140206 853.455
19: 866023/1027001 843.254
20: 732000/830000 881.928
21: 545085/703000 775.37
22: 27365/29824 917.55
23: 42888/47579 901.406
24: 19588/22501 870.539
25: 17072/19447 877.873
Total: 23168
==========
1 1000 700
2 968 2580
3 931 228778
4 932 121023
5 945 6339591
6 942 11873862
7 962 2767091
8 962 352562
9 958 462098
10 942 1761854
11 927 1613561
12 976 2977
13 988 3689
14 908 722067
15 928 2195778
16 979 1106074
17 863 15830012
18 909 1037081
19 836 859005
20 889 738000
21 725 510013
22 905 27003
23 879 41824
24 831 18705
25 865 16839
26 990 1163
27 959 3816
28 773 102093
29 818 87002
30 201 2995570
31 764 7751180
32 268 100049
33 640 236260
34 38 102366
35 663 1800778
36 0 0
37 0 0
38 727 142663
39 42 8855
40 674 78356
41 800 2940
42 759 3172
43 578 4664
44 611 3960
45 646 9111
46 517 12988
47 672 30403
48 522 11262
49 877 16362
50 573 81542
total: 37083
*/