#include "gtest/gtest.h"

#include "Board.h"
#include "Position.h"
#include "InputParser.h"


TEST(BoardTest, pathDist) {
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
    int pathDist = b.pathDist(0, Board::toID(2, 0));
    ASSERT_EQ(10, pathDist);
}

TEST(BoardTest, adjTile) {
    Position p(5, 8);
    int t = Board::toID(p);
    int adjT = Board::adjTile(t, Position::UP);
    Position adjP = Board::toPosition(adjT);
    ASSERT_EQ(Position(4, 8), adjP);
    ASSERT_EQ(Position(6, 8), Board::toPosition(Board::adjTile(t, Position::DOWN)));
    ASSERT_EQ(Position(5, 7), Board::toPosition(Board::adjTile(t, Position::LEFT)));
    ASSERT_EQ(Position(5, 9), Board::toPosition(Board::adjTile(t, Position::RIGHT)));
}


TEST(BoardTest, explode) {
    std::string input =
        "13 11 0\n"
        "...000.......\n"
        "X0.01X.......\n"
        "0X0..........\n"
        ".............\n"
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
//    b.bombs[Board::toID(0, 0)] = Bomb(Board::toID(0, 0), blastLength, 0, countdown);
//    b.bombs[Board::toID(0, 2)] = Bomb(Board::toID(0, 2), blastLength, 0, countdown + 1);
    b.players[0].range = 3;
    b.placeBomb(0);
    b.players[0].tile = 2;
    b.placeBomb(0);
    b.stepForward(Bomb::TIMEOUT);
    EXPECT_EQ(2, b.players[0].boxesDestroyed);
    EXPECT_EQ(false, b.players[0].isAlive());
    // No damage to wall.
    EXPECT_EQ(Board::WALL, b.tiles[Board::toID(1, 0)]);
    // No pass through
    EXPECT_EQ(Board::BOX, b.tiles[Board::toID(0, 4)]);
    EXPECT_EQ(1, b.aliveCount);
}

TEST(BoardTest, explodeItem) {
    std::string input =
        "13 11 0\n"
        "X.X..........\n" // 0
        "X.X..........\n" // 1
        "X.X..........\n" // 2
        "X.X..........\n" // 3
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
    int range = 3;
    int countdown = 1;
    int tile = 1;
    b.tiles[Board::toID(2, 1)] = Board::BOMB_COUNT_PU;
    b.placeBombOnly(0, tile, countdown, range);
    b.players[0].tile = Board::toID(3, 1);
    b.stepForward(1);
    EXPECT_TRUE(b.players[0].isAlive());
    EXPECT_EQ(b.tiles[Board::toID(2, 1)], Board::EMPTY);
}

TEST(BoardTest, survivalTime) {
    std::string input =
        "13 11 0\n"
        "0............\n" // 0
        ".X.X.........\n" // 1
        "10.1.........\n" // 2
        ".X.X.........\n" // 3
        "0..1.........\n" // 4
        ".X.X.........\n" // 5
        "0..1.........\n" // 6
        ".X.X.........\n" // 7
        ".X.X.........\n" // 8
        ".X.X.........\n" // 9
        "..X..........\n" // 10
        "2\n"
        "0 0 0 0 1 3\n"
        "0 1 12 10 1 3\n";

    std::istringstream stream(input);
    InputParser ip(stream);
    ip.init();
    Board b = ip.parse();
    // Freedom.
    b.players[0].tile = Board::toID(6, 2);
    int bombTimer = 3;
    int range = 3;
    b.placeBombOnly(0, Board::toID(6, 1), bombTimer, range);
    b.stepForward(1);
    int max = 8;
    EXPECT_EQ(max, b.survivalTurns(0, max));
    // Completely trapped.
    b.tiles[Board::toID(5, 2)] = Board::BOX;
    b.tiles[Board::toID(7, 2)] = Board::BOX;
    EXPECT_EQ(2, b.survivalTurns(0, max));
}

TEST(BoardTest, placeBomb) {
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
    // Freedom.
    int bombTimer = 1;
    int range = 3;
    // A bomb at an edge.
    int tile = Board::toID(5, 12);
    b.placeBombOnly(0, tile, bombTimer, range);
    bool ans[Board::TILE_COUNT] = {0};
    // Bomb square
    ans[Board::toID(5, 12)] = true;
    // Above, by 3.
    ans[Board::toID(4, 12)] = true;
    ans[Board::toID(3, 12)] = true;
    ans[Board::toID(2, 12)] = true;
    // Below, by 3.
    ans[Board::toID(6, 12)] = true;
    ans[Board::toID(7, 12)] = true;
    ans[Board::toID(8, 12)] = true;
    // Left, by 3.
    ans[Board::toID(5, 11)] = true;
    ans[Board::toID(5, 10)] = true;
    ans[Board::toID(5, 9)] = true;
    for(int i = 0; i < Board::TILE_COUNT; i++) {
        if(ans[i]) {
            EXPECT_EQ(b.earliestExp(i), 1) << "At tile: " << Board::toPosition(i);
        } else {
            EXPECT_EQ(b.earliestExp(i), 0) << "At tile: " << Board::toPosition(i);
        }
        if(i == tile) {
            EXPECT_EQ(b.tiles[i], Board::BOMB) << "At tile: " << Board::toPosition(i);
        } else {
            EXPECT_EQ(b.tiles[i], Board::EMPTY) << "At tile: " << Board::toPosition(i);
        }
    }
}


