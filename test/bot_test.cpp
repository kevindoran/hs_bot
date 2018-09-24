#include "gtest/gtest.h"
#include "Bot.h"
#include "Board.h"
#include "InputParser.h"

TEST(BotTest, move) {
    std::string input =
        "13 11 0\n"
        "...0.0.0.0...\n"
        "1X.01X1X1X.X.\n"
        ".X...2.2..121\n"
        ".X.X2X1X2X1X.\n"
        "....0.0.0.2.2\n"
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
    Bot<9> bot(0);

    pair<int, bool> move = bot.move(b);
    ASSERT_EQ(0, move.first);
    ASSERT_EQ(true, move.second);


}


TEST(BotTest, move2) {
    std::string input =
        "13 11 0\n"
        "....00.00....\n"
        ".X1X2X.X2X1X.\n"
        ".1...2.2...1.\n"
        ".X2X1X.X1X2X.\n"
        ".01.......10.\n"
        ".X.X2X0X2X.X.\n"
        ".01.......10.\n"
        ".X2X1X.X1X2X.\n"
        ".1...2.2...1.\n"
        ".X1X2X.X2X1X.\n"
        "....00.00....\n"
        "2\n"
        "0 0 0 0 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    Bot<9> bot(0);
    pair<int, bool> move = bot.move(b);
    EXPECT_EQ(0, move.first);
    EXPECT_EQ(false, move.second);
}


TEST(BotTest, moveAvoidOwnBomb) {
    std::string input =
        "13 11 0\n"
        "..10.0.0.0...\n"
        ".X.01X1X1X.X.\n"
        "1X...2.2..121\n"
        ".X.X2X1X2X1X.\n"
        "....0.0.0.2.2\n"
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
    Bot<6> bot(0);
    pair<int, bool> move = bot.move(b);
    // Can't place at zero, or I'll blow up.
    EXPECT_NE(4, move.first);
    EXPECT_NE(-1, move.first);
    EXPECT_EQ(false, move.second);


    // Place bomb in right corner.
    b.players[0].tile = 1;
    b.placeBomb(0);
    // Move to left corner before explosion.
    b.players[0].tile = 13;
    b.stepForward(6);
    // Shouldn't have exploded yet.
    ASSERT_EQ(Board::BOMB, b.tiles[1]);
    move = bot.move(b);
    // Stay put, and cant place bomb.
    EXPECT_EQ(4, move.first);
    EXPECT_EQ(false, move.second);
}

TEST(BotTest, moveAvoidOwnBomb3) {
    std::string input =
            "13 11 0\n"
            "..10.0.0.0...\n"
            ".1.01X1X1X.X.\n"
            ".1...2.2..121\n"
            ".X.X2X1X2X1X.\n"
            "X...0.0.0.2.2\n"
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
    b.placeBomb(0);
    b.stepForward(6);
    b.players[0].tile = Board::toID(3, 0);
    Bot<7> bot(0);
    pair<int, bool> move = bot.move(b);
    // Can't place at zero, or I'll blow up.
    EXPECT_EQ(4, move.first);
    EXPECT_EQ(false, move.second);
}


TEST(BotTest, moveAvoidOwnBomb2) {
    std::string input =
        "13 11 0\n"
        "...12121.....\n"
        ".X2X.X.......\n"
        ".21..........\n"
        ".X1X2X.......\n"
        ".22..........\n"
        ".............\n"
        ".............\n"
        ".............\n"
        ".............\n"
        ".............\n"
        ".............\n"
        "2\n"
        // Our bot at (y,x) (0,1)
        "0 0 1 0 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    Bot<7> bot(0);
    pair<int, bool> move = bot.move(b);
    // Can't place at zero, or I'll blow up.
    EXPECT_EQ(0, move.first);
    EXPECT_EQ(false, move.second);
}

