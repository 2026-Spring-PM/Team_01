#include "Construction.h"
#include <iostream>
#include <algorithm>

// ============================================================
//  유틸
// ============================================================
void printPlayerStatus(const Player& player) {
    std::cout << "\n[내 상태] Lv." << player.level
              << " | 스태미나: " << player.stamina << "/" << player.maxStamina
              << " | 소지금: " << player.money << "원"
              << " | 경험치: " << player.exp << "\n";
}

bool spendStamina(Player& player, int amount) {
    if (player.stamina < amount) {
        std::cout << "⚠️  스태미나가 부족합니다. (필요: " << amount
                  << " / 현재: " << player.stamina << ")\n";
        return false;
    }
    player.stamina -= amount;
    return true;
}

void gainExp(Player& player, int amount) {
    if (player.level >= 30) return;
    player.exp += amount;
    std::cout << "  (+" << amount << " XP)\n";
}

// ============================================================
//  집 입장: 취침 / 주방
// ============================================================
void enterHouse(Player& player, BuildingState& buildings, int currentDay, int& outNextDay) {
    std::cout << "\n============================\n";
    std::cout << "   🏡 집 안으로 들어왔습니다.\n";
    std::cout << "============================\n";
    printPlayerStatus(player);

    while (true) {
        std::cout << "\n무엇을 하시겠습니까?\n";
        std::cout << "  [B] 취침 (다음 날, 스태미나 전액 회복)\n";
        if (buildings.kitchenLevel > 0)
            std::cout << "  [K] 주방 이용 (Lv." << buildings.kitchenLevel << ")\n";
        else
            std::cout << "  [K] 주방 (미설치)\n";
        std::cout << "  [X] 나가기\n>> ";

        char key; std::cin >> key;
        key = (char)std::toupper((unsigned char)key);

        if (key == 'B') {
            player.stamina = player.maxStamina;
            outNextDay = currentDay + 1;
            std::cout << "\n😴 취침합니다... 스태미나 " << player.maxStamina << " 회복.\n";
            std::cout << "☀️  Day " << outNextDay << " 아침!\n";
            break;
        } else if (key == 'K') {
            if (buildings.kitchenLevel == 0)
                std::cout << "⚠️  주방이 없습니다. 건축 메뉴에서 건설하세요.\n";
            else
                kitchenMenu(player, buildings);
        } else if (key == 'X') {
            outNextDay = currentDay;
            break;
        } else {
            std::cout << "잘못된 입력입니다.\n";
        }
    }
}

// ============================================================
//  주방 메뉴
// ============================================================
static bool hasIngredients(const Player& p, const Recipe& r) {
    return p.inv_potato   >= r.needPotato
        && p.inv_carrot   >= r.needCarrot
        && p.inv_wheat    >= r.needWheat
        && p.inv_rice     >= r.needRice
        && p.inv_apple    >= r.needApple
        && p.inv_egg      >= r.needEgg
        && p.inv_milk     >= r.needMilk
        && p.inv_pork     >= r.needPork
        && p.inv_mackerel >= r.needMackerel
        && p.inv_flounder >= r.needFlounder
        && p.inv_tuna     >= r.needTuna;
}

static void consumeIngredients(Player& p, const Recipe& r) {
    p.inv_potato   -= r.needPotato;
    p.inv_carrot   -= r.needCarrot;
    p.inv_wheat    -= r.needWheat;
    p.inv_rice     -= r.needRice;
    p.inv_apple    -= r.needApple;
    p.inv_egg      -= r.needEgg;
    p.inv_milk     -= r.needMilk;
    p.inv_pork     -= r.needPork;
    p.inv_mackerel -= r.needMackerel;
    p.inv_flounder -= r.needFlounder;
    p.inv_tuna     -= r.needTuna;
}

