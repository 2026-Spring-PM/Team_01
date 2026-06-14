#pragma once
#include <iostream>

struct Position {
    int x, y;

    Position(int x = 0, int y = 0) : x(x), y(y) {}

    Position operator+(const Position& other) const {
        return Position(x + other.x, y + other.y);
    }
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }

    friend std::ostream& operator<<(std::ostream& os, const Position& pos) {
        os << "(" << pos.x << ", " << pos.y << ")";
        return os;
    }
};
