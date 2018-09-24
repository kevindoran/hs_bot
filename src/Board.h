#ifndef HYPERSONIC_BOARD_H
#define HYPERSONIC_BOARD_H

#include <vector>
#include <queue>
#include <unordered_set>
#include <list>
#include <limits>
#include <cstdlib>
#include <sstream>
#include <cstring>
#include "Position.h"

using std::vector;
using std::priority_queue;
using std::abs;
using std::unordered_set;
using std::list;
using std::queue;


struct Player {
    static const int DEAD = -1;
    int tile;
    int range;
    int totalBombs;
    int bombsAvailable;
    int boxesDestroyed;

    Player() : tile(DEAD), boxesDestroyed(0) {}

    Player(int tile) : tile(tile), range(2), totalBombs(1), boxesDestroyed(0), bombsAvailable(1) {}

    bool isAlive() const {
        return tile != DEAD;
    }

    void setDead() {
        tile = DEAD;
    }
};

struct PowerUp {
    enum Type {RANGE = 1, COUNT};
    int tile;
    int type;

    PowerUp(int tile, int type) : tile(tile), type(type) {}
};

struct Bomb {
    static const int TIMEOUT = 8;
    int tile;
    int blastLength;
    int explodeTurn;
    int owner;
    static const int NOT_LIVE = -1;

    Bomb() : explodeTurn(NOT_LIVE) {}

    Bomb(int tile, int blastLength, int owner, int turn) :
            tile(tile), blastLength(blastLength), explodeTurn(turn), owner(owner) {}

    bool isLive() {
        return explodeTurn != NOT_LIVE;
    }

    void disable() {
        explodeTurn = NOT_LIVE;
    }
};

struct Explosion {
    int tile;
    int turn;
    int owner;

    Explosion(int tile, int turn, int owner) : tile(tile), turn(turn), owner(owner) {}
};

struct ExplosionComparator {
    bool operator()(Explosion const& e1, Explosion const& e2) {
        return e1.turn < e2.turn;
    }
};

struct CompareCountown {
    bool operator()(Bomb const& b1, Bomb const& b2) {
        return b1.explodeTurn < b2.explodeTurn;
    }
};

struct PosMap {
    Position m[13 * 11 * Bomb::TIMEOUT]; // Hacky. How to avoid circular dependency.
    PosMap();
};

class Board {
public:
    static const int WIDTH = 13;
    static const int HEIGHT = 11;
    static const int TILE_COUNT = WIDTH * HEIGHT;
    static const int MAX_PLAYERS = 4;
    static const int EMPTY = '.';
    static const int BOMB = 'B';
    static const int BOX = '0';
    static const int BOMB_RANGE_BOX = '1';
    static const int BOMB_COUNT_BOX = '2';
    // Don't change the order of the next three (BR_B_D, BC_B_D, B_D) as used in step forward.
    static const int BOX_DESTROYED = '3';
    static const int BOMB_RANGE_BOX_DESTROYED = '4';
    static const int BOMB_COUNT_BOX_DESTROYED = '5';
    static const int BOMB_RANGE_PU = 'r';
    static const int BOMB_COUNT_PU = 'c';
    static const int WALL = 'X';
    static const int NOT_SET = 0;
    static const int INVALID = 'Z';
    static const int INVALID_TILE = -1;

    int aliveCount;
    static int playerCount;
    static int US;
    static int totalBoxes;
    int turn;
    char tiles[TILE_COUNT];
//    int explodeTurn[TILE_COUNT] = {0};
//    list<Bomb> bombs;
    bool explodeM[Bomb::TIMEOUT][TILE_COUNT];// = {0};
    int scoresM[Bomb::TIMEOUT][MAX_PLAYERS];// = {0};
    // Set these to char? Faster to copy? uint8_t?
//    int explodeOwner[TILE_COUNT];
//    int explodeRange[TILE_COUNT];
    // Unsafe to pickup item (for survival estimate)
    bool unsafe[TILE_COUNT]; // Bit manipulation with 10 ints?
    static bool hash[TILE_COUNT * Bomb::TIMEOUT];
    static PosMap positionMap;
//    Bomb bombs[TILE_COUNT];
//    list<int> bombList;
//    priority_queue<Explosion, vector<Explosion>, ExplosionComparator> explodeQueue;
    Player players[MAX_PLAYERS];
    static const int MAX_BOMB_COUNT = MAX_PLAYERS * (Bomb::TIMEOUT - 1);
    Bomb bombs[MAX_BOMB_COUNT];
    int bombCount = 0;

