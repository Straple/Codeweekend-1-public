#include "Objects/answer.hpp"
#include "Objects/randomizer.hpp"
#include "Objects/test_data.hpp"
#include "Objects/timer.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <set>
#include <thread>

int main() {
    std::cout << "Randomizer example:" << std::endl;
    Randomizer rnd(42);
    std::cout << rnd.get() << std::endl;       // random uint64_t
    std::cout << rnd.get(-3, 5) << std::endl;  // random int64_t e [-3, 5]
    std::cout << rnd.get_d() << std::endl;     // random double e [0, 1]
    std::cout << rnd.get_d(-3, 5) << std::endl;// random double e [-3, 5]
    std::vector<int> my_vec = {0, 5, 10, 15};
    std::cout << rnd.get(my_vec) << std::endl;// random element from container
    const std::set<int> my_set = {3, -5, 10, 1};
    std::cout << rnd.get(my_set) << std::endl;// random element from container

    std::cout << "Timer example:" << std::endl;
    Timer timer;// get tick from constructor
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::cout << timer << std::endl;// beautiful output time
    timer.reset();                  // reset time tick
    std::string s = "a";
    for (int k = 0; k < 10; k++) {
        s += s;
    }
    std::cout << timer << std::endl;// and more

    std::cout << "TestData example:" << std::endl;
    TestData test_data;
    {// read TestData
        std::ifstream input("Tests/test_" + std::to_string(1));
        input >> test_data;
        // std::cout << test_data << std::endl; // can write TestData
    }

    std::cout << "Answer example:" << std::endl;
    Answer answer;
    {// read Answer
        std::ifstream input("Answers/test_" + std::to_string(1));
        input >> answer;
        // std::cout << answer << std::endl;// can write Answer
    }

    std::cout << "Read all tests example:" << std::endl;
    for (int test = 1; test <= 50; test++) {
        TestData data;
        std::ifstream input("Tests/test_" + std::to_string(test));
        input >> data;
    }
}