#ifndef HYPERSONIC_INPUTPARSER_H
#define HYPERSONIC_INPUTPARSER_H

#include <iostream>
#include <unordered_set>
#include <cstring>

#include "Board.h"

class InputParser {
    std::istream& stream;
    Board prev;
    int turn = 0;

public:
    int ourID;
    InputParser(std::istream& stream) : stream(stream) {};

    void init() {
        int dummy;
        // Width & height
        stream >> dummy >> dummy;
        stream >> ourID;
        stream.ignore();  // \n
    }

    Board parse() {
        Board board;
        update(board);
        return board;
    }

    void update(Board& board) {
        board.turn = turn;
        memset(board.explodeM, 0, sizeof(bool) * Bomb::TIMEOUT * Board::TILE_COUNT);
        memset(board.scoresM, 0, sizeof(int) * Bomb::TIMEOUT * Board::MAX_PLAYERS);
//        memset(board.explodeRange, 0, sizeof(board.explodeRange));
//        memset(board.explodeOwner, 0, sizeof(board.explodeOwner));
        memset(board.unsafe, 0, sizeof(board.unsafe));
//        board.bombs.clear();
        board.bombCount = 0;
        int boxCount = 0;
        for (int i = 0; i < Board::HEIGHT; i++) {
            std::string row;
            getline(stream, row);
//            std::cerr << row << std::endl;
            for(int j = 0; j < row.length(); j++) {
                board(i, j) = row[j];
                if(row[j] == Board::EMPTY) {
                } else if(board.isBox(row[j])) {
                    boxCount++;
                }
            }
        }
        int playerCount = 0;
        int entities;
        stream >> entities;
        stream.ignore();
        int prevEntityType = 0;
        vector<Bomb> bombs;
        for (int i = 0; i < entities; i++) {
            int entityType;
            int owner;
            int x;
            int y;
            int param1;
            int param2;
            stream >> entityType >> owner >> x >> y >> param1 >> param2; stream.ignore();
//            std::cerr << "ET: " << entityType << "  Owner: " << owner << "  X: " << x << "  Y: " << y <<  "  Param1: " << param1 <<  "  Param2: " << param2 << std::endl;
//            std::cerr << entityType << " " << owner << " " << x << " " << y <<  " " << param1 <<  " " << param2 << std::endl;
            int tile = Board::toID(y, x);
            if(entityType < prevEntityType) {
                throw std::runtime_error("Entities are assumed to be in order");
            }
            prevEntityType = entityType;
            if(entityType == 0) {
                board.players[owner].tile = tile;
                board.players[owner].bombsAvailable = param1;
                board.players[owner].totalBombs = param1;
                board.players[owner].range = param2-1;
                playerCount++;
            } else if(entityType == 1) {
//                board.bombs[tile] = Bomb(tile, param2, owner, param1);
//                if(board.dist(tile, board.players[owner].tile) > 1) {
//                    throw std::runtime_error("Bomb not at player's position.");
//                }
                // Need to deal with bombs after powerups are placed.
                bombs.push_back(Bomb(tile, param2-1, owner, turn + param1));
//                board.placeBombOnly(owner, tile, param1, param2-1);
//                board.players[owner].totalBombs++;
            } else if(entityType == 2) {
                if(param1 == PowerUp::RANGE) {
                    board.tiles[tile] = Board::BOMB_RANGE_PU;
                } else if(param1 == PowerUp::COUNT) {
                    board.tiles[tile] = Board::BOMB_COUNT_PU;
                } else {
                    throw std::runtime_error("Unexpected powerup.");
                }
            } else {
                throw std::runtime_error("Unexpected entity.");
            }
        }
        for(auto& b : bombs) {
            board.placeBombOnly(b.owner, b.tile, b.explodeTurn - turn, b.blastLength);
        }
        board.aliveCount = playerCount;
        if(turn == 0) {
            board.playerCount = playerCount;
            board.US = ourID;
            board.totalBoxes = boxCount;
        }
        turn++;
    }
};
#endif //HYPERSONIC_INPUTPARSER_H