TEST(BotTest, avoidBombTrail) {
    std::string input =
        "13 11 0\n"
        ".............\n" // 0
        ".............\n" // 1
        ".............\n" // 2
        ".............\n" // 3
        ".............\n" // 4
        ".............\n" // 5
        ".............\n" // 6
        ".X.X.X.X.X.X.\n" // 7
        "0............\n" // 8
        ".X.X.X.X.X.X.\n" // 9
        ".............\n" // 10
        "2\n"
        // Our bot at (y,x) (0,1)
        "0 0 4 10 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    int blastLen = 3;
    b.placeBombOnly(0, Board::toID(9, 0), 3, blastLen);
    b.placeBombOnly(0, Board::toID(10, 0), 5, blastLen);
    b.placeBombOnly(0, Board::toID(10, 1), 3, blastLen);
    b.placeBombOnly(0, Board::toID(10, 2), 7, blastLen);
    b.placeBombOnly(0, Board::toID(10, 3), 8, blastLen);
    b.players[0].bombsAvailable = 1;
    Bot<4> bot(0);
    pair<int, bool> move = bot.move(b);
    // Moving right and placing a bomb is suicide.
    EXPECT_FALSE(move.first == Position::RIGHT && move.second == true);
}


TEST(BotTest, avoidUnsafeItem) {
    std::string input =
        "13 11 0\n"
        ".............\n" // 0
        "XXX.X........\n" // 1
        ".............\n" // 2
        ".............\n" // 3
        ".............\n" // 4
        ".............\n" // 5
        ".............\n" // 6
        ".............\n" // 7
        ".............\n" // 8
        ".............\n" // 9
        ".............\n" // 10
        "2\n"
        "0 0 0 0 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    b.players[0].tile = Board::toID(0, 3);
    // Powerup acts as shield, behind it, but not when picked up.
    b.tiles[Board::toID(0, 4)] = Board::BOMB_COUNT_PU;
    int bombTimer = 3;
    int range = 10;
    b.placeBombOnly(0, Board::toID(0, 2), bombTimer, range);
    Bot<4> bot(0);
    pair<int, bool> move = bot.move(b);
    EXPECT_EQ(1, move.first);
    EXPECT_FALSE(move.second);
}

TEST(BotTest, sparseWithItems) {
    std::string input =
        "13 11 0\n"
        ".............\n" // 0
        ".............\n" // 1
        ".............\n" // 2
        ".............\n" // 3
        ".............\n" // 4
        ".............\n" // 5
        ".............\n" // 6
        ".............\n" // 7
        ".............\n" // 8
        ".............\n" // 9
        ".............\n" // 10
        "2\n"
        "0 0 0 0 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    b.players[0].tile = Board::toID(5, 5);
    // Powerup acts as shield, behind it, but not when picked up.
    b.tiles[Board::toID(4, 4)] = Board::BOMB_COUNT_PU;
    b.tiles[Board::toID(0, 4)] = Board::BOMB_COUNT_PU;
    b.tiles[Board::toID(6, 4)] = Board::BOMB_COUNT_PU;
    b.tiles[Board::toID(8, 3)] = Board::BOMB_COUNT_PU;
    int bombTimer = 8;
    int range = 10;
    b.placeBombOnly(0, Board::toID(0, 2), bombTimer, range);
    Bot<5> bot(0);
    pair<int, bool> move = bot.move(b);
}

TEST(BotTest, simpleCornerStart) {
    std::string input =
        "13 11 0\n"
        "..00.........\n" // 0
        ".XX..........\n" // 1
        "0X...........\n" // 2
        "0............\n" // 3
        ".............\n" // 4
        ".............\n" // 5
        ".............\n" // 6
        ".............\n" // 7
        ".............\n" // 8
        ".............\n" // 9
        ".............\n" // 10
        "2\n"
        "0 0 0 0 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    b.players[0].tile = 0;
    Bot<5> bot(0);
    pair<int, bool> move = bot.move(b);
    EXPECT_NE(4, move.first);
    EXPECT_FALSE(move.second);
    b.players[0].tile = Board::toID(0, 1);
    move = bot.move(b);
    EXPECT_EQ(2, move.first);
    EXPECT_TRUE(move.second);
}


