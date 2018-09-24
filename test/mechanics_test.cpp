#include "gtest/gtest.h"

#include <iostream>
#include "Mechanics.h"
#include "Board.h"
#include "InputParser.h"
#include "Position.h"


TEST(MechanicsTest, blastCount) {
    std::string input =
        "13 11 0\n"
        "....0.0.0.0..\n"
        ".............\n"
        ".....0.0.....\n"
        "0.0.0...0.0.0\n"
        ".............\n"
        ".............\n"
        ".............\n"
        ".............\n"
        ".............\n"
        ".............\n"
        ".............\n"
        "2\n"
        "0 0 0 0 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    Mechanics mech;

    int blastLength = 2;
    ASSERT_EQ(0, mech.blastCount(b, Position(0, 0), blastLength));
    ASSERT_EQ(1, mech.blastCount(b, Position(0, 3), blastLength));
    blastLength = 3;
    ASSERT_EQ(2, mech.blastCount(b, Position(1, 4), blastLength));
    ASSERT_EQ(3, mech.blastCount(b, Position(2, 4), blastLength));
}

TEST(MechanicsTest, maximizeBlast) {
    std::string input =
        "13 11 0\n"
        "....0.0.0.0..\n"
        ".............\n"
        ".....0.0.....\n"
        "0.0.0...0.0.0\n"
        ".............\n"
        ".............\n"
        ".............\n"
        ".............\n"
        ".............\n"
        ".............\n"
        ".............\n"
        "2\n"
        "0 0 0 0 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    Mechanics mech;


    Board b1 = b;
    ASSERT_EQ(Position(2, 4), Board::toPosition(mech.maximizeBlast(b1, b1.US, 7, 3)));
}


TEST(MechanicsTest, closestBoxSide) {
    std::string input =
        "13 11 0\n"
        "......0.0.0..\n"
        "....0........\n"
        ".............\n"
        "........0.0.0\n"
        ".............\n"
        ".............\n"
        ".............\n"
        ".............\n"
        ".............\n"
        ".............\n"
        ".............\n"
        "2\n"
        "0 0 0 0 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    Mechanics mech;


    Board b1 = b;
    Position closest = Board::toPosition(mech.closestBoxSide(b1, b1.ourTile()));
    ASSERT_TRUE(Position(1, 4) == closest || Position(0, 4) == closest);
}



TEST(MechanicsTest, maximizeBlastWithWalls) {
    std::string input =
        "13 11 0\n"
        "...0.0.0.0...\n"
        ".X.X1X1X1X.X.\n"
        "121..2.2..121\n"
        ".X1X2X1X2X1X.\n"
        "2.2.0.0.0.2.2\n"
        ".X.X0X.X0X.X.\n"
        "2.2.0.0.0.2.2\n"
        ".X1X2X1X2X1X.\n"
        "121..2.2..121\n"
        ".X.X1X1X1X.X.\n"
        "...0.0.0.0...\n"
        "2\n"
        "0 0 0 0 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    Mechanics mech;
    int steps = 4;
    int blastLength = 2;
    int tile = mech.maximizeBlast(b, b.US, steps, blastLength);
    ASSERT_EQ(2, tile);
}

TEST(MechanicsTest, maximizeBlastWithBombs) {
    std::string input =
        "13 11 0\n"
        "...0.0.0.0...\n"
        "1X.01X1X1X.X.\n"
        "121..2.2..121\n"
        ".X1X2X1X2X1X.\n"
        "2.2.0.0.0.2.2\n"
        ".X.X0X.X0X.X.\n"
        "2.2.0.0.0.2.2\n"
        ".X1X2X1X2X1X.\n"
        "121..2.2..121\n"
        ".X.X1X1X1X.X.\n"
        "...0.0.0.0...\n"
        "2\n"
        "0 0 0 0 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    Mechanics mech;

    b.players[b.US] = Board::toID(0, 2);
    b.placeBomb(b.US);
    b.stepForward(1);
    int tile = mech.maximizeBlast(b, b.US, Bomb::TIMEOUT, b.players[b.US].range);
    ASSERT_EQ(15, tile);
}


