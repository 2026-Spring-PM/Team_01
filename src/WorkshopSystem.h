#pragma once

#include "Player.h"
#include <string>

class WorkshopSystem {
public:
    WorkshopSystem();

    void interact(Player& player);
    int get_level() const { return level_; }
    bool perform_action(Player& player, int action, std::string& message); // 1 증축, 2 구리, 3 철, 4 금

private:
    int level_;
    static constexpr int MAX_LEVEL = 3;

    void show_menu(Player& player);
    bool upgrade_workshop(Player& player, std::string& message);
    bool upgrade_tool(Player& player, int target_grade, std::string& message);
};
