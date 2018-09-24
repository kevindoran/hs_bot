#include <iostream>
#include <ctime>
#include <stdlib.h>

#include "Position.h"
#include "Board.h"
#include "InputParser.h"
#include "AnnealingBot.h"
#include "Bot.h"
#include <chrono>

using namespace std;


int main() {
    srand(time(NULL));
    InputParser ip(cin);
    ip.init();
    Board board;
    Bot<4> bot4(ip.ourID);
    Bot<4> bot4Duel(ip.ourID);
    Bot<5> bot5(ip.ourID);
    Bot<6> bot6(ip.ourID);
    while (1) {
        long long startTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
        ip.update(board);
//        for(int i = 0; i < Board::HEIGHT; i++) {
//            for(int j = 0; j < Board::WIDTH; j++) {
//                cerr << board.explodeTurn[j + i * Board::WIDTH] << " ";
//            }
//            cerr << endl;
//        }

//        cerr << "Board: " << endl << board;
        int targetTile;
        string action = "MOVE ";
        int boxCount = 0;
        int minBombTimer = 8;
        for(int i = 0; i < Board::TILE_COUNT; i++) {
            if(board.isBox(i)) boxCount++;
        }
        for(int i = 0; i < board.bombCount; i++) {
            minBombTimer = min(minBombTimer, board.bombs[i].explodeTurn - board.turn);
        }

        // Our model of the enemies
        for(int i = 0; i < board.playerCount; i++) {
            if(i != board.US && board.players[i].isAlive() && board.players[i].bombsAvailable) {
                board.placeBomb(i);
            }
        }

        pair<int, int> cePair = BoardStats::closestPlayer(board, board.US);
        bool disconnected = cePair.second == -1 && boxCount > 25;
        bool enemyClose =  cePair.second < 8;
        pair<int, bool> toMove;
        cerr << "Score pos: " << BoardStats::scorePos(board, board.US);
        int pos = 0;
        for(int i=0; i < board.playerCount; i++) {
            if(i == board.US) continue;
            if(board.players[board.US].boxesDestroyed - board.players[i].boxesDestroyed <= boxCount) {
                pos++;
            }
        }
        if(boxCount == 0 || pos == 0) {
            cerr << "End game." << endl;
            pair<int, int> closestP = BoardStats::closestPlayer(board, board.US);
            cerr << "Closest: " << " (" << closestP.first << ", " << closestP.second << ")" << endl;
            Bot<3> bot3(ip.ourID);
            if(pos < board.aliveCount - 1) {
                cerr << "Flee " << endl;
                bot3.flee = true;
                bot3.fleeFrom = board.players[closestP.first].tile;
            } else {
                bot3.fight = true;
            }
            toMove = bot3.move(board);
        } else {
            if (disconnected && !(board.bombCount > 3 && minBombTimer == 1)) {
                bot6.distEnabled = false;
                cerr << "Disconnected" << endl;
                toMove = bot6.move(board);
            } else {
                if (false) {
                    cerr << "Modelling" << endl;
                    Bot<3> enemyBot(cePair.first);
                    enemyBot.distEnabled = false;
                    enemyBot.move(board);
                    // Now ours
                    bot4Duel.distEnabled = true;
                    bot4Duel.setEnemy(cePair.first, enemyBot.best, 4);
                    toMove = bot4.move(board);
                } else {
                    if (boxCount < 10 || (board.bombCount > 3 && minBombTimer == 1)) {
                        cerr << "Caution, low box count. 4 dist." << endl;
                        bot4.distEnabled = true;
                        toMove = bot4.move(board);
                    } else {
                        cerr << "Normal (5 dist)." << endl;
                        bot5.distEnabled = true;
                        toMove = bot5.move(board);
                    }
                }
            }
        }

        targetTile = board.adjTile(board.players[board.US].tile, toMove.first);
        if(toMove.second) {
                action = "BOMB ";
        }
        Position target = Board::toPosition(targetTile);
        cout << action << " " << target.x << " " << target.y << endl;
        long long endTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
        cerr << "Runtime: " << endTime-startTime << endl;
    }
}

