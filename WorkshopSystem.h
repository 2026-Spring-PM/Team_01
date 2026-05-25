#pragma once

#include "Player.h"
#include <string>

class WorkshopSystem {
public:
    WorkshopSystem();

    void interact(Player& player);
    int get_level() const { return level_; }

private:
    int level_;
    static constexpr int MAX_LEVEL = 3;

    void show_menu(Player& player);
    bool upgrade_workshop(Player& player, std::string& message);
    bool upgrade_tool(Player& player, int target_grade, std::string& message);
};
