#pragma once

#include "Tile.h"
#include "Player.h"
#include "ItemType.h"
#include <string>

struct LivestockInfo {
    int type;
    ItemType baby_item;
    ItemType product_item;
    const char* name;
    int adult_age;
    int product_amount;
    int stamina_cost;
    int xp;
    bool remove_after_product;
};

class LivestockSystem {
public:
    static const LivestockInfo* get_info(int animal_type);
    static bool place_animal(Player& player, Tile& tile, int animal_type, std::string& message);
    static bool collect_product(Player& player, Tile& tile, std::string& message);
    static bool remove_animal(Tile& tile, std::string& message);
    static void grow_animal(Tile& tile);
    static bool hatch_egg(Player& player, std::string& message);
};