TEST(BotTest, futureMovePicksItem) {
    std::string input =
        "13 11 0\n"
        "...1.....1...\n" // 0
        ".X1X.....X1X.\n" // 1
        "...1.....1...\n" // 2
        ".X1X.....X1X.\n" // 3
        ".............\n" // 4
        ".............\n" // 5
        ".............\n" // 6
        ".............\n" // 7
        ".............\n" // 8
        ".............\n" // 9
        ".............\n" // 10
        "2\n"
        "0 0 0 0 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    b.players[0].tile = Board::toID(1, 0);
    // Item in top right inlet.
    b.tiles[2] = Board::BOMB_COUNT_PU;
    // Item also at mirror position on enemy side
    b.tiles[Board::toID(0, 11)] = Board::BOMB_COUNT_PU;
    // Bomb in second inlet.
    int bombTimer = 6;
    int range = 4;
    b.placeBombOnly(0, Board::toID(2, 2), bombTimer, range);
    // Enemy bomb in their inlet.
    bombTimer = 5;
    range = 3;
    b.placeBombOnly(1, Board::toID(2, 10), bombTimer, range);
    Bot<5> bot(0);
    pair<int, bool> move = bot.move(b);
    // Don't segfault.
    EXPECT_TRUE(true);
}

TEST(BotTest, futureMovePicksItem2) {
    std::string input =
        "13 11 0\n"
        ".....2.2.0...\n"
        ".X.X1X.X1X.X.\n"
        "...r.111.1...\n"
        ".XBX2X.X2XcX.\n"
        "..20..1.BBB..\n"
        ".X.X1X2X1X.X.\n"
        "..20..1..0B..\n"
        ".X2X2X.X2X2XB\n"
        "..01.111.10..\n"
        ".X.X1X.X1X.X.\n"
        "..00.2.2.0...\n"
        "2\n"
        "0 0 0 0 1 3\n"
        // Enemy at (9, 4)
        "0 1 9 4 1 3\n"
        "0 2 12 7 1 3\n";


    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    b.players[0].tile = Board::toID(2, 2);
    // Item in top left-ish.
    b.tiles[Board::toID(2, 3)] = Board::BOMB_RANGE_PU;
    // Item on top right-ish.
    b.tiles[Board::toID(3, 10)] = Board::BOMB_COUNT_PU;
    // Bomb in top left-ish.
    int bombTimer = 8;
    int range = 2;
    b.placeBombOnly(0, Board::toID(3, 2), bombTimer, range);
    // Five bombs on their side.
    bombTimer = 3;
    b.placeBombOnly(1, Board::toID(4, 8), bombTimer, range);
    bombTimer = 4;
    b.placeBombOnly(1, Board::toID(4, 9), bombTimer, range);
    bombTimer = 3;
    b.placeBombOnly(1, Board::toID(4, 10), bombTimer, range);
    bombTimer = 5;
    b.placeBombOnly(1, Board::toID(6, 10), bombTimer, range);
    bombTimer = 8;
    b.placeBombOnly(1, Board::toID(7, 12), bombTimer, range);
    Bot<5> bot(0);
    pair<int, bool> move = bot.move(b);
    // Don't segfault.
    EXPECT_TRUE(true);
}

TEST(BotTest, escape) {
    std::string input =
        "13 11 0\n"
        ".............\n" // 0
        ".............\n" // 1
        ".............\n" // 2
        ".....X.X.....\n" // 3
        ".....0.0.....\n" // 4
        ".....X.X.....\n" // 5
        ".....0.......\n" // 6
        ".....X2X.....\n" // 7
        ".............\n" // 8
        ".............\n" // 9
        ".............\n" // 10
        "2\n"
        "0 0 0 0 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    // Player in end of alley.
    b.players[0].tile = Board::toID(6, 6);
    // No items.
    // 4 Bombs. 3 In alley, one to the left side, about to blow and create an escape.
    int timer = 2;
    int range = 2;
    b.placeBombOnly(1, Board::toID(2, 6), timer, range);
    timer = 4;
    b.placeBombOnly(1, Board::toID(4, 6), timer, range);
    timer = 7;
    b.placeBombOnly(1, Board::toID(5, 6), timer, range);
    // Escape bomb.
    timer = 1;
    b.placeBombOnly(1, Board::toID(6, 4), timer, range);
    Bot<5> bot(0);
    pair<int, bool> move = bot.move(b);
    // Bot should take the right exit, even though it will die in 4 turns. Choosing left will die in 2 turns,
    // as it can't walk on the box due to the game bug.
    EXPECT_EQ(0, move.first);
    EXPECT_FALSE(move.second);
}


