#pragma once

#include "Player.h"
#include "ItemType.h"

#include <string>
#include <vector>

struct RecipeInfo {
    int kitchen_level;
    std::string name;
    ItemType result;
    int sell_price;
    std::vector<ItemType> ingredients;
};

class KitchenSystem {
public:
    KitchenSystem();

    void interact(Player& player);
    void show_menu(Player& player);
    bool cook(Player& player, std::vector<ItemType> inputs, std::string& message);
    bool upgrade(Player& player, std::string& message);
    std::string get_recipe_name(std::vector<ItemType> inputs);
    static int recipe_count();
    int get_level() const { return level_; }
    bool perform_action(Player& player, int sel, std::string& message);

private:
    int level_;
    static constexpr int MAX_LEVEL = 4;
};
