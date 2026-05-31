#pragma once

#include "Tile.h"
#include <vector>

class Map {
public:
    static constexpr int WIDTH = 12;
    static constexpr int HEIGHT = 12;

    static constexpr int BED_X = 10;
    static constexpr int BED_Y = 1;

    static constexpr int MARKET_X = 10;
    static constexpr int MARKET_Y = 4;

    static constexpr int KITCHEN_X = 8;
    static constexpr int KITCHEN_Y = 7;

    static constexpr int WORKSHOP_X = 10;
    static constexpr int WORKSHOP_Y = 7;

    static constexpr int STORAGE_X = 8;
    static constexpr int STORAGE_Y = 10;

    static constexpr int MINE_X = 10;
    static constexpr int MINE_Y = 10;

    Map();

    const Tile& get_tile(int x, int y) const;
    Tile& get_tile(int x, int y);
    bool is_in_bounds(int x, int y) const;

    void advance_day();
    void kill_unwatered_crops_during_drought();
    void spawn_wild_trees();
private:
    std::vector<std::vector<Tile>> tiles_;
    Tile out_of_bounds_tile_;
};