static void printIngredientReq(const Recipe& r) {
    std::cout << "      재료: ";
    if (r.needPotato   > 0) std::cout << "감자x"   << r.needPotato   << " ";
    if (r.needCarrot   > 0) std::cout << "당근x"   << r.needCarrot   << " ";
    if (r.needWheat    > 0) std::cout << "밀x"     << r.needWheat    << " ";
    if (r.needRice     > 0) std::cout << "쌀x"     << r.needRice     << " ";
    if (r.needApple    > 0) std::cout << "사과x"   << r.needApple    << " ";
    if (r.needEgg      > 0) std::cout << "달걀x"   << r.needEgg      << " ";
    if (r.needMilk     > 0) std::cout << "우유x"   << r.needMilk     << " ";
    if (r.needPork     > 0) std::cout << "돼지고기x" << r.needPork   << " ";
    if (r.needMackerel > 0) std::cout << "고등어x" << r.needMackerel << " ";
    if (r.needFlounder > 0) std::cout << "광어x"   << r.needFlounder << " ";
    if (r.needTuna     > 0) std::cout << "참치x"   << r.needTuna     << " ";
    std::cout << "\n";
}

void kitchenMenu(Player& player, const BuildingState& buildings) {
    std::cout << "\n============================\n";
    std::cout << "   🍳 주방 (Lv." << buildings.kitchenLevel << ")\n";
    std::cout << "============================\n";

    // 재료 현황
    std::cout << "[재료 현황]\n";
    std::cout << "  감자:" << player.inv_potato
              << " 당근:" << player.inv_carrot
              << " 밀:"   << player.inv_wheat
              << " 쌀:"   << player.inv_rice
              << " 사과:" << player.inv_apple << "\n";
    std::cout << "  달걀:" << player.inv_egg
              << " 우유:" << player.inv_milk
              << " 돼지고기:" << player.inv_pork
              << " 고등어:" << player.inv_mackerel
              << " 광어:" << player.inv_flounder
              << " 참치:" << player.inv_tuna << "\n";
    std::cout << "[스태미나] " << player.stamina << "/" << player.maxStamina << "\n\n";

    // 레시피 목록
    int shownIdx[RECIPE_COUNT];
    int shownCount = 0;

    for (int i = 0; i < RECIPE_COUNT; i++) {
        const Recipe& r = RECIPE_TABLE[i];
        if (r.kitchenLevel > buildings.kitchenLevel) continue;

        bool canMake = hasIngredients(player, r) && (player.stamina >= r.staminaCost);
        shownIdx[shownCount++] = i;

        std::cout << "  [" << shownCount << "] " << r.name
                  << (canMake ? "" : " ❌") << "\n";
        std::cout << "      판매가: " << r.sellPrice << "원"
                  << " | 원가: " << r.costPrice << "원"
                  << " | 스태미나: -" << r.staminaCost;
        if (r.hpRestore > 0)
            std::cout << " | 회복: +" << r.hpRestore;
        else
            std::cout << " | (판매 전용)";
        std::cout << "\n";
        printIngredientReq(r);
    }

    if (shownCount == 0) {
        std::cout << "  현재 레벨에서 제작 가능한 레시피가 없습니다.\n";
    }

    std::cout << "  [0] 나가기\n>> ";
    int choice; std::cin >> choice;
    if (choice == 0 || choice < 1 || choice > shownCount) return;

    const Recipe& sel = RECIPE_TABLE[shownIdx[choice - 1]];

    if (!hasIngredients(player, sel)) { std::cout << "⚠️  재료 부족\n"; return; }
    if (!spendStamina(player, sel.staminaCost)) return;

    consumeIngredients(player, sel);
    player.inv_cooked[sel.resultItem]++;

    // XP: 원가 기준 비례 부여 (원가/10, int 버림) - 기획서 "레시피에 따라 +30~+150 XP"
    int xp = std::max(30, std::min(150, sel.costPrice / 10));
    gainExp(player, xp);

    std::cout << "✅ " << sel.name << " 완료! (인벤토리 보관)\n";

    // 회복 가능한 요리면 즉시 섭취 여부 질문
    if (sel.hpRestore > 0) {
        std::cout << "  섭취해서 스태미나 +" << sel.hpRestore << " 회복하시겠습니까? [Y/N] >> ";
        char c; std::cin >> c;
        if ((char)std::toupper((unsigned char)c) == 'Y') {
            player.stamina = std::min(player.stamina + sel.hpRestore, player.maxStamina);
            player.inv_cooked[sel.resultItem]--;
            std::cout << "  스태미나: " << player.stamina << "/" << player.maxStamina << "\n";
        }
    }

    printPlayerStatus(player);
}