TEST(BotTest, stayOnBomb) {
    std::string input =
        "13 11 0\n"
        ".............\n" // 0
        "XXX.X........\n" // 1
        "..XXX........\n" // 2
        ".............\n" // 3
        ".............\n" // 4
        ".............\n" // 5
        ".............\n" // 6
        ".............\n" // 7
        ".............\n" // 8
        ".............\n" // 9
        ".............\n" // 10
        "2\n"
        "0 0 0 0 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    // Player on bomb in kink.
    b.players[0].tile = Board::toID(1, 3);
    // Bomb under player.
    int blastLength = 2;
    int timer = 8;
    b.placeBombOnly(0, Board::toID(1, 3), timer, blastLength);
    // Bomb in corridor. Unsafe.
    blastLength = 10;
    timer = 2;
    b.placeBombOnly(0, Board::toID(0, 0), timer, blastLength);
    int depth = 1;
    EXPECT_EQ(8, b.survivalTurns(0, 8));
    Bot<2> bot(0);
    pair<int, bool> move = bot.move(b);
    EXPECT_EQ(4, move.first);
    depth = 4;
    Bot<5> bot2(0);
    move = bot2.move(b);
    // Player should stay on bomb, where it is safe.
    EXPECT_EQ(4, move.first);
    EXPECT_FALSE(move.second);
}

TEST(BotTest, suicidePrevention) {
    std::string input =
        "13 11 0\n"
        ".............\n" // 0
        ".............\n" // 1
        ".............\n" // 2
        ".............\n" // 3
        "2............\n" // 4
        ".X.X.........\n" // 5
        ".1.1.........\n" // 6
        ".X.X.........\n" // 7
        ".............\n" // 8
        ".............\n" // 9
        ".............\n" // 10
        "2\n"
        "0 0 0 0 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    // Player trapped, should wait for box to open and escape.
    b.players[0].tile = Board::toID(6, 0);
    // Two bombs: one below, and another behind box to the right.
    int blastLength = 3;
    int timer = 7;
    b.placeBombOnly(0, Board::toID(8, 0), timer, blastLength);
    timer = 5;
    b.placeBombOnly(0, Board::toID(6, 2), timer, blastLength);
    EXPECT_EQ(8, b.survivalTurns(0, 8));
    Bot<3> bot(0);
    pair<int, bool> move = bot.move(b);
    EXPECT_FALSE(move.second);
    Bot<4> bot2(0);
    move = bot2.move(b);
    EXPECT_FALSE(move.second);
    Bot<5> bot3(0);
    move = bot3.move(b);
    EXPECT_FALSE(move.second);
    Bot<6> bot4(0);
    move = bot4.move(b);
    EXPECT_FALSE(move.second);
}

TEST(BotTest, cantMoveToDestroyedBox) {
    std::string input =
        "13 11 0\n"
        ".............\n" // 0
        ".............\n" // 1
        ".............\n" // 2
        ".............\n" // 3
        ".............\n" // 4
        ".X1X.........\n" // 5
        ".............\n" // 6
        ".X0X1X.X.....\n" // 7
        ".............\n" // 8
        ".X.X2X.X.....\n" // 9
        "....2........\n" // 10
        "14\n"
        "0 0 2 8 3 5\n"
        "0 1 8 8 1 8\n"
        "1 0 2 6 1 4\n"
        "1 1 6 8 2 7\n"
        "1 0 1 8 6 5\n"
        "1 1 7 10 6 8\n"
        "1 1 8 10 7 8\n"
        "2 0 12 4 2 2\n"
        "2 0 11 6 2 2\n"
        "2 0 4 3 1 1\n"
        "2 0 5 2 1 1\n"
        "2 0 4 1 2 2\n"
        "2 0 4 0 2 2\n"
        "2 0 5 10 1 1\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    // Player trapped, should go down. Up is to a not yet cleared destroyed box.
//    b.players[0].tile = Board::toID(8, 2);
    // Extra item.
//    b.tiles[Board::toID(10, 5)] = Board::BOMB_RANGE_PU;
    // Three bombs. One above danger box. Other two, same line, forcing player up or down.
//    int blastLength = 6;
//    int timer = 1;
//    b.placeBombOnly(0, Board::toID(6, 2), timer, blastLength);
//    timer = 2;
//    b.placeBombOnly(0, Board::toID(8, 6), timer, blastLength);
//    timer = 6;
//    b.placeBombOnly(0, Board::toID(8, 1), timer, blastLength);
    // The above has no issues. Adding the other bombs.
//    timer = 6;
//    b.placeBombOnly(0, Board::toID(10, 7), timer, blastLength);
//    timer = 7;
//    b.placeBombOnly(0, Board::toID(10, 8), timer, blastLength);

    EXPECT_EQ(8, b.survivalTurns(0, 8));
    Bot<3> bot(0);
    pair<int, bool> move = bot.move(b);
    EXPECT_EQ(1, move.first);
    EXPECT_FALSE(move.second);
}


