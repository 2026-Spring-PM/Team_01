#ifndef WORKSHOPSYSTEM_H
#define WORKSHOPSYSTEM_H

#include "Player.h"
#include "ItemType.h"
#include <string>

struct ToolRecipe {
    int workshop_level;
    int tool_type; // 1: 괭이, 2: 낚싯대, 3: 곡괭이
    int target_grade; // 1: 동, 2: 철, 3: 금
    std::string name;
    int copper_req, iron_req, gold_req, wood_req, money_req;
    std::string effect;
};

class WorkshopSystem {
public:
    WorkshopSystem();
    int get_level() const;
    void interact(Player& player);
    bool perform_action(Player& player, int action, std::string& message);

private:
    int level_;
    void show_menu(Player& player);
    bool upgrade_workshop(Player& player, std::string& message);
    bool craft_tool(Player& player, int recipe_index, std::string& message);
};

#endif