    Board() {}

    char& operator()(int y, int x) {
        return tiles[x + y*WIDTH];
    }

    int ourTile() const {
        return players[US].tile;
    }

    int playerTile(int player) const {
        return players[player].tile;
    }


    friend std::ostream& operator <<(std::ostream& out, const Board& b);


    static inline const Position& toPosition(int id) {
//    return Position(id / WIDTH, id % WIDTH);
        return positionMap.m[id];
    }

    static int tileAt(const Position& from, int dir, int steps) {
        int x = from.x + steps * Position::dx[dir];
        int y = from.y + steps * Position::dy[dir];
        if(x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
            return INVALID_TILE;
        } else {
            return x + y*WIDTH;
        }
    }

    static bool isValid(const Position& p) {
        return p.y >= 0 && p.y < HEIGHT && p.x >= 0 && p.x < WIDTH;
    }

    static inline int dist(const Position& a, const Position& b) {
        return abs(a.x - b.x) + abs(a.y - b.y);
    }

    static inline int dist(int a, int b) {
        return abs(a % WIDTH - b % WIDTH) + abs(a / WIDTH - b / WIDTH);
    }


    static inline int toID(const Position& pos) {
        return pos.x + WIDTH * pos.y;
    }

    static inline int toID(int y, int x) {
        return x + WIDTH * y;
    }

    inline vector<int> neighbours(int p) const  {
        vector<int> ans;
        if(p % WIDTH != 0) {
            ans.push_back(p - 1);
        }
        if(p % WIDTH != WIDTH - 1) {
            ans.push_back(p + 1);
        }
        if(p >= WIDTH) {
            ans.push_back(p - WIDTH);
        }
        if(p < WIDTH * (HEIGHT - 1)) {
            ans.push_back(p + WIDTH);
        }
        return ans;
    }

void neighbours(int t, int* ans, int* count) const {
    (*count) = 0;
    if(t % WIDTH != 0) {
        ans[(*count)++] = (t - 1);
    }
    if(t % WIDTH != WIDTH - 1) {
        ans[(*count)++] = (t + 1);
    }
    if(t >= WIDTH) {
        ans[(*count)++] = (t - WIDTH);
    }
    if(t < WIDTH * (HEIGHT - 1)) {
        ans[(*count)++] = (t + WIDTH);
    }
}

    vector<int> neighboursInc(int p) const {
        vector<int> ans;
        ans.push_back(p);
        if(p % WIDTH != 0) {
            ans.push_back(p - 1);
        }
        if(p % WIDTH != WIDTH - 1) {
            ans.push_back(p + 1);
        }
        if(p >= WIDTH) {
            ans.push_back(p - WIDTH);
        }
        if(p < WIDTH * (HEIGHT - 1)) {
            ans.push_back(p + WIDTH);
        }
        return ans;
    }


    inline vector<Position> neighbours(const Position& pos) const {
        vector<Position> ans;
        if(pos.y > 0) {
            ans.push_back(Position(pos.y-1, pos.x));
        }
        if(pos. y < HEIGHT - 1) {
            ans.push_back(Position(pos.y+1, pos.x));
        }
        if(pos.x > 0) {
            ans.push_back(Position(pos.y, pos.x - 1));
        }
        if(pos.x < WIDTH - 1) {
            ans.push_back(Position(pos.y, pos.x + 1));
        }
        return ans;
    }

    inline vector<int> freeNeighbours(int tile) const {
        vector<int> all = neighbours(tile);
        for(auto it  = all.begin(); it != all.end();) {
            if(!isFree(*it)) {
                it = all.erase(it);
            } else {
                ++it;
            }
        }
        return all;
    }