TEST(BoardTest, placeBombCorner) {
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
    // Freedom.
    int bombTimer = 1;
    int range = 3;
    // A bomb at an edge.
    int tile = Board::toID(0, 12);
    b.placeBombOnly(0, tile, bombTimer, range);
    bool ans[Board::TILE_COUNT] = {0};
    // Bomb square
    ans[Board::toID(0, 12)] = true;
    // Below, by 3.
    ans[Board::toID(1, 12)] = true;
    ans[Board::toID(2, 12)] = true;
    ans[Board::toID(3, 12)] = true;
    // Left, by 3.
    ans[Board::toID(0, 11)] = true;
    ans[Board::toID(0, 10)] = true;
    ans[Board::toID(0, 9)] = true;
    for(int i = 0; i < Board::TILE_COUNT; i++) {
        if(ans[i]) {
            EXPECT_EQ(b.earliestExp(i), 1) << "At tile: " << Board::toPosition(i);
        } else {
            EXPECT_EQ(b.earliestExp(i), 0) << "At tile: " << Board::toPosition(i);
        }
        if(i == tile) {
            EXPECT_EQ(b.tiles[i], Board::BOMB) << "At tile: " << Board::toPosition(i);
        } else {
            EXPECT_EQ(b.tiles[i], Board::EMPTY) << "At tile: " << Board::toPosition(i);
        }
    }
}

TEST(BoardTest, placeBombNearItem) {
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
    int range = 3;
    int countdown = 1;
    int tile = Board::toID(5, 5);
    b.tiles[tile - 1] = Board::BOMB_RANGE_PU;
    b.tiles[tile + 1] = Board::BOMB_COUNT_PU;
    b.tiles[tile - Board::WIDTH] = Board::BOMB_RANGE_PU;
    b.tiles[tile + Board::WIDTH] = Board::BOMB_COUNT_PU;
    b.placeBombOnly(0, tile, countdown, range);
    EXPECT_EQ(1, b.earliestExp(tile-1));
    EXPECT_EQ(1, b.earliestExp(tile+1));
    EXPECT_EQ(1, b.earliestExp(tile-Board::WIDTH));
    EXPECT_EQ(1, b.earliestExp(tile+Board::WIDTH));
    // Outside the items, there should be no blast.
    EXPECT_EQ(0, b.earliestExp(tile-2));
    EXPECT_EQ(0, b.earliestExp(tile+2));
    EXPECT_EQ(0, b.earliestExp(tile-2*Board::WIDTH));
    EXPECT_EQ(0, b.earliestExp(tile+2*Board::WIDTH));
    // Items should be marked as unsafe.
    EXPECT_TRUE(b.unsafe[tile-1]);
    EXPECT_TRUE(b.unsafe[tile+1]);
    EXPECT_TRUE(b.unsafe[tile-Board::WIDTH]);
    EXPECT_TRUE(b.unsafe[tile+Board::WIDTH]);
    // Outside should be safe
    EXPECT_FALSE(b.unsafe[tile-2]);
    EXPECT_FALSE(b.unsafe[tile+2]);
    EXPECT_FALSE(b.unsafe[tile-2*Board::WIDTH]);
    EXPECT_FALSE(b.unsafe[tile+2*Board::WIDTH]);
}



TEST(BoardTest, survivalAllMaxEqual) {
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
    b.players[0].tile = Board::toID(0, 0);
    int bombTimer = 8;
    int range = 2;
    b.placeBombOnly(0, Board::toID(1, 0), bombTimer, range);
    b.stepForward(1);
    for(int max = 1; max < 8; max++) {
        EXPECT_EQ(max, b.survivalTurns(0, max));
    }
}


TEST(BoardTest, stepForwardItemShield) {
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
    b.players[0].tile = Board::toID(0, 0);
    // Item at both bomb's intersection.
    b.tiles[3] = Board::BOMB_COUNT_PU;
    // Big bomb at 0
    int bombTimer = 8;
    int range = 10;
    b.placeBombOnly(0, Board::toID(0, 0), bombTimer, range);
    // Small, soon bomb in small alley
    bombTimer = 1;
    range = 2;
    b.placeBombOnly(0, Board::toID(1, 3), bombTimer, range);
    EXPECT_EQ(1, b.earliestExp(3));
    b.stepForward(1);
    // The item position and after it should now be in the blast of the big bomb.
    EXPECT_EQ(7, b.earliestExp(4));
    // Uncomment when double explosions are enabled
    EXPECT_EQ(7, b.earliestExp(3));
}



