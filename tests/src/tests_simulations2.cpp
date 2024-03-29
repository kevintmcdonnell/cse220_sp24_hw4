#include "test_utils.h"
#include <fstream>

extern "C" {
#include "hw4.h"
}


class simulations2_TestSuite : public testing::Test {
protected:
    void SetUp() override {
        mkdir("./tests/expected_outputs/", 0700);
        mkdir("./tests/actual_outputs/", 0700);
    }
};

TEST_F(simulations2_TestSuite, simulation02_actually_passed) {
    // check if the simulation passed from     std::ofstream file("./tests/actual_outputs/simulation01_passed.txt"); below
    std::string solution_filename = "./tests/actual_outputs/simulation01_passed.txt";
    std::ifstream solution_file(solution_filename);
    std::string line;
    bool simulation01_passed = false;
    if (solution_file.is_open()) {
        while (getline(solution_file, line)) {
            if (line == "simulation01_passed is true.") {
                simulation01_passed = true;
            }
        }
        solution_file.close();
    }
    ASSERT_TRUE(simulation01_passed) << "simulation01_passed is false." << std::endl;
}