// ============================================================
//  농기구 제작소 메뉴
// ============================================================
void workshopMenu(Player& player, BuildingState& buildings) {
    std::cout << "\n============================\n";
    std::cout << "   🔨 농기구 제작소 (Lv." << buildings.workshopLevel << ")\n";
    std::cout << "============================\n";
    std::cout << "[현재 장착] " << TOOL_TABLE[player.toolLevel].name
              << "  심기:" << TOOL_TABLE[player.toolLevel].staminaPlant
              << " / 수확:" << TOOL_TABLE[player.toolLevel].staminaHarvest << "\n";
    std::cout << "[광물/자재]  구리:" << player.inv_copper
              << "  철:" << player.inv_iron
              << "  금:" << player.inv_gold
              << "  나무:" << player.inv_wood << "\n\n";

    for (int i = 1; i <= 3; i++) {
        const ToolStats& t = TOOL_TABLE[i];
        if (i <= player.toolLevel) {
            std::cout << "  [" << i << "] " << t.name << " ✅ 완료\n";
            continue;
        }
        if (i > buildings.workshopLevel) {
            std::cout << "  [" << i << "] " << t.name
                      << " 🔒 제작소 Lv." << i << " 필요\n";
            continue;
        }
        if (player.level < t.unlockLevel) {
            std::cout << "  [" << i << "] " << t.name
                      << " 🔒 플레이어 Lv." << t.unlockLevel << " 필요\n";
            continue;
        }
        bool ok = player.inv_copper >= t.needCopper
               && player.inv_iron   >= t.needIron
               && player.inv_gold   >= t.needGold
               && player.inv_wood   >= t.needWood
               && player.stamina    >= t.craftStamina;

        std::cout << "  [" << i << "] " << t.name << (ok ? "" : " ❌") << "\n";
        std::cout << "      재료: ";
        if (t.needCopper) std::cout << "구리x" << t.needCopper << " ";
        if (t.needIron)   std::cout << "철x"   << t.needIron   << " ";
        if (t.needGold)   std::cout << "금x"   << t.needGold   << " ";
        if (t.needWood)   std::cout << "나무x" << t.needWood   << " ";
        std::cout << "| 스태미나: -" << t.craftStamina
                  << " | +" << t.craftExp << " XP\n";
        std::cout << "      효과: 심기 " << t.staminaPlant
                  << " / 수확 " << t.staminaHarvest << "\n";
    }

    std::cout << "  [0] 나가기\n>> ";
    int choice; std::cin >> choice;
    if (choice == 0 || choice < 1 || choice > 3) return;

    const ToolStats& t = TOOL_TABLE[choice];
    if (choice <= player.toolLevel)         { std::cout << "⚠️  이미 보유 중\n"; return; }
    if (choice > buildings.workshopLevel)   { std::cout << "⚠️  제작소 레벨 부족\n"; return; }
    if (player.level < t.unlockLevel)       { std::cout << "⚠️  플레이어 레벨 부족\n"; return; }
    if (player.inv_copper < t.needCopper || player.inv_iron < t.needIron
     || player.inv_gold < t.needGold   || player.inv_wood < t.needWood) {
        std::cout << "⚠️  재료 부족\n"; return;
    }
    if (!spendStamina(player, t.craftStamina)) return;

    player.inv_copper -= t.needCopper;
    player.inv_iron   -= t.needIron;
    player.inv_gold   -= t.needGold;
    player.inv_wood   -= t.needWood;
    player.toolLevel   = choice;
    gainExp(player, t.craftExp);

    std::cout << "✅ " << t.name << " 장착 완료!\n";
    std::cout << "  심기: " << t.staminaPlant << " / 수확: " << t.staminaHarvest << "\n";
    printPlayerStatus(player);
}

// ============================================================
//  건물 건설 / 업그레이드 메뉴
// ============================================================
struct BuildOption {
    std::string label;
    int cost;
    int stamCost;
    int expGain;
    int reqLevel;
};

