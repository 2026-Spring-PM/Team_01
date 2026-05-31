#pragma once

#include "Player.h"
#include <string>
#include "ItemType.h"


class KitchenSystem {
public:
    KitchenSystem();

    void interact(Player& player);
    int get_level() const { return level_; }
    void perform_action(Player& player, int action, std::string& message); 

private:
    int level_;
    static constexpr int MAX_LEVEL = 3;

    void show_menu(Player& player);
    bool upgrade(Player& player, std::string& message);
    bool cook_bread(Player& player, std::string& message);
    bool cook_grilled_fish(Player& player, std::string& message);
    bool cook_apple_pie(Player& player, std::string& message);
};