    bool isFree(int tile) const {
        return (tiles[tile] == EMPTY || tiles[tile] == BOMB_RANGE_PU || tiles[tile] == BOMB_COUNT_PU);
    }

    bool isBox(int tile) const{
        return tiles[tile] >= '0' && tiles[tile] <= '2';
    }

    bool isDestroyedBox(int tile) const {
        return tiles[tile] >= BOX_DESTROYED && tiles[tile] <= BOMB_COUNT_BOX_DESTROYED;
    }

    bool isPowerUp(int tile) const {
        return tiles[tile] == BOMB_COUNT_PU || tiles[tile] == BOMB_RANGE_PU;
    }

    bool isEmptyBox(int tile) const {
        return tiles[tile] == '0';
    }

    bool isPowerUpBox(int tile) const {
        return tiles[tile] == '1' || tiles[tile] == '2';
    }

    void killPlayer(int player) {
        players[player].setDead();
    }

    void addPowerUp(int tile, int type) {
//        powerUps.push_back(PowerUp(tile, type));
    }

    bool canMove(int player, int direction) const {
        Position adj = toPosition(players[player].tile).adj(direction);
        int id = toID(adj);
        return isValid(adj) && (isFree(id) || (tiles[id] == BOMB && direction == Position::NONE));
    }

    static int adjTile(int tile, int dir)  {
        return tile + (Position::dx[dir] + WIDTH * Position::dy[dir]);
    }

    void move(int player, int direction) {
        int current = players[player].tile;
        int next = adjTile(current, direction);
        players[player].tile = next;
        if(isPowerUp(next)) {
            if (tiles[next] == BOMB_COUNT_PU) {
                players[player].bombsAvailable++;
                players[player].totalBombs++;
            } else if (tiles[next] == BOMB_RANGE_PU) {
                players[player].range++;
            }
            tiles[next] = EMPTY;
            unsafe[next] = false;
            int min = -1;
            for(int i = 0; i < Bomb::TIMEOUT; i++) {
                if(explodeM[i][next]) {
                    min = i;
                    break;
                }
            }
            if(min >= 0) {
                rebomb(turn + min); // +1, as it is the following go....
            }
        }
    }

/*
    void removePowerup(int tile, int onTurn) {

        if(explodeTurn[tile] < onTurn) {
            throw std::runtime_error("Current turn shouldn't be greater than expected explode time.");
        }
        for(int d = Position::RIGHT; d <= Position::UP; d++) {
            int i = 1;
            Position p = toPosition(tile).translate(d, i);
            int t = toID(p);
            while(tiles[t] == EMPTY && explodeTurn[tile] == explodeTurn[t]) {
                i++;
                p = toPosition(tile).translate(d, i);
                t = toID(p);
            }
            if(tiles[t] == BOMB && explodeOwner[tile] == explodeOwner[t]) {
                placeBombOnly(explodeOwner[t], t, explodeTurn[t] - turn, explodeRange[t]);
            }
        }
    }*/

    int pathDist(int from, int to) const {
        int dist = -1;
        unordered_set<int> seen;
        queue<int> queue;
        queue.push(from);
        while(!queue.empty()) {
            int size = queue.size();
            dist++;
            for(int i = 0; i < size; i++) {
                int t = queue.front();
                queue.pop();
                if(seen.count(t)) continue;
                seen.insert(t);
                if(t == to) return dist;
                for(auto n : freeNeighbours(t)) {
                    if(!seen.count(n))  queue.push(n);
                }
            }
        }
        return -1;
    }

    // Includes current position.
    vector<int> reachableFrom(int tile, int steps) {
        unordered_set<int> seen;
        vector<int> ans;
        queue<int> queue;
        queue.push(tile);
        while(!queue.empty() && steps > 0) {
            steps--;
            int size = queue.size();
            for(int i = 0; i < size; i++) {
                int t = queue.front();
                queue.pop();
                if(seen.count(t)) continue;
                seen.insert(t);
                ans.push_back(t);
                for(auto n : freeNeighbours(t)) {
                    queue.push(n);
                }
            }
        }
        return ans;
    }


