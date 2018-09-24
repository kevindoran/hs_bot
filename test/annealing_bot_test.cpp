#include "gtest/gtest.h"

#include "AnnealingBot.h"
#include "InputParser.h"
#include "Board.h"

TEST(AnnealingBot, avoidUnsafeItem) {
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
    int depth = 3;

    // Setup AI
    AnnealingBot<6, 2> ab(100, 0);
    MinimalBot enemyAI(1);
    SimBot* allEnemyAI[] {&enemyAI};
    ab.setEnemyAI(allEnemyAI);
    Move move = ab.move(b);
    EXPECT_EQ(1, move.dir);
    EXPECT_FALSE(move.bomb);
}
