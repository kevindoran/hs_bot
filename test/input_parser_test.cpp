#include "gtest/gtest.h"

#include <string>
#include <iostream>

#include "InputParser.h"

class InputParserTest: public ::testing::Test {
protected:
    std::string input =
    "13 11 0\n"
    "..0.0.0.0.0..\n"
    ".............\n"
    ".....0.0.....\n"
    "0.0.......0.0\n"
    ".....0.0.....\n"
    ".0.0.....0.0.\n"
    ".....0.0.....\n"
    "0.0.......0.0\n"
    ".....0.0.....\n"
    ".............\n"
    "..0.0.0.0.0..\n"
    "2\n"
    "0 0 0 0 1 3\n"
    "0 1 12 10 1 3\n";
};

TEST_F(InputParserTest, initAndParse) {
    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    ASSERT_EQ(0, b.ourTile());
}
