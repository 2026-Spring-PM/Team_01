#include "WorkshopSystem.h"
#include "Input.h"
#include "ItemType.h"
#include <iostream>

WorkshopSystem::WorkshopSystem() : level_(0) {}

void WorkshopSystem::interact(Player& player) {
    show_menu(player);
}

// 기존 perform_action은 메뉴 로직 안으로 통합되어 더 이상 쓰이지 않지만, 
// Game.cpp 호환성을 위해 껍데기만 남겨둡니다.
bool WorkshopSystem::perform_action(Player& player, int action, std::string& message) {
    return false; 
}

void WorkshopSystem::show_menu(Player& player) {
    while (true) {
        system("clear");
        std::cout << "🔨 [대장간] 작업장 레벨: Lv." << level_ << " / " << MAX_LEVEL << "\n";
        std::cout << " [현재 장비] 괭이: Lv." << player.get_hoe_grade() 
                  << " | 낚싯대: Lv." << player.get_fishing_rod_grade() 
                  << " | 곡괭이: Lv." << player.get_pickaxe_grade() << "\n";
        std::cout << " 골드: " << player.get_money() << "G | 나무: " << player.count_item(ItemType::WOOD) << "\n";
        std::cout << " 광석: 구리(" << player.count_item(ItemType::COPPER) << "), 철(" 
                  << player.count_item(ItemType::IRON) << "), 금(" << player.count_item(ItemType::GOLD) << ")\n\n";

        std::cout << "1. 작업장 증축 (Lv. " << level_ << " -> " << level_ + 1 << ")\n";
        std::cout << "2. 동제 농기구 제작 (제작소 Lv.1, 캐릭터 Lv.6 필요)\n";
        std::cout << "3. 철제 농기구 제작 (제작소 Lv.2, 캐릭터 Lv.10 필요)\n";
        std::cout << "4. 금제 농기구 제작 (제작소 Lv.3, 캐릭터 Lv.20 필요)\n";
        std::cout << "0. 나가기\n> ";

        int choice = read_int_safe(0);
        if (choice == 0) break;

        std::string msg;
        if (choice == 1) {
            upgrade_workshop(player, msg);
        } else if (choice >= 2 && choice <= 4) {
            int target_grade = choice - 1;
            std::cout << "\n어떤 도구를 제작하시겠습니까? (1.괭이  2.낚싯대  3.곡괭이  0.취소): ";
            int tool_choice = read_int_safe(0);
            if (tool_choice >= 1 && tool_choice <= 3) {
                upgrade_tool(player, target_grade, tool_choice, msg);
            } else {
                msg = "제작을 취소했습니다.";
            }
        } else {
            msg = "잘못된 선택입니다.";
        }
        std::cout << "\n" << msg << "\n[아무 키나 눌러 계속]\n";
        wait_key();
    }
}

bool WorkshopSystem::upgrade_workshop(Player& player, std::string& message) {
    if (level_ >= MAX_LEVEL) { message = "이미 최고 레벨입니다."; return false; }
    
    int gold_req = (level_ == 0) ? 100 : (level_ == 1) ? 400 : 1000;
    int wood_req = (level_ == 0) ? 10 : (level_ == 1) ? 25 : 60;

    if (player.get_money() < gold_req || player.count_item(ItemType::WOOD) < wood_req) {
        message = "재료가 부족합니다! (요구: " + std::to_string(gold_req) + "G, 나무 " + std::to_string(wood_req) + "개)";
        return false;
    }
    player.spend_money(gold_req);
    player.remove_item(ItemType::WOOD, wood_req);
    ++level_;
    message = "🎉 작업장을 Lv." + std::to_string(level_) + "(으)로 증축했습니다!";
    return true;
}

// 기존 upgrade_tool의 인자를 수정하여(tool_type 추가) 기획서 맞춤형으로 구현!
bool WorkshopSystem::upgrade_tool(Player& player, int target_grade, int tool_type, std::string& message) {
    int req_p_lvl = (target_grade == 1) ? 6 : (target_grade == 2) ? 10 : 20;
    if (player.get_level() < req_p_lvl) { message = "캐릭터 레벨이 부족합니다! (필요 Lv." + std::to_string(req_p_lvl) + ")"; return false; }
    if (level_ < target_grade) { message = "작업장 레벨이 부족합니다! (필요 Lv." + std::to_string(target_grade) + ")"; return false; }

    int current_grade = (tool_type == 1) ? player.get_hoe_grade() : (tool_type == 2) ? player.get_fishing_rod_grade() : player.get_pickaxe_grade();
    if (current_grade >= target_grade) { message = "이미 해당 등급이거나 더 좋은 도구를 가지고 있습니다."; return false; }

    int gold = (target_grade == 1) ? 200 : (target_grade == 2) ? 600 : 2000;
    ItemType ore = (target_grade == 1) ? ItemType::COPPER : (target_grade == 2) ? ItemType::IRON : ItemType::GOLD;
    int wood = 0, ore_amt = 0;

    // 기획서 재료비 테이블 적용
    if (target_grade == 1) { // 동제
        if (tool_type == 1) { wood = 8; ore_amt = 4; } else if (tool_type == 2) { wood = 12; ore_amt = 2; } else { wood = 10; ore_amt = 3; }
    } else if (target_grade == 2) { // 철제
        if (tool_type == 1) { wood = 15; ore_amt = 5; } else if (tool_type == 2) { wood = 25; ore_amt = 3; } else { wood = 30; ore_amt = 4; }
    } else { // 금제
        if (tool_type == 1) { wood = 30; ore_amt = 5; } else if (tool_type == 2) { wood = 50; ore_amt = 2; } else { wood = 40; ore_amt = 3; }
    }

    if (player.get_money() < gold || player.count_item(ItemType::WOOD) < wood || player.count_item(ore) < ore_amt) {
        message = "제작 재료 또는 골드가 부족합니다."; return false;
    }

    player.spend_money(gold); player.remove_item(ItemType::WOOD, wood); player.remove_item(ore, ore_amt);

    if (tool_type == 1) player.set_hoe_grade(target_grade);
    else if (tool_type == 2) player.set_fishing_rod_grade(target_grade);
    else player.set_pickaxe_grade(target_grade);

    message = "✨ 성공적으로 장비를 제작 및 장착했습니다!";
    return true;
}