#include "WorkshopSystem.h"

#include "Input.h"
#include "ItemType.h"

#include <iostream>

namespace {
const ToolRecipe TOOL_RECIPES[] = {
    {1, 1, 1, "구리 괭이", 4, 0, 0, 8, 100, "수확 스태미나 5 -> 4"},
    {2, 1, 2, "철 괭이", 0, 5, 0, 15, 300, "수확 스태미나 4 -> 3"},
    {3, 1, 3, "금 괭이", 0, 0, 5, 30, 1000, "수확 스태미나 3 -> 2"},
    {1, 2, 1, "구리 낚싯대", 2, 0, 0, 12, 100, "낚시 Perfect 보상 해금"},
    {2, 2, 2, "철 낚싯대", 0, 3, 0, 25, 300, "낚시 Great 보상 강화"},
    {3, 2, 3, "금 낚싯대", 0, 0, 2, 50, 1000, "낚시 스태미나 5 -> 3"},
    {1, 3, 1, "구리 곡괭이", 3, 0, 0, 10, 100, "누적 압력 대사 힌트"},
    {2, 3, 2, "철 곡괭이", 0, 4, 0, 30, 300, "누적 압력 범위 표시"},
    {3, 3, 3, "금 곡괭이", 0, 0, 3, 40, 1000, "광산 스태미나 5 -> 3, 압력 상승 1~10"},
};

constexpr int TOOL_COUNT = static_cast<int>(sizeof(TOOL_RECIPES) / sizeof(TOOL_RECIPES[0]));

int current_grade_for(const Player& player, int tool_type) {
    if (tool_type == 1) return player.get_hoe_grade();
    if (tool_type == 2) return player.get_rod_grade();
    return player.get_pick_grade();
}

void set_grade_for(Player& player, int tool_type, int grade) {
    if (tool_type == 1) player.set_hoe_grade(grade);
    else if (tool_type == 2) player.set_rod_grade(grade);
    else player.set_pick_grade(grade);
}
}

WorkshopSystem::WorkshopSystem() : level_(0) {}

void WorkshopSystem::interact(Player& player) {
    show_menu(player);
}

bool WorkshopSystem::perform_action(Player& player, int action, std::string& message) {
    if (action == 1) return upgrade_workshop(player, message);
    if (action >= 2 && action <= TOOL_COUNT + 1) return craft_tool(player, action - 2, message);
    message = "올바르지 않은 작업장 행동입니다.";
    return false;
}

void WorkshopSystem::show_menu(Player& player) {
    while (true) {
        std::cout << "\n[작업장] Lv." << level_ << " / " << MAX_LEVEL
                  << " | 괭이 " << player.get_hoe_grade()
                  << " | 낚싯대 " << player.get_rod_grade()
                  << " | 곡괭이 " << player.get_pick_grade() << "\n";
        std::cout << "1. 작업장 증축\n";
        for (int i = 0; i < TOOL_COUNT; ++i) {
            const auto& r = TOOL_RECIPES[i];
            std::cout << (i + 2) << ". " << r.name << " - " << r.effect << "\n";
        }
        std::cout << "0. 뒤로\n>> " << std::flush;

        int action = read_int_safe(-1);
        if (action == 0) return;
        std::string msg;
        bool acted = perform_action(player, action, msg);
        std::cout << (acted ? "\n성공: " : "\n안내: ") << msg << "\n";
    }
}

bool WorkshopSystem::upgrade_workshop(Player& player, std::string& message) {
    if (level_ >= MAX_LEVEL) {
        message = "작업장이 이미 최고 레벨입니다.";
        return false;
    }

    int need_level = 0;
    int base_gold = 0;
    int need_stamina = 0;
    if (level_ == 0) { need_level = 6; base_gold = 200; need_stamina = 8; }
    else if (level_ == 1) { need_level = 10; base_gold = 600; need_stamina = 15; }
    else { need_level = 15; base_gold = 1200; need_stamina = 20; }

    int need_gold = player.discounted_upgrade_cost(base_gold);
    if (player.get_level() < need_level) {
        message = "플레이어 Lv." + std::to_string(need_level) + "이 필요합니다.";
        return false;
    }
    if (!player.spend_money(need_gold)) {
        message = "골드가 부족합니다. 필요: " + std::to_string(need_gold) + "G";
        return false;
    }
    if (!player.use_stamina(need_stamina)) {
        player.add_money(need_gold);
        message = "스태미나가 부족합니다. 필요: " + std::to_string(need_stamina);
        return false;
    }

    ++level_;
    player.add_exp(80 * level_);
    message = "작업장을 Lv." + std::to_string(level_) + "로 증축했습니다. 비용 "
            + std::to_string(need_gold) + "G.";
    if (need_gold < base_gold) message += " INT 할인 적용.";
    return true;
}

bool WorkshopSystem::craft_tool(Player& player, int recipe_index, std::string& message) {
    if (recipe_index < 0 || recipe_index >= TOOL_COUNT) {
        message = "알 수 없는 제작식입니다.";
        return false;
    }

    const ToolRecipe& r = TOOL_RECIPES[recipe_index];
    int money_req = player.discounted_upgrade_cost(r.money_req);
    if (level_ < r.workshop_level) {
        message = "작업장 Lv." + std::to_string(r.workshop_level) + "이 필요합니다.";
        return false;
    }
    if (current_grade_for(player, r.tool_type) >= r.target_grade) {
        message = "이미 같거나 더 좋은 도구를 보유하고 있습니다.";
        return false;
    }
    if (player.get_money() < money_req ||
        player.count_item(ItemType::COPPER) < r.copper_req ||
        player.count_item(ItemType::IRON) < r.iron_req ||
        player.count_item(ItemType::GOLD) < r.gold_req ||
        player.count_item(ItemType::WOOD) < r.wood_req) {
        message = "재료가 부족합니다. 필요: " + std::to_string(money_req) + "G, 구리 "
                + std::to_string(r.copper_req) + ", 철 " + std::to_string(r.iron_req)
                + ", 금 " + std::to_string(r.gold_req) + ", 나무 " + std::to_string(r.wood_req);
        return false;
    }

    player.spend_money(money_req);
    player.remove_item(ItemType::COPPER, r.copper_req);
    player.remove_item(ItemType::IRON, r.iron_req);
    player.remove_item(ItemType::GOLD, r.gold_req);
    player.remove_item(ItemType::WOOD, r.wood_req);
    set_grade_for(player, r.tool_type, r.target_grade);
    player.add_exp(60 * r.target_grade);
    message = r.name + " 제작 완료. " + r.effect;
    if (money_req < r.money_req) message += " INT 할인 적용.";
    return true;
}
