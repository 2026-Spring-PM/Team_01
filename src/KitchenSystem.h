#pragma once

#include "Player.h"
#include <string>

class KitchenSystem {
public:
    KitchenSystem();

    void interact(Player& player);
    int get_level() const { return level_; }

private:
    int level_;
    static constexpr int MAX_LEVEL = 3;

    void show_menu(Player& player);
    bool upgrade(Player& player, std::string& message);
    bool cook_bread(Player& player, std::string& message);
    bool cook_grilled_fish(Player& player, std::string& message);
    bool cook_apple_pie(Player& player, std::string& message);
};
