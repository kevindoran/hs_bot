#ifndef HYPERSONIC_MECHANICS_H
#define HYPERSONIC_MECHANICS_H

#include <unordered_set>
#include "Position.h"
#include "Board.h"

using namespace std;

struct TilePlayer {
    int tile;
    int player;
    TilePlayer(int tile, int player):tile(tile), player(player) {}
    TilePlayer() {}
};

class BoardStats {
public:
//    BoardStats(const Board& b, int player) {
//        calculatePlayerDist(b, player);
//        calculateRemainingBoxes(b);
//        calculateClosestCount(b, player);
//    }

    static int totalPlayerDist(const Board &b, int player) {
        int ans = 0;
        for(int i = 0; i < b.playerCount; i++) {
            if(i == player) continue;
            ans += b.dist(b.players[player].tile, b.players[i].tile);
        }
        return ans;
    }

    static int closestPlayerDist(const Board &b, int player) {
        int ans = 2000;
        for(int i = 0; i < b.playerCount; i++) {
            if(i = player) continue;
            ans = min(ans, b.dist(b.players[player].tile, b.players[i].tile));
        }
        return ans;
    }

    static int remainingBoxes(const Board& b) {
        int totalScored = 0; // Ignoring double score.
        for(int t = 0; t < Bomb::TIMEOUT; t++) {
            for (int i = 0; i < b.playerCount; i++) {
                totalScored += b.scoresM[t][i];
            }
        }
        return b.totalBoxes - totalScored;
    }


    static int seen[Board::TILE_COUNT];
    static TilePlayer qu[Board::TILE_COUNT];
    static int quInt[Board::TILE_COUNT];
    static int neigh[4];

    static pair<int, int> closestPlayer(const Board& b, int fromPlayer) {
        memset(seen, 0, sizeof(seen));
        int qIn = 0;
        int qOut = 0;
        int neighCount = 0;
        int steps = 1;
        quInt[qIn++] = b.players[fromPlayer].tile;
        while(qOut < qIn) {
            int size = qIn - qOut;
            for(int i = 0; i < size; i++) {
                int t = quInt[qOut++];
                b.neighbours(t, neigh, &neighCount);
                for(int j = 0; j < neighCount; j++) {
                    int n = neigh[j];
                    if(n == Board::INVALID_TILE || seen[n]) continue;
                    seen[n] = true;
                    for(int p = 0; p < b.playerCount; p++) {
                        if (p == fromPlayer) continue;
                        if (b.players[p].tile == n) {
                            return pair<int, int>(p, steps);
                        }
                    }
                    if(b.isFree(n)) {
                        quInt[qIn++] = n;
                    }
                }
            }
            steps++;
        }
        return {-1, -1};
    };

    static int stepsToClosestBox(const Board& b, int player) {
        memset(seen, 0, sizeof(seen));
        int qIn = 0;
        int qOut = 0;
        int count = 0;
        int neighCount = 0;
        quInt[qIn++] = b.players[player].tile;
        while(qOut < qIn) {
            int size = qIn - qOut;
            for(int i = 0; i < size; i++) {
                int t = quInt[qOut++];
                b.neighbours(t, neigh, &neighCount);
                for(int j = 0; j < neighCount; j++) {
                    int n = neigh[j];
                    if(n == Board::INVALID_TILE || seen[n]) continue;
                    seen[n] = true;
                    if(b.isBox(n)){
                        bool exp = false;
                        for(int f = 0; f < Bomb::TIMEOUT; f++) {
                            if(b.explodeM[f][n]) {
                                exp = true;
                                break;
                            }
                        }
                        if(!exp) {
                            return count;
                        }
                    } else if(b.isFree(n)) {
                        quInt[qIn++] = n;
                    }
                }
            }
            count++;
        }
        return -1;
    }
    static int closestCount(const Board& b, int player) {
        memset(seen, 0, sizeof(seen));
        int qIn = 0;
        int qOut = 0;
        int count = 0;
        int neighCount = 0;
//        queue<TilePlayer> queue;
        for(int i = 0; i < b.playerCount; i++) {
            if(i == player) continue;
//            queue.push(TilePlayer(b.players[i].tile, i));
            qu[qIn++] = TilePlayer(b.players[i].tile, i);
        }
        // I want to maximize the number of squares that I am the closest to (not equal closest).
        // So add me to the queue last.
//        queue.push(TilePlayer(b.players[player].tile, player));
        qu[qIn++] = TilePlayer(b.players[player].tile, player);
        int expMin = 0;
        int expMax = 0;
        int expCount = 0;
        while(qOut < qIn) {
            int size = qIn - qOut;
            for(int i = 0; i < size; i++) {
                TilePlayer tp = qu[qOut++];
                // Starting square will be viewed twice.
//                if(seen.count(tp.tile)) continue;
                b.neighbours(tp.tile, neigh, &neighCount);
                for(int j = 0; j < neighCount; j++) {
                    int n = neigh[j];
                    if(seen[n]) continue;
                    seen[n] = true;
                    if(b.isBox(n)) {
                        b.minMaxCount(n, &expMin, &expMax, &expCount);
                        if(tp.player == player &expCount == 0) count++;
                    } else if(b.isFree(n)){
//                        queue.push(TilePlayer(n, tp.player));
                        qu[qIn++] = TilePlayer(n, tp.player);
                    }
                }
            }
        }
        return count;
    }

