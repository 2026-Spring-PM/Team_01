#pragma once

#include "Player.h"

#include <string>

struct ToolRecipe {
    int workshop_level;
    int tool_type;
    int target_grade;
    std::string name;
    int copper_req;
    int iron_req;
    int gold_req;
    int wood_req;
    int money_req;
    std::string effect;
};

class WorkshopSystem {
public:
    WorkshopSystem();

    void interact(Player& player);
    int get_level() const { return level_; }
    bool perform_action(Player& player, int action, std::string& message);
    bool craft_tool(Player& player, int recipe_index, std::string& message);

private:
    int level_;
    static constexpr int MAX_LEVEL = 3;

    void show_menu(Player& player);
    bool upgrade_workshop(Player& player, std::string& message);
};
