#include "Board.h"
#include "Mechanics.h"

int BoardStats::seen[Board::TILE_COUNT];
TilePlayer BoardStats::qu[Board::TILE_COUNT];
int BoardStats::neigh[4];
int BoardStats::quInt[Board::TILE_COUNT];

