#include "WorkshopSystem.h"
#include "Input.h"
#include <iostream>

static const int WORKSHOP_UPGRADE_COST[] = {0, 200, 600, 2000}; 
static const int WORKSHOP_UPGRADE_LEVEL[] = {6, 10, 20, 99}; // 동제=6, 철제=10, 금제=20 해금

static const ToolRecipe TOOLS[] = {
    {1, 1, 1, "동제 괭이",   4, 0, 0,  8,  200, "수확량 추가 확률"},
    {1, 2, 1, "동제 낚싯대", 2, 0, 0, 12,  200, "Perfect 시 초급 보물상자 지급"},
    {1, 3, 1, "동제 곡괭이", 3, 0, 0, 10,  200, "누적 압력을 대사로 알림"},
    {2, 1, 2, "철제 괭이",   0, 5, 0, 15,  600, "수확량 추가 확률 상승"},
    {2, 2, 2, "철제 낚싯대", 0, 3, 0, 25,  600, "Great/Perfect 보물상자 등급 향상"},
    {2, 3, 2, "철제 곡괭이", 0, 4, 0, 30,  600, "누적 압력을 크기 6 범위로 알림"},
    {3, 1, 3, "금제 괭이",   0, 0, 5, 30, 2000, "성장 1일 단축 + 수확량 추가 확률"},
    {3, 2, 3, "금제 낚싯대", 0, 0, 2, 50, 2000, "상자 등급 향상 + 낚시 스태미나 소모 감소"},
    {3, 3, 3, "금제 곡괭이", 0, 0, 3, 40, 2000, "압력 1~10 무작위 변경 + 광산 스태미나 감소"}
};
static const int TOOL_COUNT = 9;

WorkshopSystem::WorkshopSystem() : level_(0) {}

int WorkshopSystem::get_level() const { return level_; }

void WorkshopSystem::interact(Player& player) { show_menu(player); }

bool WorkshopSystem::perform_action(Player& player, int action, std::string& message) {
    if (action == 1) return upgrade_workshop(player, message);
    if (action >= 2 && action <= TOOL_COUNT + 1) return craft_tool(player, action - 2, message);
    message = "잘못된 선택입니다.";
    return false;
}

void WorkshopSystem::show_menu(Player& player) {
    while (true) {
        system("clear");
        std::cout << "🔨 [작업장] 현재 제작소 레벨: " << level_ << " / 3";
        if (level_ == 0) std::cout << " (건설 필요)";
        std::cout << "\n골드: " << player.get_money() << "G | 보유 재료: 나무" << player.count_item(ItemType::WOOD)
                  << " 구리" << player.count_item(ItemType::COPPER)
                  << " 철" << player.count_item(ItemType::IRON)
                  << " 금" << player.count_item(ItemType::GOLD) << "\n\n";

        if (level_ < 3) {
            std::cout << "1. 제작소 업그레이드 (Lv." << level_ << " -> Lv." << (level_ + 1)
                      << ") - " << WORKSHOP_UPGRADE_COST[level_] << "G (요구 레벨: Lv." << WORKSHOP_UPGRADE_LEVEL[level_] << ")\n\n";
        } else {
            std::cout << "1. 제작소 업그레이드 (최고 레벨 도달)\n\n";
        }

        int opt_num = 2;
        for (int lvl = 1; lvl <= 3; ++lvl) {
            std::cout << "--- 제작소 Lv." << lvl << " --- ";
            if (level_ < lvl) std::cout << "🔒 (Lv." << WORKSHOP_UPGRADE_LEVEL[lvl-1] << " 해금)\n";
            else std::cout << "🔓\n";

            for (int i = 0; i < TOOL_COUNT; ++i) {
                if (TOOLS[i].workshop_level == lvl) {
                    std::cout << "  " << opt_num << ". " << TOOLS[i].name << " - ";
                    if (TOOLS[i].copper_req > 0) std::cout << "구리 " << TOOLS[i].copper_req << " / ";
                    if (TOOLS[i].iron_req > 0) std::cout << "철 " << TOOLS[i].iron_req << " / ";
                    if (TOOLS[i].gold_req > 0) std::cout << "금 " << TOOLS[i].gold_req << " / ";
                    std::cout << "나무 " << TOOLS[i].wood_req << " / " << TOOLS[i].money_req << "G\n";
                    std::cout << "      효과: " << TOOLS[i].effect;
                    
                    bool equipped = false;
                    if (TOOLS[i].tool_type == 1 && player.get_hoe_grade() >= TOOLS[i].target_grade) equipped = true;
                    if (TOOLS[i].tool_type == 2 && player.get_rod_grade() >= TOOLS[i].target_grade) equipped = true;
                    if (TOOLS[i].tool_type == 3 && player.get_pick_grade() >= TOOLS[i].target_grade) equipped = true;
                    
                    if (equipped) std::cout << " [✓ 이미 장착 중]";
                    else if (level_ < lvl) std::cout << " 🔒";
                    
                    std::cout << "\n";
                    opt_num++;
                }
            }
            std::cout << "\n";
        }

        std::cout << "0. 나가기\n>> " << std::flush;
        int sel = read_int_safe(-1);
        if (sel == 0) return;
        std::string msg;
        if (sel == 1) upgrade_workshop(player, msg);
        else if (sel >= 2 && sel <= TOOL_COUNT + 1) craft_tool(player, sel - 2, msg);
        else msg = "잘못된 선택입니다.";
        
        std::cout << "\n" << msg << "\n";
        wait_key();
    }
}