    static int scorePos(const Board& b, int player) {
        int pScore = score(b, player);
        int standing = 0;
        for(int i = 0; i < b.playerCount; i++) {
            if(i == player) continue;
            if(b.players[i].isAlive() && score(b, i) > pScore) {
                standing++;
            }
        }
        return standing;
    }

    static int score(const Board& b, int player) {
        int score = b.players[player].boxesDestroyed;
        for(int i = 0; i < Bomb::TIMEOUT; i++) {
            score += b.scoresM[i][player];
        }
    }

//    int closestCount;
//    int remainingBoxCount;
//    int totalPlayerDist;
//    int minPlayerDist;

};

class Mechanics {
public:

    pair<int, int> closestPosToBomb(Board board, int from, const unordered_set<int> &boxes) {
        pair<int, int> ans = pair<int, int>(Position::INVALID_IDX, Position::INVALID_IDX);
        int min = std::numeric_limits<int>::max();
        for (auto &box : boxes) {
            vector<int> neigh = board.neighbours(box);
            for (auto &neighbour : neigh) {
                if (board.dist(from, neighbour) <= min) {
                    min = board.dist(from, neighbour);
                    ans = pair<int, int>(box, neighbour);
                }
            }
        }
        return ans;
    }

// Returns -1 if none found.
    int maximizeBlast(Board board, int player, int steps, int blastLength) {
        int max = 0;
        int pos = -1;
        for (auto t : board.reachableFrom(board.playerTile(player), steps)) {
            board.stepForward(board.pathDist(board.playerTile(player), t)); // +1 ?
            int score = blastCount(board, Board::toPosition(t), blastLength);
            cerr << "score : " << score << "  at: " << t << endl;
            if (score > max) {
                max = score;
                pos = t;
            }
        }
        return pos;
    }


// Only works for single bombs, currently.
    int blastCount(Board board, Position bombPos, int blastLength) {
        board.stepForward(Bomb::TIMEOUT);
        int boxCount = 0;
        for (int dir = Position::RIGHT; dir <= Position::UP; dir++) {
            for (int i = 1; i <= blastLength; i++) {
                int t = board.tileAt(bombPos, dir, i);
                if (board.tiles[t] == Board::INVALID_TILE || board.tiles[t] == Board::WALL) {
                    break;
                }
                if (board.isBox(t)) {
                    boxCount++;
                    break;
                }
            }
        }
        return boxCount;
    }

    int closestBoxSide(Board board, int fromTile) {
        unordered_set<int> seen;
        queue<int> q;
        queue<int> fromQueue;
        q.push(fromTile);
        fromQueue.push(fromTile);
        while (!q.empty()) {
            int size = q.size();
            for (int i = 0; i < size; i++) {
                int t = q.front();
                int tPrev = fromQueue.front();
                q.pop();
                fromQueue.pop();
                if (seen.count(t)) continue;
                seen.insert(t);
                if (board.isBox(t)) {
                    return tPrev;
                }
                for (auto n : board.neighbours(t)) {
                    q.push(n);
                    fromQueue.push(t);
                }
            }
        }
        return -1;
    }

    int stepsToNearestBox(const Board &b, int player) {
        queue<int> queue;
        unordered_set<int> seen;
        queue.push(b.players[player].tile);
        seen.insert(b.players[player].tile);
        int size;
        int steps = 0;
        while (!queue.empty()) {
            size = queue.size();
            for (int i = 0; i < size; i++) {
                int t = queue.front();
                queue.pop();
                if (b.isBox(b.tiles[t])) {
                    return steps;
                }
                for (auto n : b.freeNeighbours(t)) {
                    queue.push(n);
                }
            }
            steps++;
        }
        return -1;
    }
};


#endif //HYPERSONIC_MECHANICS_H