    int nextBombTurn() {
        return turn + Bomb::TIMEOUT;
    }

    void placeBomb(int player) {
        players[player].bombsAvailable--;
        placeBombOnly(player, players[player].tile, Bomb::TIMEOUT, players[player].range);
    }

    void minMaxCount(int tile, int* min, int* max, int* count) const {
        int seen = false;
        *(count) = 0;
        *(max) = 0;
        *(min) = 0;
        for(int i = 0; i < Bomb::TIMEOUT; i++) {
            if(explodeM[i][tile]) {
                (*count)++;
                *max = i;
                if(!seen) *min = i;
                seen = true;
            }
        }
    }

    int earliestExp(int tile) const {
        int min = -1;
        for(int i = 0; i < Bomb::TIMEOUT; i++) {
            if(explodeM[i][tile]) {
                min = i;
                break;
            }
        }
        return min+1;
    }



    void placeBombOnly(int player, int placedAt, int timeout, int blastLength) {
        int turnsBefore; // used later
        int turnsBeforeInc;
        int min;
        int max;
        int count;
        minMaxCount(placedAt, &min, &max, &count);
//        int expAt = count == 0 ? turn + timeout : std::min(turn + timeout, min);
        int relExpAt = count == 0 ? timeout - 1 : std::min(timeout- 1, min);
        // Add to bomb list (unordered)
//        bombs.push_back(Bomb(placedAt, blastLength, player, relExpAt + turn + 1));
        bombs[bombCount++] = Bomb(placedAt, blastLength, player, relExpAt + turn + 1);
        explodeM[relExpAt][placedAt] = true;
        tiles[placedAt] = BOMB;
//        explodeOwner[placedAt] = player; // Useful?
//        explodeRange[t] = blastLength;
        for(int dir = Position::RIGHT; dir <= Position::UP; dir++) {
            for (int i = 1; i <= blastLength; i++) {
                Position p = toPosition(placedAt).translate(dir, i);
                int t = Board::toID(p);
                if (!isValid(p) || tiles[t] == WALL) {
                    break;
                }
                minMaxCount(t, &min, &max, &count);
                if (max <= relExpAt) { // Equal okay?
                    // All explosions here are earlier, or there are none.
                    // All easy in this case.
                    explodeM[relExpAt][t] = true;
                    // Empty tiles have no processing.
                    if (tiles[t] == EMPTY) {
                        continue;
                    }
                    // Count destroyed boxes. Doesn't handle counting boxes twice for 2 people.
                    turnsBeforeInc = count;
                    turnsBefore = count == 0 ? 0 : relExpAt == max ? count -1 : count;
                    if(isBox(t) && turnsBeforeInc == 0) {
                        scoresM[relExpAt][player]++;
                    }
                    // Moving to this square causes unpredictable blasts.
                    if(isPowerUp(t) && turnsBefore == 0) {
                        unsafe[t] = true;
                    }
                    // Test if blast should continue.
                    if(turnsBefore >= 2 && isPowerUpBox(t)) {
                        continue;
                    }
                     // Walls and empty have already been delt with, so the next 3 can be a simple else.
                     //if(turnsBefore >= 1 && (isEmptyBox(t) || isPowerUp(t) || tiles[t] == BOMB)) break;
                    else if(turnsBefore >= 1) {
                        continue;
                    } else {
                        // turnsBefore == 0 && tile = (emptyBox || powerup || bomb).
                        break;
                    }
                } else {
                    // Tile which has a later explosion.
                    if(tiles[t] == EMPTY || (min != 0 && min < relExpAt) && (isEmptyBox(t) || isPowerUp(t) || tiles[t] == BOMB)) {
                        // Empty tiles, or empty boxes, powerup or bombs which have already been destroyed, thus
                        // handled before.
                        // Already done, above.
                        explodeM[relExpAt][t] = true;
//                        explodeM[relExpAt][t] = true;
                    } else  {
                        // We have hit a bomb, box or item which has a later explosion. Later explosions will no
                        // be based on incorrect information, so much be replaced.
                        // Sort bomb list and replace all bombs from this time forward.
                        // Clear stale data
                        rebomb(relExpAt + turn);
                    }
                }
            }
        }
    }