TEST(BotTest, cantPassDestroyedBoxMove) {
    std::string input1 =
        "13 11 0\n"
        ".............\n"
        ".X.XcX.X.X.X.\n"
        "B....r.r....B\n"
        ".X.X.X.X.X.X.\n"
        "00B.r...r.B00\n"
        ".X.X.X.X.X.X.\n"
        ".0r.1...1..0.\n"
        ".X.X.X.X.X.X.\n"
        "....B1.1BB...\n"
        ".X.X.X.XBX.X.\n"
        ".............\n"
        "18\n"
        "0 0 2 2 1 4\n"
        "0 1 10 8 1 3\n"
        "0 2 10 6 1 5\n"
        "0 3 2 7 3 3\n"
        "1 0 0 2 3 3\n"
        "1 2 12 2 3 3\n"
        "1 1 8 9 4 3\n"
        "1 1 8 8 6 3\n"
        "1 3 4 8 6 3\n"
        "1 0 2 4 7 4\n"
        "1 2 10 4 7 4\n"
        "1 1 9 8 8 3\n"
        "2 0 2 6 1 1\n"
        "2 0 4 4 1 1\n"
        "2 0 7 2 1 1\n"
        "2 0 8 4 1 1\n"
        "2 0 5 2 1 1\n"
        "2 0 4 1 2 2\n";

    std::string input2 =
        ".............\n"
        ".X.XcX.X.X.X.\n"
        "B....r.r....B\n"
        ".X.X.X.X.X.X.\n"
        "00B.r...r.B00\n"
        ".X.X.X.X.X.X.\n"
        ".0..1...1..0.\n"
        ".X.X.X.X.X.X.\n"
        "....B1.1BBB..\n"
        ".X.X.X.XBX.X.\n"
        ".............\n"
        "18\n"
        "0 0 3 2 1 4\n"
        "0 1 11 8 0 3\n"
        "0 2 10 7 1 5\n"
        "0 3 2 6 3 4\n"
        "1 0 0 2 2 3\n"
        "1 2 12 2 2 3\n"
        "1 1 8 9 3 3\n"
        "1 1 8 8 5 3\n"
        "1 3 4 8 5 3\n"
        "1 0 2 4 6 4\n"
        "1 2 10 4 6 4\n"
        "1 1 9 8 7 3\n"
        "1 1 10 8 8 3\n"
        "2 0 4 4 1 1\n"
        "2 0 7 2 1 1\n"
        "2 0 8 4 1 1\n"
        "2 0 5 2 1 1\n"
        "2 0 4 1 2 2\n";
// From replay: https://www.codingame.com/replay/139359602
    std::ostringstream os;
    os << input1 << input2;
    std::istringstream stream(os.str());
    InputParser ip(stream);
    ip.init();
    Board b;
    ip.update(b);
    ip.update(b);

    Bot<2> bot(3);
    pair<int, bool> move = bot.move(b);
    EXPECT_FALSE(move.first == 0 && move.second);
    Bot<3> bot2(3);
    move = bot2.move(b);
    EXPECT_FALSE(move.first == 0 && move.second);
    Bot<4> bot3(3);
    move = bot3.move(b);
    EXPECT_FALSE(move.first == 0 && move.second);
    Bot<5> bot4(3);
    move = bot4.move(b);
    EXPECT_FALSE(move.first == 0 && move.second);
}


TEST(BotTest, stepForwardItemShield) {
    std::string input =
        "13 11 0\n"
        "...2.........\n" // 0
        ".X0X2X.......\n" // 1
        ".0...1.......\n" // 2
        ".X2X.X.......\n" // 3
        ".............\n" // 4
        ".............\n" // 5
        ".............\n" // 6
        ".............\n" // 7
        ".............\n" // 8
        ".............\n" // 9
        ".............\n" // 10
        "2\n"
        "0 0 0 0 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    b.players[0].tile = Board::toID(2, 2);
    // Two bombs. Don't place another.
    int timer = 6;
    int range = 2;
    b.placeBombOnly(0, Board::toID(0, 2), timer, range);
    timer = 8;
    b.placeBombOnly(0, Board::toID(2, 3), timer, range);
    EXPECT_EQ(8, b.survivalTurns(0, 8));
    int depth = 1;
    Bot<5> bot(0);
    pair<int, bool> move = bot.move(b);
    EXPECT_FALSE(move.second);
}

