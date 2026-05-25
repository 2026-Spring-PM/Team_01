#pragma once

#include "Tile.h"

#include <vector>

// 12x12 격자 맵
class Map {
public:
    static constexpr int WIDTH = 12;
    static constexpr int HEIGHT = 12;

    // 주요 위치 상수
    static constexpr int BED_X = 10;
    static constexpr int BED_Y = 1;

    static constexpr int MARKET_X = 10;
    static constexpr int MARKET_Y = 5;

    static constexpr int KITCHEN_X = 9;
    static constexpr int KITCHEN_Y = 10;

    static constexpr int WORKSHOP_X = 11;
    static constexpr int WORKSHOP_Y = 10;

    Map();

    const Tile& get_tile(int x, int y) const;
    bool is_in_bounds(int x, int y) const;

private:
    std::vector<std::vector<Tile>> tiles_;
    Tile out_of_bounds_tile_;
};