    void sortBombs() {
        if(bombCount == 0) return;
        if(bombCount < 20) {
            insertionSort();
        } else {
            // Quicksort
            shuffleBombs();
            sort(bombs, 0, bombCount - 1);
        }
    }

    void insertionSort() {
        Bomb temp;
        for(int i = 1; i < bombCount; i++) {
            for(int j = i; j > 0 && bombs[j].explodeTurn < bombs[j-1].explodeTurn; j--) {
                temp = bombs[j];
                bombs[j] =bombs[j-1];
                bombs[j-1] = temp;
            }
        }
    }

    void sort(Bomb bombs[], int lo, int hi) {
        if(hi <= lo) return;
        int j = partition(bombs, lo, hi);
        sort(bombs, lo, j-1);
        sort(bombs, j+1, hi);
    }

    int partition(Bomb bombs[], int lo, int hi) {
        int i = lo;
        int j = hi+1;
        Bomb temp;
        while(i <= j) {
            while(bombs[++i].explodeTurn <= bombs[lo].explodeTurn) if(i == hi) break;
            while(bombs[--j].explodeTurn >= bombs[lo].explodeTurn) if(j == lo) break;
            if(i >= j) break;
            temp = bombs[i];
            bombs[i] = bombs[j];
            bombs[j] = temp;
        }
        temp = bombs[j];
        bombs[j] = bombs[lo];
        bombs[lo] = temp;
        return j;
    }

    void shuffleBombs() {
        int idx;
        Bomb temp;
        for(int i = 0; i < bombCount; i++) {
            idx = i + rand() % (bombCount - i);
            temp = bombs[idx];
            bombs[idx] = bombs[i];
            bombs[i] = temp;
        }
    }

