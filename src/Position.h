#ifndef HYPERSONIC_POSITION_H
#define HYPERSONIC_POSITION_H

#include <iostream>


struct Position {
    static const int INVALID_IDX = -1;
    enum Dir {RIGHT, DOWN, LEFT, UP, NONE};
    static const int DIR_COUNT = 5;
    static int dx[];
    static int dy[];
//    static Position INVALID;
    int y;
    int x;

    Position() {}

    Position(int y, int x) : y(y), x(x) {}

    Position(const Position& toCopy) : y(toCopy.y), x(toCopy.x) {}

    Position translate(int dir, int steps) const {
        return Position(y + dy[dir] * steps, x + dx[dir] * steps);
    }

    Position adj(int dir) const {
        return Position(y + dy[dir], x + dx[dir]);
    }

    bool operator==(const Position& other) const {
        return y == other.y && x == other.x;
    }

    friend std::ostream &operator<<(std::ostream &os, const Position &pos) {
        os << "(" << pos.y << "," << pos.x << ")";
    }
};

//Position Position::INVALID = Position(INVALID_IDX,);

//namespace std {
//    template<> class hash<Position> {
//    public:
//        size_t operator()(const Position& p) const {
//            return
//        }
//    };
//}
#endif //HYPERSONIC_POSITION_H
