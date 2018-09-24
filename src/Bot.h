#ifndef HYPERSONIC_ALL_BOT_H_H
#define HYPERSONIC_ALL_BOT_H_H

#include <cmath>
#include <limits>

#include "Board.h"
#include "Mechanics.h"
#include "AnnealingBot.h"

using namespace std;

template<int MAX_DEPTH>
class Bot {
    static constexpr double boxScore = 1;
    static double depreciationM[16];
public:
    static constexpr double boxSF = 1;
    static constexpr double boxDepreciation = 0.875;
    static constexpr double powerupSF = 0.1;
    static constexpr double bombsAvailableSF = 0.125;
    static constexpr double distSF = 0.1;
    static constexpr double closestSF = 0.005;

    Move best[MAX_DEPTH];
    Move current[MAX_DEPTH];
    Move* enemyPreset;
    int enemyMoveCount = 0;
    int enemyPlayer;
    int player;
    double bestScore;
    bool distEnabled = true;
    bool fight = false;
    bool flee = false;
    int fleeFrom = 0;

    Bot(int player) : player(player) {}

    void update(double curScore) {
        if(curScore > bestScore) {
            // TODO: why sizeof(bestScore)? This isn't copying the whole move.
            memcpy(best, current, sizeof(bestScore));
            bestScore = curScore;
        }
    }

    void setEnemy(int player, Move* moves, int moveCount) {
        enemyPlayer = player;
        enemyPreset = moves;
        enemyMoveCount = moveCount;
    }

    void score(Board b, int depth, float curScore) {
        if(current[depth-1].bomb) {
            b.placeBomb(player);
            if(depth == 1 && !flee) {
                for(int i = 0; i < b.playerCount; i++) {
                    if(i == player || !b.players->isAlive()) continue;
                    int turnsLeft = b.survivalTurns(i, Bomb::TIMEOUT);
                    curScore += (15 * (Bomb::TIMEOUT - turnsLeft));

                }
            }
        }
        b.move(player, current[depth-1].dir);
//        if(enemyMoveCount >= depth) {
//            if(enemyPreset[depth-1].bomb) {
//                b.placeBomb(enemyPlayer);
//            }
//            b.move(enemyPlayer, enemyPreset[depth-1].dir);
//        }
        int beforeBoxCount = b.players[player].boxesDestroyed;
        b.stepForward(1);
        if(!b.players[player].isAlive()) {
            update(-12000);
            return;
        }
//        bool occupied = false;
//        for(int i = 0; i < b.playerCount; i++) {
//            if(i == player) continue;
//            if(b.players[i].tile == b.players[player].tile) {
//                occupied = true;
//                break;
//            }
//        }
//        if(occupied) {
//            curScore -= 100 * (MAX_DEPTH - depth + 1);
//        }
        int afterBoxCount = b.players[player].boxesDestroyed;
        curScore += boxSF * (afterBoxCount - beforeBoxCount) * depreciationM[depth];
        if(depth == MAX_DEPTH) {
            const int max = 8;
            int turnsLeftAlive = b.survivalTurns(player, max);
            if(turnsLeftAlive < max) {
                curScore += (-1000 * (max - turnsLeftAlive));
            }
            // Evaluate
            if(flee) {
                // Move away and don't bomb.
                update(curScore + 10*b.dist(b.players[player].tile, fleeFrom) + b.players[player].bombsAvailable);
                return;
            }
            if(fight) {
                pair<int, int> cp = BoardStats::closestPlayer(b, player);
                int score  = - cp.second;
                int turnsLeft = b.survivalTurns(cp.first, max);
                score -= 100 * (max - turnsLeft);
                update(curScore + score);
                return;
            }
            curScore += powerupSF * b.players[player].totalBombs * depreciationM[depth];
            curScore += powerupSF * b.players[player].range * depreciationM[depth];
            curScore += bombsAvailableSF * b.players[player].bombsAvailable;
            for(int i = 0; i < Bomb::TIMEOUT; i++) {
                curScore += boxSF * b.scoresM[i][player] * depreciationM[boxDepreciation, depth + i + 1]; // i + 1?
            }

            if(distEnabled) {
                int boxDist = BoardStats::stepsToClosestBox(b, player);
                if (boxDist != -1) {
                    curScore -= closestSF * boxDist;
                }
            }
            update(curScore);
//            int rem = BoardStats::remainingBoxes(b);
//            if(BoardStats::closestPlayerDist(b, player) <= 2) {
//                curScore *= 0.95;
//            }
//            if(rem == 0) {
//                const int scorePos = BoardStats::scorePos(b, player);
//                if(scorePos == 0) {
//                    curScore += distSF * BoardStats::totalPlayerDist(b, player);
//                } else if(scorePos == b.playerCount) {
//                    curScore -= distSF * BoardStats::totalPlayerDist(b, player);
//                }
//            } else if(rem > 5) {
//                curScore += closestSF * BoardStats::closestCount(b, player);
//            }
        } else {
            for(int d = Position::RIGHT; d <= Position::NONE; d++) {
                if(!b.canMove(player, d)) continue;
                bool occupied = false;
                current[depth+1-1].dir = d;
                if(b.players[player].bombsAvailable && !b.tiles[b.players[player].tile] != Board::BOMB) {
                    current[depth+1-1].bomb = true;
                    score(b, depth + 1, curScore);
                }
                current[depth+1-1].bomb = false;
                score(b, depth + 1, curScore);
            }
        }
    }

    pair<int,bool> move(Board b) {
        bestScore = -std::numeric_limits<double>::infinity();
        b.stepForward(1);
        if(!b.players[player].isAlive()) return {0, 0};
        int maxDir = -1;
        bool bomb = false;
        for (int d = Position::RIGHT; d <= Position::NONE; d++) {
            if (!b.canMove(player, d)) continue;
            current[0].dir = d;
            current[0].bomb = false;
            score(b, 1, 0);
            if (b.players[player].bombsAvailable && !flee) {
                current[0].bomb = true;
                score(b, 1, 0);
            }
        }
        cerr << "Score: " << bestScore << endl;
        return pair<int, bool>(best[0].dir, best[0].bomb);
    }

    static pair<int, bool> moveTest(Board b, int player, int depth) {
        AnnealingBot<6,2> ab(750, 0);
        MinimalBot enemyAI(1);
        SimBot* allEnemyAI[] {&enemyAI};
        ab.setEnemyAI(allEnemyAI);
        Move move = ab.move(b);
        return pair<int, bool>(move.dir, move.bomb);
    };
};

template<int MAX_DEPTH>
double Bot<MAX_DEPTH>::depreciationM[16] = {
        0,
        Bot::boxDepreciation,
        Bot::boxDepreciation * Bot::boxDepreciation,
        Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation,
        Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation,
        Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation,
        Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation,
        Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation,
        Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation,
        Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation,
        Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation,
        Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation,
        Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation,
        Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation,
        Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation,
        Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation * Bot::boxDepreciation
};
    #endif //HYPERSONIC_ALL_BOT_H_H
