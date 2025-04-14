#include <Objects/Complex/test_solver.hpp>

#include <Objects/Complex/solver.hpp>

#include <filesystem>
#include <fstream>

void TestSolver::mow() {
    for (uint32_t i = 1; i < items.size(); i++) {
        if (items[i].cnt_failed_improve > 3) {
            ASSERT(i != 0, "remove best answer");
            std::cout << "remove: " << items[i].answer.gold << std::endl;
            items.erase(items.begin() + i);
            i--;
        }
    }
}

void TestSolver::bubble_sort(uint32_t i) {
    for (; i + 1 < items.size() && items[i].answer.gold < items[i + 1].answer.gold; i++) {
        std::swap(items[i], items[i + 1]);
    }
    for (; i > 0 && items[i - 1].answer.gold < items[i].answer.gold; i--) {
        std::swap(items[i - 1], items[i]);
    }
}

TestSolver::TestSolver(TestData init_test_data, std::string init_dirname) : test_data(std::move(init_test_data)), dirname(std::move(init_dirname)) {
    std::filesystem::create_directories(dirname);
}

void TestSolver::improve(Randomizer &rnd) {
    if (items.empty()) {
        return;
    }
    uint32_t i = 0;

    if (true || rnd.get_d() < 0.3) {
        i = rnd.get(0, items.size() - 1);
    } else {
        for (; i + 1 < items.size() && rnd.get_d() < 0.93; i++) {}
    }

    Solver solver(items[i].answer, test_data);
    Answer answer = solver.solve(rnd.get());
    ASSERT(items[i].answer.gold <= answer.gold, "failed to improve");
    if (items[i].answer.gold == answer.gold) {
        items[i].cnt_failed_improve++;
    } else {
        items[i].cnt_failed_improve = 0;

        // write answer
        /*{
            std::ofstream output(dirname + "/" + std::to_string(answer.gold) + ".json");
            std::cout << "write improve: " << dirname + "/" + std::to_string(answer.gold) + ".json" << std::endl;
            output << answer;
        }*/
    }
    items[i].answer = answer;
    bubble_sort(i);
    mow();

    // write log
    {
        std::ofstream output(dirname + "/log", std::ios::app);
        print_state(output);
    }
}

void TestSolver::add(Randomizer &rnd) {
    if (items.size() >= 10) {
        return;
    }
    auto monsters_order = test_data.monsters_order;
    std::shuffle(monsters_order.begin(), monsters_order.end(), rnd.generator);

    Solver solver(monsters_order, test_data);
    Answer answer = solver.solve(rnd.get());

    items.push_back({});
    items.back().answer = answer;

    bubble_sort(items.size() - 1);

    mow();

    // write answer
    /*{
        std::ofstream output(dirname + "/" + std::to_string(answer.gold) + ".json");
        std::cout << "write new:     " << dirname + "/" + std::to_string(answer.gold) + ".json" << std::endl;
        output << answer;
    }*/

    // write log
    {
        std::ofstream output(dirname + "/log", std::ios::app);
        print_state(output);
    }
}

void TestSolver::print_state(std::ostream &output) const {
    output << "[TestSolver] " << items.size() << ": ";
    for (auto &item: items) {
        output << "(" << item.answer.gold << "," << item.cnt_failed_improve << ")" << ' ';
    }
    output << '\n';
}

void TestSolver::write(const std::string &dirname) const {
    for (auto &item: items) {
        std::ofstream output(dirname + "/" + std::to_string(item.answer.gold) + ".json");
        output << item.answer;
    }
}

Answer TestSolver::get_best() const {
    if (items.empty()) {
        return {};
    } else {
        return items[0].answer;
    }
}