        void rebomb(int fromTurn) {
            // extra -1? hmmm
            int relTurn = fromTurn - turn;
            int turnsToDelete = Bomb::TIMEOUT - relTurn;
            std::memset(explodeM + relTurn, 0, sizeof(bool) * turnsToDelete * TILE_COUNT);
            std::memset(scoresM + relTurn, 0, sizeof(int) * turnsToDelete * MAX_PLAYERS);
            // Sort bombs
            sortBombs();
            int i = bombCount - 1;
            for (; i >= 0 && bombs[i].explodeTurn >= relTurn; i--);
            int oldBombCount = bombCount;
            bombCount = i + 1;
            for (int j = i + 1; j < oldBombCount; j++) {
                placeBombOnly(bombs[j].owner, bombs[j].tile, bombs[j].explodeTurn - turn, bombs[j].blastLength);
            }
        }

//    void rebombOld(int fromTurn) {
//         extra -1? hmmm
//        int relTurn = fromTurn - turn;
//        int turnsToDelete = Bomb::TIMEOUT - relTurn;
//        std::memset(explodeM + relTurn, 0, sizeof(bool) * turnsToDelete * TILE_COUNT);
//        std::memset(scoresM + relTurn, 0, sizeof(int) * turnsToDelete * MAX_PLAYERS);
        // Sort bombs
//        bombs.sort(CompareCountown());
//        int size = bombs.size();
//        vector<Bomb> toReapply;
//        auto bItr = bombs.begin();
//        while(bItr != bombs.end()) {
//            if(bItr == bombs.end()) {
//                std::cerr << "From turn: " << fromTurn << "  turn: " << turn << "  bombs size: "  << bombs.size() << std::endl;
//                throw std::runtime_error("This list should grow, and have no end.");
//            }
//            if(bItr->explodeTurn >= relTurn) {
//                placeBombOnly(bItr->owner, bItr->tile, bItr->explodeTurn - turn, bItr->blastLength);
//                bItr = bombs.erase(bItr);
//                toReapply.push_back((*bItr));
//                bItr = bombs.erase(bItr);
//            } else {
//                ++bItr;
//            }
//        }
//        for(auto& b : toReapply) {
//            placeBombOnly(b.owner, b.tile, b.explodeTurn - turn, b.blastLength);
//        }
//    }
/*
    void placeBombOnlyOld(int player, int placedAt, int timeout, int blastLength) {
        tiles[placedAt] = BOMB;
        int bombExp = turn + timeout;
        int expAt = explodeTurn[placedAt] == 0 ? bombExp : std::min(bombExp, explodeTurn[placedAt]);
        for(int dir = Position::RIGHT; dir <= Position::UP; dir++) {
            int start = dir == Position::RIGHT ? 0 : 1;
            for(int i = start; i <= blastLength; i++) {
                Position p = toPosition(placedAt).translate(dir, i);
                int t = Board::toID(p);
                if(!isValid(p) || tiles[t] == WALL) break;
                if(tiles[t] == BOMB && explodeTurn[t] > expAt) {
                    // Need to replace this bomb. It is now linked in a new chain.
                    // Bomb locations only have one explode owner (for convenience).
                    // First update the tile, to prevent infinite loop in this method.
                    explodeTurn[t] = expAt;
                    placeBombOnly(explodeOwner[t], t, expAt - turn, explodeRange[t]);
                }
                // Bug here. There could be overlapping explosions at different times.
                // Need to have lists for every square.
                explodeTurn[t] = explodeTurn[t] == 0 ? expAt : std::min(explodeTurn[t], expAt);
                explodeOwner[t] = player;
                explodeRange[t] = blastLength;
//                explodeQueue.push(Explosion(t, expAt, player));
                // Don't stop at a box if it has already been scheduled for destruction.
                if(isPowerUp(t)) {
                    // If not exploded by this time, but later exploded
                    if(!alreadyExploded(t, expAt) && unsafe[t]) {
                        // Replace later bombs.
                        // Too hacky, so wait for our 8 deep bool
//                        removePowerup(t, expAt);
                    }
                    unsafe[t] = true;
                }
                if((isPowerUp(t) || isBox(t)) && !alreadyExploded(t, expAt)) break;
            }
        }
    }
    */
/*
    bool alreadyExploded(int tile, int turn) {
        return explodeTurn[tile] != 0 && explodeTurn[tile] < turn;
    }
    */

//    void stepForwardOld(int steps) {
//        for(int s = 0; s < steps; s++) {
//            turn++;
//            auto bombItr = bombList.begin();
//            while (bombItr != bombList.end() && bombs[*bombItr].explodeTurn == turn - Bomb::TIMEOUT) {
//                explode(*bombItr);
//                bombItr = bombList.erase(bombItr);
//            }
//            for (auto i : destroyed) {
//                if (tiles[i] == BOMB_COUNT_BOX) {
//                    tiles[i] = BOMB_COUNT_PU;
//                addPowerUp(i, PowerUp::COUNT);
//                } else if (tiles[i] == BOMB_RANGE_BOX) {
//                    tiles[i] = BOMB_RANGE_PU;
//                addPowerUp(i, PowerUp::RANGE);
//                }
//                tiles[i] = EMPTY;
//            }
//        }
//    }

