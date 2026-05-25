#pragma once

#include "Tile.h"
#include "Player.h"
#include "ItemType.h"
#include <string>

struct CropInfo {
    int id;
    ItemType seed;
    ItemType product;
    const char* name;
    int growth_days;
    bool perennial;
    int min_yield;
    int max_yield;
    int xp;
};

class CropSystem {
public:
    static const CropInfo* get_crop_info(int crop_id);
    static const CropInfo* get_crop_info_by_seed(ItemType seed);
    static bool plant_crop(Player& player, Tile& tile, int crop_id, std::string& message);
    static bool harvest_crop(Player& player, Tile& tile, std::string& message);
    static void grow_crop(Tile& tile);
    static bool water_crop(Player& player, Tile& tile, std::string& message);
};
