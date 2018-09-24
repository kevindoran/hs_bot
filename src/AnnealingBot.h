
#ifndef HYPERSONIC_ALL_ANNEALINGBOT_H
#define HYPERSONIC_ALL_ANNEALINGBOT_H

#include <cmath>
#include <chrono>
#include <cstring>
#include <cstdlib>

#include "Board.h"
#include "OnlineMedian.h"

using namespace std;

struct Move {
    int dir;
    bool bomb;

    Move() {}

    Move(int dir, bool bomb) : dir(dir), bomb(bomb) {}
};

class SimBot {
public:
    virtual Move move(Board& b) = 0;
    virtual void setTurn(int turn) {};
    virtual int player() = 0;
};

class MinimalBot: public SimBot {
    int player_;

public:
    MinimalBot() {}
    MinimalBot(int player) : player_(player) {}

    Move move(Board& b) {
        return Move(Position::Dir::NONE, false);
    }

    int player() {
        return player_;
    }
};

class CustomAI : public SimBot {
    const Move* moves;
    int turn = 0;
    int player_;

public:
    CustomAI(int player, const Move moves[], int startFromTurn) :
            player_(player), moves(moves), turn(startFromTurn) {}

    void setTurn(int fromTurn) {
        turn = fromTurn;
    }

    Move move(Board& b) {
        return moves[turn++];
    }

    int player() {
        return player_;
    }
};

struct ScoreFactors {
    double boxesDestroyed;
    double boxDepreciation;
    double rangePU;
    double countPU;
    double enemyDeath;
    double victory;
    double defeat;
};

static ScoreFactors defaultFactors = {
    1.0,        // boxesDestroyed
    0.875,      // boxDepreciation
    0.1,        // rangePU
    0.1,        // countPU
    50,         // enemyDeath
    200,        // victory
    -200        // defeat
};

template<int TURNS, int PLAYERS>
class AnnealingBot {
public:
    ScoreFactors sFactors = defaultFactors;
private:
    static constexpr float maxScore = 10000;
    static constexpr float minScore = 0;
    // Loop control and timing.
    static const int reevalPeriodMilli = 4;
    static const int timeBufferMilli = 1;
    static constexpr float initTemp = 23000.0; // TODO
    static constexpr float initCoolingFraction = 0.95;
    static constexpr float startAcceptanceRate = 0.96;
    static constexpr float endAcceptanceRate = 0.00000000001; // TODO
    static constexpr float stepsVsCoolRatio = 1.3;
    static const int initCoolingSteps = 160; // TODO
    static const int initStepsPerTemp = 140; // TODO
    long long startTime;
    static const int UNSET = -1;
    long allocatedTime = UNSET;
    long long lastUpdateTime;
    double diffSum = 0;
    int simCount = 0;
    int tunnelCount = 0;
    int nonTunnelCount = 0;
    int simsSinceUpdate = 0;
    int coolCount = 0;
    int coolingSteps = initCoolingSteps;
    int coolingIdx = 0;
    int stepsPerTemp = initStepsPerTemp;
    float currentTemp = initTemp;
    float coolingFraction = UNSET;//initCoolingFraction;
    bool toDeleteEnemy = false;
    // SD & mean
    float mean;
    double M2;
    OnlineMedian<float> onlineMedian;

    int player;
    SimBot **enemyBots;
    Move previousSolution[TURNS];
    bool hasPrevious = false;
    Board simHistory[TURNS + 1];

    long long getTimeMilli() {
        long long ms = chrono::duration_cast<chrono::milliseconds>(
                chrono::system_clock::now().time_since_epoch()).count();
        return ms;
    }