    void stepForward(int steps) {
//        bombs.sort(CompareCountown());
        sortBombs();
        for(int t = 0; t < steps; t++) {
            // Engine bug fix.
            for(int i = 0; i < TILE_COUNT; i++) {
                if(tiles[i] == BOX_DESTROYED) {
                    tiles[i] = EMPTY;
                } else if(tiles[i] == BOMB_RANGE_BOX_DESTROYED) {
                    tiles[i] = BOMB_RANGE_PU;
                } else if(tiles[i] == BOMB_COUNT_BOX_DESTROYED) {
                    tiles[i] = BOMB_COUNT_PU;
                }
            }
            turn++;
//            auto bombItr = bombs.begin();
//            while(bombItr != bombs.end() && bombItr->explodeTurn <= turn) {
//                if(bombItr->explodeTurn < turn) throw std::runtime_error("All bombs should have been delt with in previous turn.");
//                players[bombItr->owner].bombsAvailable++;
//                bombItr = bombs.erase(bombItr);
//            }
            int shift = 0;
            for(; shift < bombCount; shift++) {
                if(bombs[shift].explodeTurn > turn) break;
                if(bombs[shift].explodeTurn < turn) throw std::runtime_error("All bombs should have been delt with in previous turn.");
                players[bombs[shift].owner].bombsAvailable++;
            }
            for(int i = shift; i < bombCount; i++) {
                bombs[i-shift] = bombs[i];
            }
            bombCount -= shift;
            for(int p = 0; p < aliveCount; p++) {
                players[p].boxesDestroyed += scoresM[0][p];
            }
            for(int i = 0; i < TILE_COUNT; i++) {
                if(explodeM[0][i]) {
                    explode(i);
                }
            }
            memmove(explodeM, explodeM + 1, (Bomb::TIMEOUT - 1) * sizeof(bool) * TILE_COUNT);
            memmove(scoresM, scoresM + 1, (Bomb::TIMEOUT - 1) * sizeof(int) * MAX_PLAYERS);
            memset(explodeM +  (Bomb::TIMEOUT - 1), 0, sizeof(bool) * TILE_COUNT);
            memset(scoresM + (Bomb::TIMEOUT - 1),  0, sizeof(int) * MAX_PLAYERS);
        }
    }

/*
    void stepForwardOld(int steps) {
        for(int t = 0; t < steps; t++) {
            turn++;
            for(int i = 0; i < TILE_COUNT; i++) {
                if(explodeTurn[i] == turn) {
                    explode(i);
                }
                if(explodeTurn[i] != 0 && explodeTurn[i] < turn) {
                    std::ostringstream ss;
                    ss << "Tile: " << i << " should have been processed. et: "
                       << explodeTurn[i] << " at turn: " << turn << std::endl;
                    throw std::runtime_error(ss.str());
                }
            }
            for (auto i : destroyed) {
                if (tiles[i] == BOMB_COUNT_BOX) {
                    tiles[i] = BOMB_COUNT_PU;
                } else if (tiles[i] == BOMB_RANGE_BOX) {
                    tiles[i] = BOMB_RANGE_PU;
                }
                tiles[i] = EMPTY;
            }
            destroyed.clear();
        }
    }
    */

    // Assuming timers for turn already decremented.
    int survivalTurns(int player, int max = Bomb::TIMEOUT) const {
        if(!players[player].isAlive()) {
            return 0;
        }
        int startTile = players[player].tile;
//        unordered_set<int> seen(30);
        memset(hash, 0, sizeof(hash));
        return survivalTurns(startTile, max, 1);//, seen);
    }

    int survivalTurns(int t, int targetDepth, int currentDepth) const {//, unordered_set<int>& seen) const {
        if(currentDepth == targetDepth) return currentDepth;
        // Orthogonal (prime) numbers
        static const int P1 = 2;
        static const int P2 = -3;
        int maxTurns = currentDepth;
        // Note part 1: Need to try Position::None first due to the asymmetry of being allow to stay on bomb squares
        // if you started there. This is tied to the seen hashMap.
        int h;
        int n;
        int y;
        int x;
        for(int i = Position::NONE; i >= Position::RIGHT; i--) {
            const Position p = toPosition(t);
            y = p.y + Position::dy[i];
            x = p.x + Position::dx[i];
            if(x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) continue;
            n = x + y*WIDTH;
            if(tiles[n] == WALL) continue;
//            int hash = P1 * n + P2 * currentDepth;
            // Note part 2: If you try move off bomb and back, you will save the position, and can't later investigate
            // staying on bomb. Therefore, need to try staying on bomb first.

            // New hashing
//            if(seen.count(hash)) continue;
//            seen.insert(hash);
            h = n * (currentDepth - 1);
            if(hash[h]) continue;
            hash[h] = true;


            if(willBeFree(n, currentDepth, i)) {
                maxTurns = std::max(maxTurns, survivalTurns(n, targetDepth, currentDepth+1));//, seen));
                if(maxTurns == targetDepth) return targetDepth;
            }
        }
        return maxTurns;
    }

