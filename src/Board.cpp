#include "Board.h"

const int Board::EMPTY;
const int Board::BOMB;
const int Board::BOX;
const int Board::BOMB_RANGE_BOX;
const int Board::BOMB_COUNT_BOX;
const int Board::BOMB_RANGE_PU;
const int Board::BOMB_COUNT_PU;
const int Board::WALL;
const int Board::INVALID;


int Board::playerCount;
int Board::US;
int Board::totalBoxes;
bool Board::hash[Board::TILE_COUNT * Bomb::TIMEOUT];
PosMap Board::positionMap;

PosMap::PosMap() {
    for(int i = 0; i < Board::TILE_COUNT; i++) {
        PosMap::m[i] =Position(i / Board::WIDTH, i % Board::WIDTH);
    }
}