bool WorkshopSystem::upgrade_workshop(Player& player, std::string& message) {
    if (level_ >= 3) { message = "이미 최고 레벨 제작소입니다."; return false; }
    int need_level = WORKSHOP_UPGRADE_LEVEL[level_];
    int cost = WORKSHOP_UPGRADE_COST[level_];
    if (player.get_level() < need_level) {
        message = "플레이어 레벨이 부족합니다. (필요 Lv." + std::to_string(need_level) + ")";
        return false;
    }
    if (!player.spend_money(cost)) {
        message = "골드가 부족합니다.";
        return false;
    }
    ++level_;
    message = "제작소를 Lv." + std::to_string(level_) + "(으)로 업그레이드했습니다.";
    return true;
}

bool WorkshopSystem::craft_tool(Player& player, int index, std::string& message) {
    if (index < 0 || index >= TOOL_COUNT) { message = "잘못된 레시피입니다."; return false; }
    const ToolRecipe& r = TOOLS[index];
    if (level_ < r.workshop_level) {
        message = "제작소 Lv." + std::to_string(r.workshop_level) + " 이상이 필요합니다.";
        return false;
    }
    int current_grade = 0;
    if (r.tool_type == 1) current_grade = player.get_hoe_grade();
    else if (r.tool_type == 2) current_grade = player.get_rod_grade();
    else if (r.tool_type == 3) current_grade = player.get_pick_grade();

    if (current_grade >= r.target_grade) {
        message = "이미 같거나 더 높은 등급의 농기구를 장착 중입니다.";
        return false;
    }
    if (player.count_item(ItemType::COPPER) < r.copper_req || player.count_item(ItemType::IRON) < r.iron_req ||
        player.count_item(ItemType::GOLD) < r.gold_req || player.count_item(ItemType::WOOD) < r.wood_req || player.get_money() < r.money_req) {
        message = "재료 또는 골드가 부족합니다.";
        return false;
    }

    player.remove_item(ItemType::COPPER, r.copper_req);
    player.remove_item(ItemType::IRON, r.iron_req);
    player.remove_item(ItemType::GOLD, r.gold_req);
    player.remove_item(ItemType::WOOD, r.wood_req);
    player.spend_money(r.money_req);

    if (r.tool_type == 1) player.set_hoe_grade(r.target_grade);
    else if (r.tool_type == 2) player.set_rod_grade(r.target_grade);
    else if (r.tool_type == 3) player.set_pick_grade(r.target_grade);

    message = r.name + " 장착 완료! " + r.effect;
    return true;
}