TEST(BotTest, moveToItem) {
    std::string input =
        "13 11 0\n"
        ".............\n" // 0
        ".............\n" // 1
        ".............\n" // 2
        ".............\n" // 3
        ".............\n" // 4
        ".............\n" // 5
        ".............\n" // 6
        ".......X.X.X.\n" // 7
        ".........0...\n" // 8
        ".......X.X.X.\n" // 9
        ".........1...\n" // 10
        "2\n"
        // Our bot at (8, 12)
        "0 0 12 7 1 3\n"
        "0 1 0 0 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    b.players[0].bombsAvailable = 0;
    b.tiles[Board::toID(6, 12)] = Board::BOMB_COUNT_PU;
    int range = 2;
    int timer = 5;
    b.placeBombOnly(0, Board::toID(10, 11), timer, range);
    Bot<5> bot(0);
    pair<int, bool> move = bot.move(b);
    // Should move up to take item.
    EXPECT_EQ(3, move.first);
}

TEST(BotTest, timeoutTest) {
    std::string input =
        "13 11 0\n"
        ".............\n"
        ".X.X.X.X.X.X.\n"
        "...........r.\n"
        "BXcX.X.X.XcX1\n"
        "1....c.c....1\n"
        ".X.X.X.X.X.X.\n"
        "1....c.c....1\n"
        "1X2X.X.X.X2X1\n"
        "011.0B.B0.11.\n"
        ".X2X.X.X.XcX.\n"
        ".............\n"
        "14\n"
        "0 0 6 6 3 6\n"
        "0 1 6 6 1 3\n"
        "0 2 0 2 1 6\n"
        "1 1 7 8 1 3\n"
        "1 1 5 8 4 3\n"
        "1 2 0 3 8 6\n"
        "2 0 10 9 2 2\n"
        "2 0 2 3 2 2\n"
        "2 0 11 2 1 1\n"
        "2 0 10 3 2 2\n"
        "2 0 5 4 2 2\n"
        "2 0 7 6 2 2\n"
        "2 0 5 6 2 2\n"
        "2 0 7 4 2 2\n";
    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    Bot<5> bot(0);
    pair<int, bool> move = bot.move(b);
    // Don't time out.
}
// This breaks in this replay: https://www.codingame.com/replay/140593903
// The bot doesn't want to break the box, as he will be penalized for being far away from
// the next one.
TEST(BotTest, distScoreTooHigh) {
    std::string input =
        "13 11 0\n"
        ".........0...\n" // 0
        ".........X.X.\n" // 1
        ".............\n" // 2
        ".............\n" // 3
        ".............\n" // 4
        ".............\n" // 5
        ".............\n" // 6
        ".............\n" // 7
        ".............\n" // 8
        ".............\n" // 9
        "..1..........\n" // 10
        "2\n"
        "0 0 0 0 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    b.players[0].tile = Board::toID(2, 10);
    b.players[1].tile = Board::toID(2, 10);
    b.tiles[Board::toID(0, 10)] = Board::BOMB_COUNT_PU;
    Bot<6> bot(0);
    pair<int, bool> move = bot.move(b);
    // Move up and get the item and blow up the box.
    ASSERT_EQ(3, move.first);
}

TEST(BotTest, ab) {
    std::string input =
            "13 11 0\n"
                    ".............\n" // 0
                    ".............\n" // 1
                    ".............\n" // 2
                    ".............\n" // 3
                    ".............\n" // 4
                    ".............\n" // 5
                    ".............\n" // 6
                    ".X.X.X.X.X.X.\n" // 7
                    "0............\n" // 8
                    ".X.X.X.X.X.X.\n" // 9
                    ".............\n" // 10
                    "2\n"
                    // Our bot at (y,x) (0,1)
                    "0 0 4 10 1 3\n"
                    "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}