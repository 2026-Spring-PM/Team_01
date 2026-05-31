#include "WorkshopSystem.h"
#include "Input.h"
#include "ItemType.h"

#include <iostream>

WorkshopSystem::WorkshopSystem() : level_(0) {}

void WorkshopSystem::interact(Player& player) {
    show_menu(player);
}

bool WorkshopSystem::perform_action(Player& player, int action, std::string& message) {
    if (action == 1) return upgrade_workshop(player, message);
    if (action == 2) return upgrade_tool(player, 1, message);
    if (action == 3) return upgrade_tool(player, 2, message);
    if (action == 4) return upgrade_tool(player, 3, message);
    message = "잘못된 선택입니다.";
    return false;
}

void WorkshopSystem::show_menu(Player& player) {
    while (true) {
        system("clear");
        std::cout << "🔨 [작업장] 작업장 Lv." << level_ << " / " << MAX_LEVEL
                  << " | 농기구 등급: " << player.get_tool_grade() << "\n";
        std::cout << "골드: " << player.get_money() << "G, 스태미나: " << player.get_stamina() << "\n";
        std::cout << "보유 재료: 나무 " << player.count_item(ItemType::WOOD)
                  << ", 구리 " << player.count_item(ItemType::COPPER)
                  << ", 철 " << player.count_item(ItemType::IRON)
                  << ", 금 " << player.count_item(ItemType::GOLD) << "\n\n";
        std::cout << "1. 작업장 증축\n";
        std::cout << "2. 구리 농기구 업그레이드 - 작업장 Lv.1, 100G, 나무5, 구리3, 스태미나30\n";
        std::cout << "3. 철 농기구 업그레이드   - 작업장 Lv.2, 300G, 나무10, 철4, 스태미나30\n";
        std::cout << "4. 금 농기구 업그레이드   - 작업장 Lv.3, 800G, 나무20, 금2, 스태미나30\n";
        std::cout << "0. 나가기\n>> " << std::flush;

        char sel = getch();
        std::string msg;
        bool acted = false;
        if (sel == '0') return;
        if (sel == '1') acted = upgrade_workshop(player, msg);
        else if (sel == '2') acted = upgrade_tool(player, 1, msg);
        else if (sel == '3') acted = upgrade_tool(player, 2, msg);
        else if (sel == '4') acted = upgrade_tool(player, 3, msg);
        else msg = "잘못된 선택입니다.";

        (void)acted;
        std::cout << "\n" << msg << "\n";
        wait_key();
    }
}

bool WorkshopSystem::upgrade_workshop(Player& player, std::string& message) {
    if (level_ >= MAX_LEVEL) {
        message = "이미 최고 레벨 작업장입니다.";
        return false;
    }

    int need_level = 0;
    int need_gold = 0;
    int need_stamina = 0;

    if (level_ == 0) { need_level = 6; need_gold = 200; need_stamina = 8; }
    else if (level_ == 1) { need_level = 10; need_gold = 600; need_stamina = 15; }
    else { need_level = 15; need_gold = 1200; need_stamina = 20; }

    if (player.get_level() < need_level) {
        message = "레벨이 부족합니다. (필요 Lv." + std::to_string(need_level) + ")";
        return false;
    }
    if (!player.spend_money(need_gold)) {
        message = "골드가 부족합니다. (필요: " + std::to_string(need_gold) + "G)";
        return false;
    }
    if (!player.use_stamina(need_stamina)) {
        player.add_money(need_gold);
        message = "스태미나가 부족합니다. (필요: " + std::to_string(need_stamina) + ")";
        return false;
    }

    ++level_;
    message = "작업장을 Lv." + std::to_string(level_) + "로 증축했습니다.";
    return true;
}

bool WorkshopSystem::upgrade_tool(Player& player, int target_grade, std::string& message) {
    if (target_grade <= player.get_tool_grade()) {
        message = "이미 해당 등급 이상의 농기구를 보유하고 있습니다.";
        return false;
    }
    if (level_ < target_grade) {
        message = "작업장 레벨이 부족합니다. (필요 Lv." + std::to_string(target_grade) + ")";
        return false;
    }

    int gold = 0;
    int wood = 0;
    ItemType ore = ItemType::COPPER;
    int ore_amount = 0;

    if (target_grade == 1) { gold = 100; wood = 5; ore = ItemType::COPPER; ore_amount = 3; }
    else if (target_grade == 2) { gold = 300; wood = 10; ore = ItemType::IRON; ore_amount = 4; }
    else { gold = 800; wood = 20; ore = ItemType::GOLD; ore_amount = 2; }

    if (player.get_money() < gold || player.count_item(ItemType::WOOD) < wood || player.count_item(ore) < ore_amount) {
        message = "재료 또는 골드가 부족합니다.";
        return false;
    }
    if (!player.use_stamina(3)) {
        message = "스태미나가 부족합니다. (필요: 3)";
        return false;
    }

    player.spend_money(gold);
    player.remove_item(ItemType::WOOD, wood);
    player.remove_item(ore, ore_amount);
    player.set_tool_grade(target_grade);
    player.add_exp(target_grade * 200);

    message = "농기구를 " + std::to_string(target_grade) + "등급으로 업그레이드했습니다. XP +" + std::to_string(target_grade * 200);
    return true;
}
