#include "Objects/answer.hpp"
#include "Objects/randomizer.hpp"
#include "Objects/test_data.hpp"
#include "Objects/timer.hpp"

#include <fstream>
#include <iostream>

int main() {

    TestData data;
    {// read TestData
        std::ifstream input("Tests/test_" + std::to_string(1));
        input >> data;
        std::cout << data << std::endl;
    }

}