    void updateLoopControl() {
        if (allocatedTime == UNSET) return;
        long long timeNow = getTimeMilli();
        long elapsed = timeNow - lastUpdateTime;
        long timeRemaining = allocatedTime - (timeNow - startTime) - timeBufferMilli;
        if (timeRemaining < 0) {
            coolingSteps = 0;
            stepsPerTemp = 0;
            cerr << "Tunnel %: " << (float) tunnelCount / (tunnelCount + nonTunnelCount) << endl;
        } else if (elapsed > reevalPeriodMilli) {
            // The update timer has elapsed, or we are on our second loop, so need to create a better estimate of
            // start temp, end temp and cooling fraction.
            lastUpdateTime = timeNow;
            float simRate = simsSinceUpdate / elapsed;
            int simsRemaining = simRate * timeRemaining;
            // A*2A = C
            // A = sqrt(C/2)
            coolingSteps = sqrt((simCount + simsRemaining) / (stepsVsCoolRatio));
            stepsPerTemp = coolingSteps * stepsVsCoolRatio;
            //            cerr << "Tunnel %: " << (float) tunnelCount / (tunnelCount + nonTunnelCount) << endl;
            tunnelCount = 0;
            nonTunnelCount = 0;
            simsSinceUpdate = 0;
            //            cerr << "alpha: " << coolingFraction << endl;
            //            cerr << "Steps per temp: " << stepsPerTemp << endl;
        }
        if (elapsed > reevalPeriodMilli || coolingIdx == 1) {
            // T0 = -sd/ln(startAcceptanceRate)    [from startAcceptanceRate = exp(-sd/T0)]
            float SD = sqrt(M2 / simCount);
            //            cerr << "SD: " << SD << endl;
            float median = onlineMedian.median();
            //            cerr << "mean: " << mean << "    median: " << median << endl;
            float startTemp = -median / log(startAcceptanceRate);
            float endTemp = -median / log(endAcceptanceRate);
            coolingFraction = pow(endTemp / startTemp, 1.0 / (coolingSteps * 0.6));
            currentTemp = startTemp * pow(coolingFraction, coolingIdx);
            //            cerr << "Current Temp: " << currentTemp << "    coolingIdx: " << coolingIdx << endl;
            //            cerr << "Cooling Fraction: " << coolingFraction  << "     Cooling steps: " << coolingSteps << endl;
        }
    }

    void init() {
        startTime = getTimeMilli();
        lastUpdateTime = startTime;
        currentTemp = initTemp;
        coolingSteps = allocatedTime == UNSET ? initCoolingSteps : allocatedTime * 1.2;
        coolingFraction = initCoolingFraction;
        stepsPerTemp = initStepsPerTemp;
        simCount = 0;
        tunnelCount = 0;
        nonTunnelCount = 0;
        simsSinceUpdate = 0;
        coolingIdx = 0;
        diffSum = 0;
        coolCount = 0;
        mean = 0;
        //        M2 = 0;
        onlineMedian = OnlineMedian<float>();
    }

public:
    AnnealingBot() {}

    AnnealingBot(int player, long allocatedTimeMilli) : player(player), allocatedTime(allocatedTimeMilli) {}

    void setEnemyAI(SimBot* enemyAI[]) {
        enemyBots = enemyAI;
    }


    Move random() {
        Move m;
        m.dir = rand() % (Position::DIR_COUNT);
        if(rand() / RAND_MAX > 0.5) {
            m.bomb = true;
        } else {
            m.bomb = false;
        }
        return m;
    }

    // Only produces output in the space of valid outputs for this turn.
    int dir[Position::DIR_COUNT];
    bool randomEdit(Move& m, int turn) {
        double flip = rand() / RAND_MAX;
        if(flip > 0.5 && ((!m.bomb && simHistory[turn].players[player].bombsAvailable) || m.bomb)) {
            m.bomb = !m.bomb;
        } else {
            int count = 0;
            int n;
            for (int i = Position::NONE; i >= Position::RIGHT; i--) {
                if(i == m.dir) continue;
                const Position p = Board::toPosition(simHistory[turn].players[player].tile);
                n = Board::tileAt(p, i, 1);
                if (n == Board::INVALID_TILE || simHistory[turn].tiles[n] == Board::WALL) continue;
                if (simHistory[turn].willBeFree(n, 1, i)) {
                    dir[count++] = i;
                }
            }
            if(count > 0) {
                int sw = rand() % count;
                m.dir = dir[sw];
                return true;
            } else {
                return false;
            }
        }
    }