    bool willBeFree(int n, int turnsInFuture, int moveDir) const {
        // If free and not exploding next turn.
        // If box and exploded this turn, or previous turn.
//        bool occupied = false;
//        for(int i = 0; i < playerCount; i++) {
//            if(i == US) continue;
//            if(n == players[i].tile) {
//                occupied = true;
//                break;
//            }
//        }
        const bool free = isFree(n) && !explodeM[turnsInFuture-1][n] && !unsafe[n] //&& !occupied
                || tiles[n] == BOMB && moveDir == Position::NONE && !explodeM[turnsInFuture-1][n];
        if(free) return true;

        // GameEngine bug.
        const bool isB = isBox(n);
        int prevExpCount = 0;
        const int until = isB ? turnsInFuture - 2 : turnsInFuture - 1;
        for(int j = 0; j < until; j++) { // -1? or not...
            if(explodeM[j][n]) prevExpCount++;
        }
        // Will be free
        return (isDestroyedBox(n) && !explodeM[turnsInFuture-1][n]) || (isB && prevExpCount > 0 && !explodeM[turnsInFuture-1][n]) && !unsafe[n];
    }

    void explode(int tile) {
       if(isBox(tile)) {
//           players[explodeOwner[tile]].boxesDestroyed++;
           if (tiles[tile] == BOMB_COUNT_BOX) {
//               tiles[tile] = BOMB_COUNT_PU;
               tiles[tile] = BOMB_COUNT_BOX_DESTROYED;
           } else if (tiles[tile] == BOMB_RANGE_BOX) {
//               tiles[tile] = BOMB_RANGE_PU;
               tiles[tile] = BOMB_RANGE_BOX_DESTROYED;
           } else {
               tiles[tile] = BOX_DESTROYED;
           }
       } else {
           for(int p = 0; p < MAX_PLAYERS; p++) {
               if(players[p].tile == tile && players[p].isAlive()) {
                   players[p].setDead();
                   aliveCount--;
               }
           }
           // Can't keep track of bomb owner properly. This next bit is incorrect. Could
           // skip bomb tiles being marked by other owner...
//           if(tiles[tile] == BOMB) {
//               players[explodeOwner[tile]].bombsAvailable++;
//           }
           // Powerup & bombs (gets destroyed).
           tiles[tile] = EMPTY;
       }
        // Should be pop, with stacked explosions.
//        explodeTurn[tile] = NOT_SET;
        unsafe[tile] = false;
    }


    friend std::ostream& operator<<(std::ostream& out, const Board& board) {
        for(int i = 0; i < Board::HEIGHT; i++) {
            for(int j = 0; j < Board::WIDTH; j++) {
                out << board.tiles[i*Board::WIDTH + j];
            }
            out << "\n";
        }
    }

//    void explode(int bombIdx) {
//        Bomb& bomb = bombs[bombIdx];
//        bomb.disable();
//        int explodeCount = 0;
//        for(int dir = Position::RIGHT; dir <= Position::UP; dir++) {
//             Explode the bombs current position only once.
//            int start = dir == Position::RIGHT ? 0 : 1;
//            for(int i = start; i <= bomb.blastLength; i++) {
//                Position p = toPosition(bomb.tile).translate(dir, i);
//                int tile = Board::toID(p);
//                if(!isValid(p) || tiles[tile] == WALL) break;
//                explodeTurn[tile] = NOT_SET;
//                 TODO: swich to playerCount
//                for(int p = 0; p < MAX_PLAYERS; p++) {
//                    if(players[p].tile == tile) {
//                        players[p].setDead();
//                        aliveCount--;
//                    }
//                }
//                if(isBox(tile)) {
//                    destroyed.push_back(tile);
//                    explodeCount++;
//                    break;
//                } else if(bombs[tile].isLive()) {
//                    explode(tile);
//                    break;
//                }
//            }
//        }
//         Box tally.
//        players[bomb.owner].boxesDestroyed += explodeCount;
//    }
};

#endif //HYPERSONIC_BOARD_H