TEST(MechanicsTest, closestCount) {
    std::string input =
        "13 11 0\n"
        "012012.......\n" // 0
        ".....1.......\n" // 1
        ".....XXX.....\n" // 2
        ".....X1X.....\n" // 3
        ".....XXX.....\n" // 4
        ".............\n" // 5
        ".............\n" // 6
        ".............\n" // 7
        "........02...\n" // 8
        ".......2.1...\n" // 9
        ".............\n" // 10
        "2\n"
        // Our bot at (y,x) (0,1)
        "0 0 1 1 1 3\n"
        "0 1 1 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    ASSERT_EQ(7, BoardStats::closestCount(b, 0));
    ASSERT_EQ(4, BoardStats::closestCount(b, 1));

}

TEST(MechanicsTest, closestCount2) {
    std::string input =
        "13 11 0\n"
        "XXX012.......\n" // 0
        "X.X..1.......\n" // 1
        "XXX..XXX.....\n" // 2
        ".....X1X.....\n" // 3
        ".....XXX.....\n" // 4
        ".............\n" // 5
        ".............\n" // 6
        ".............\n" // 7
        "........02...\n" // 8
        "XXXXXXXXXXXXX\n" // 9
        ".............\n" // 10
        "2\n"
        // Our bot at (y,x) (0,1)
        "0 0 1 1 1 3\n"
        "0 1 1 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    ASSERT_EQ(0, BoardStats::closestCount(b, 0));
    ASSERT_EQ(0, BoardStats::closestCount(b, 1));

}


TEST(MechanicsTest, stepsToClosestBox) {
    std::string input =
        "13 11 0\n"
        "1............\n" // 0
        ".............\n" // 1
        ".....XXX.....\n" // 2
        "00...X1X.....\n" // 3
        ".....XXX.....\n" // 4
        ".............\n" // 5
        ".............\n" // 6
        ".............\n" // 7
        ".............\n" // 8
        ".............\n" // 9
        ".............\n" // 10
        "3\n"
        // Our bot at (y,x) (0,1)
        "0 0 1 1 1 3\n"
        "0 1 1 10 1 3\n"
        "0 2 6 3 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    ASSERT_EQ(1, BoardStats::stepsToClosestBox(b, 0));
    ASSERT_EQ(6, BoardStats::stepsToClosestBox(b, 1));
    ASSERT_EQ(-1, BoardStats::stepsToClosestBox(b, 2));
}

TEST(MechanicsTest, stepsToClosestPlayer) {
    std::string input =
        "13 11 0\n"
        "1............\n" // 0
        ".............\n" // 1
        ".....XXX.....\n" // 2
        "00...X1X.....\n" // 3
        ".....XXX.....\n" // 4
        ".............\n" // 5
        ".............\n" // 6
        ".............\n" // 7
        ".............\n" // 8
        ".............\n" // 9
        ".............\n" // 10
        "4\n"
        // Our bot at (y,x) (0,1)
        "0 0 0 0 1 3\n"
        "0 1 0 10 1 3\n"
        "0 2 6 3 1 3\n"
        "0 3 8 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    EXPECT_EQ(1, BoardStats::closestPlayer(b, 0).first);
    EXPECT_EQ(14, BoardStats::closestPlayer(b, 0).second);
    EXPECT_EQ(3, BoardStats::closestPlayer(b, 1).first);
    EXPECT_EQ(8, BoardStats::closestPlayer(b, 1).second);
    EXPECT_EQ(-1, BoardStats::closestPlayer(b, 2).first);
    EXPECT_EQ(-1, BoardStats::closestPlayer(b, 2).second);
    EXPECT_EQ(1, BoardStats::closestPlayer(b, 3).first);
    EXPECT_EQ(8, BoardStats::closestPlayer(b, 3).second);
}