    void randomSolution(Move sol[]) {
        for(int i = 0; i < TURNS; i++) {
            sol[i] = random();
        }
    }

    // Use sim history
    double score() {
        const Board& endBoard = simHistory[TURNS];
        const Board& startBoard = simHistory[0];
        double score = 0;
        // Victory & Defeat
        // Should be broken up to determine the placing. 3rd is better than 4th.
        if(!endBoard.players[player].isAlive()) {
            return sFactors.defeat;
        } else if(endBoard.aliveCount == 1) {
            // Only player left is us.
            return  sFactors.victory;
        } else {
            score += (Board::playerCount - endBoard.aliveCount) * sFactors.enemyDeath;
        }

        // Continued survival
        int turnsLeftAlive = endBoard.survivalTurns(player, Bomb::TIMEOUT);
        if(turnsLeftAlive < Bomb::TIMEOUT) {
            return sFactors.defeat * (Bomb::TIMEOUT - turnsLeftAlive) / Bomb::TIMEOUT;
        }

        // Box counts
        // Haven't yet given any depreciation to the pre-survival box counts.
        int beforeBoxCount = startBoard.players[player].boxesDestroyed;
        int afterBoxCount = endBoard.players[player].boxesDestroyed;
        score += (afterBoxCount - beforeBoxCount) * sFactors.boxesDestroyed;

        for(int i = 0; i < Bomb::TIMEOUT; i++) {
            score += sFactors.boxesDestroyed * endBoard.scoresM[i][player] * pow(sFactors.boxDepreciation, i + 1);
        }
        return score;
    }

    int simulate(SimBot* ourSim, SimBot* enemySims[], int startFromTurn) {
        // Assuming the board has had stepForward called for the very first board.
        Move m;
        for(int i = startFromTurn; i < TURNS; i++) {
            simHistory[i+1]  = simHistory[i];
            // Us move first? Place our bomb first?
            if(simHistory[i+1].players[player].isAlive()) {
                m = ourSim->move(simHistory[i + 1]);
                if(simHistory[i+1].canMove(player, m.dir)) {
                    if(m.bomb) {
                        simHistory[i+1].placeBomb(player);
                    }
                    simHistory[i+1].move(player, m.dir);
                } else {
                    return i;
                }
            }
            for(int p = 0; p < PLAYERS - 1; p++) {
                if(simHistory[i+1].players[enemySims[p]->player()].isAlive()) {
                    m = enemySims[p]->move(simHistory[i+1]);
                    if(simHistory[i+1].canMove(p, m.dir)) {
                        if(m.bomb) {
                            simHistory[i+1].placeBomb(p);
                        }
                        simHistory[i+1].move(p, m.dir);
                    } else {
                        simHistory[i+1].move(p, Position::Dir::NONE);
                    }
                }
            }
            simHistory[i+1].stepForward(1);
        }
        return TURNS-1;
    }

    double score(const Move solution[], int startFromTurn) {
        CustomAI *customAI = new CustomAI(player, solution, startFromTurn);
        for(int i = 0; i < PLAYERS - 1; i++) {
            enemyBots[i]->setTurn(startFromTurn);
        }
        int validUntil = simulate(customAI, enemyBots, startFromTurn);
        if(validUntil < (TURNS - 1)) {
            return -10000; // Hacky. Just needs to be big number.
        }
        delete(customAI); // WHY USE HEAP?
        return -score();
    }