//                               label                     cost  sta   xp   lv
const BuildOption BUILD_OPTIONS[] = {
    {"주방 Lv.1 건설",              300,   80,  500,  6},
    {"주방 Lv.2 업그레이드",         600,  100, 1000, 10},
    {"주방 Lv.3 업그레이드",        1200,  150, 2000, 16},
    {"주방 Lv.4 업그레이드",        2500,  200, 3000, 20},
    {"농기구 제작소 Lv.1 건설",      200,   80,  500,  6},
    {"농기구 제작소 Lv.2 업그레이드", 600,  150, 2000, 10},
    {"농기구 제작소 Lv.3 업그레이드",2000,  300, 4000, 20},
};
const int BUILD_OPTION_COUNT = 7;

void buildMenu(Player& player, BuildingState& buildings) {
    std::cout << "\n============================\n";
    std::cout << "   🔧 건물 건설/업그레이드\n";
    std::cout << "============================\n";
    std::cout << "  주방: Lv." << buildings.kitchenLevel
              << "  |  제작소: Lv." << buildings.workshopLevel << "\n";
    printPlayerStatus(player);
    std::cout << "\n";

    int shown[BUILD_OPTION_COUNT];
    int shownCount = 0;

    for (int i = 0; i < BUILD_OPTION_COUNT; i++) {
        const BuildOption& b = BUILD_OPTIONS[i];

        bool alreadyDone = false;
        if (i == 0 && buildings.kitchenLevel  >= 1) alreadyDone = true;
        if (i == 1 && buildings.kitchenLevel  >= 2) alreadyDone = true;
        if (i == 2 && buildings.kitchenLevel  >= 3) alreadyDone = true;
        if (i == 3 && buildings.kitchenLevel  >= 4) alreadyDone = true;
        if (i == 4 && buildings.workshopLevel >= 1) alreadyDone = true;
        if (i == 5 && buildings.workshopLevel >= 2) alreadyDone = true;
        if (i == 6 && buildings.workshopLevel >= 3) alreadyDone = true;

        // 선행 건물 체크
        bool prereq = true;
        if (i == 1 && buildings.kitchenLevel  < 1) prereq = false;
        if (i == 2 && buildings.kitchenLevel  < 2) prereq = false;
        if (i == 3 && buildings.kitchenLevel  < 3) prereq = false;
        if (i == 5 && buildings.workshopLevel < 1) prereq = false;
        if (i == 6 && buildings.workshopLevel < 2) prereq = false;

        shown[shownCount++] = i;

        std::string status;
        if (alreadyDone)              status = " ✅ 완료";
        else if (!prereq)             status = " 🔒 선행 건물 필요";
        else if (player.level < b.reqLevel)  status = " 🔒 Lv." + std::to_string(b.reqLevel) + " 필요";
        else if (player.money < b.cost)      status = " ❌ 돈 부족";
        else if (player.stamina < b.stamCost) status = " ❌ 스태미나 부족";

        std::cout << "  [" << shownCount << "] " << b.label << status << "\n";
        std::cout << "      비용: " << b.cost << "원 | 스태미나: -" << b.stamCost
                  << " | +" << b.expGain << " XP\n";
    }

    std::cout << "  [0] 나가기\n>> ";
    int choice; std::cin >> choice;
    if (choice == 0 || choice < 1 || choice > shownCount) return;

    int idx = shown[choice - 1];
    const BuildOption& sel = BUILD_OPTIONS[idx];

    if (player.level  < sel.reqLevel)  { std::cout << "⚠️  레벨 부족\n"; return; }
    if (player.money  < sel.cost)      { std::cout << "⚠️  돈 부족\n"; return; }
    if (!spendStamina(player, sel.stamCost)) return;

    player.money -= sel.cost;

    switch (idx) {
        case 0: buildings.kitchenLevel  = 1; break;
        case 1: buildings.kitchenLevel  = 2; break;
        case 2: buildings.kitchenLevel  = 3; break;
        case 3: buildings.kitchenLevel  = 4; break;
        case 4: buildings.workshopLevel = 1; break;
        case 5: buildings.workshopLevel = 2; break;
        case 6: buildings.workshopLevel = 3; break;
    }

    gainExp(player, sel.expGain);
    std::cout << "✅ " << sel.label << " 완료!\n";
    printPlayerStatus(player);
}
