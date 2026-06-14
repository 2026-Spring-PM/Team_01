#pragma once

#include <ostream>

// Small value object used for tile coordinates.  The overloaded operators make
// movement and coordinate comparison explicit in the game logic.
struct Position {
    int x = 0;
    int y = 0;

    constexpr Position() = default;
    constexpr Position(int x_value, int y_value) : x(x_value), y(y_value) {}

    constexpr Position operator+(const Position& other) const {
        return Position{x + other.x, y + other.y};
    }

    Position& operator+=(const Position& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }

    constexpr bool operator!=(const Position& other) const {
        return !(*this == other);
    }

    friend std::ostream& operator<<(std::ostream& os, const Position& position) {
        os << "(" << position.x << ", " << position.y << ")";
        return os;
    }
};