    void train(Board board, Move solution[TURNS]) {
        board.stepForward(1);
        init();
        simHistory[0] = board;
        double exponent;
        double merit, flip;
        if(hasPrevious) {
            for (int i = 0; i < TURNS - 1; i++) {
                solution[i] = previousSolution[i + 1];
            }
            randomEdit(solution[TURNS - 1], 0);
        } else {
            randomSolution(solution);
        }
        double currentScore = score(solution, 0);
        double bestScore = currentScore;
        double updated_score;
        double startScore;
        double delta;
        int toEdit = 0;
        Move saved;
        Move best[TURNS];
//        memcpy(best, solution, TURNS * sizeof(Move));
        // SD & mean
        mean = 0;
        double d = 0;
        bool editSuccess;

        for(; coolingIdx <= coolingSteps; coolingIdx++) {
            updateLoopControl();
            startScore = currentScore;
            for(int j = 1; j <= stepsPerTemp; j++) {
                // Make edits to one turn of solution.
                toEdit = (toEdit + 1) % TURNS;//rand() % TURNS;
                saved = solution[toEdit];
                editSuccess = randomEdit(solution[toEdit], toEdit);//, TURNS - toEdit, ((float)coolingIdx)/coolingSteps);
                while(!editSuccess) {
                    toEdit = (toEdit + 1) % TURNS;//rand() % TURNS;
                    editSuccess = randomEdit(solution[toEdit], toEdit);
                }
                updated_score =  score(solution, toEdit);
                if(updated_score < 0) {
//                cerr << "Score below zero  " << updated_score << endl;
                }
                if(updated_score < bestScore) {
                    bestScore = updated_score;
                    memcpy(best, solution, TURNS * sizeof(Move));
                }
                // Stats
                delta = updated_score - currentScore;
                if(delta > 0) {
                    d = delta - mean;
                    // simCount is updated at the end of the loop, so need to add 1 here.
                    mean += d / (simCount + 1);
                    onlineMedian.add(delta);
//                M2 += d * (delta - mean);
//                if (!(M2 >= 0 || M2 <= 0)) {
//                    cerr << "Updated score: " << updated_score << endl;
//                    cerr << "M2: " << M2 << endl;
//                    cerr << "Sim count: " << simCount << endl;
//                    cerr << "Mean: " << mean << endl;
//                    cerr << "d: " << d << endl;
//                    cerr << "(coolIdx, j) " << "(" << coolingIdx << ", " << j << ")" << endl;
//                }
                }
                if(delta > 0) diffSum += delta;
                exponent = (-delta /*/currentScore*/) / (currentTemp);
                merit = exp(exponent);
                if(merit > 1.0) {
                    merit = 0.0;
                }
                // Pure hill climbing on the last turn (but allow transitions to state with equal score).
                if(merit != 1 && (coolingIdx == coolingSteps || coolingSteps == 0)) {
                    merit = 0;
                }
                if(delta < 0) {
                    currentScore += delta;
                } else {
                    // Used for random variable with mean 0.5.
                    flip = ((float) rand() / (RAND_MAX));
                    if(merit > flip) {
                        currentScore += delta;
                        tunnelCount++;
                    } else {
                        nonTunnelCount++;
                        // transition back.
                        solution[toEdit] = saved;
                    }
                }
                simCount++;
                simsSinceUpdate++;
            }
//        if(currentScore - startScore < 0.0) {
//            currentTemp /= coolingFraction;
//        }
            coolCount++;
            currentTemp *= coolingFraction;
        }
//    cerr << "End score: " << currentScore << endl;
        cerr << "Sim count:" << simCount << endl;
//    cerr << "Average score diff: " << diffSum / simCount << endl;
        memcpy(solution, best, TURNS*sizeof(Move));
//    cerr << "Current (pos, vel)   " << podsToTrain[0].pos << "   " << podsToTrain[0].vel << endl;
//    cerr << "Moving: (thrust, angle)  " << " (" << solution[0].o1.thrust << ", " << physics.radToDegrees(solution[0].o1.angle) << ")   " << endl
//         << "Expecting state: (pos, vel, cp)   " << next.pos << "   " << next.vel << "   " << next.nextCheckpoint << endl;
        memcpy(previousSolution, solution, TURNS*sizeof(Move));
        hasPrevious = true;
    }

    Move move(const Board& board) {
        Move solution[TURNS];
        train(board, solution);
        return solution[0];
    };
};

    #endif //HYPERSONIC_ALL_ANNEALINGBOT_H




