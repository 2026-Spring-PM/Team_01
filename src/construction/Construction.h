#pragma once
#include <string>

// ============================================================
//  플레이어 구조체 (다른 파일에서 정의하는 경우 해당 부분 제거)
// ============================================================
struct Player {
    int level;
    int stamina;
    int maxStamina;
    int money;
    int exp;

    // 인벤토리 - 원재료
    int inv_potato;    // 감자
    int inv_carrot;    // 당근
    int inv_wheat;     // 밀
    int inv_rice;      // 쌀
    int inv_apple;     // 사과
    int inv_egg;       // 달걀
    int inv_milk;      // 우유
    int inv_pork;      // 돼지고기 (도축 후)
    int inv_mackerel;  // 고등어
    int inv_flounder;  // 광어
    int inv_tuna;      // 참치

    // 인벤토리 - 요리 결과물 (ID 순서와 일치)
    // 0=감자수프, 1=당근달걀롤, 2=고등어당근조림,
    // 3=돼지고기감자짜글이, 4=감자밀부침개, 5=당근광어스테이크,
    // 6=3색당근쌀밥, 7=감자매쉬드비프스테이크, 8=당근사과주스만찬,
    // 9=국왕의참치감자가니쉬스테이크, 10=대농장특선하이라이스만찬, 11=특선사과감자무스케이크
    int inv_cooked[12];

    // 광물 & 자재
    int inv_copper;
    int inv_iron;
    int inv_gold;
    int inv_wood;

    // 현재 장착 농기구 (0=기본, 1=동제, 2=철제, 3=금제)
    int toolLevel;
};

// ============================================================
//  농기구 스태미나 경감표 (기획서 6.2)
// ============================================================
struct ToolStats {
    std::string name;
    int staminaPlant;
    int staminaHarvest;
    int craftStamina;
    int craftExp;
    int buildCost;
    int unlockLevel;
    int needCopper;
    int needIron;
    int needGold;
    int needWood;
};

const ToolStats TOOL_TABLE[4] = {
    {"기본 손 도구",    10, 10,   0,    0,    0,  1, 0, 0, 0, 0},
    {"동제 농기구 🥉",   7,  7,  30,  100,  200,  6, 3, 0, 0, 4},
    {"철제 농기구 🥈",   5,  5,  30,  250,  600, 10, 0, 4, 0, 5},
    {"금제 농기구 🥇",   2,  2,  30,  800, 2000, 20, 0, 0, 2, 6},
};

// ============================================================
//  주방 레시피표 (이미지 기준, 스태미나는 x10 스케일)
// ============================================================
struct Recipe {
    std::string name;
    int kitchenLevel;  // 필요 주방 레벨 (1~4)
    int sellPrice;     // 판매가 (원가는 참고용)
    int costPrice;     // 원가
    int staminaCost;   // 소모 스태미나 (이미지 수치 x10)
    int hpRestore;     // 섭취 시 스태미나 회복 (0 = 판매 전용)
    // 재료 (없으면 0)
    int needPotato;
    int needCarrot;
    int needWheat;
    int needRice;
    int needApple;
    int needEgg;
    int needMilk;
    int needPork;
    int needMackerel;
    int needFlounder;
    int needTuna;
    // 결과 아이템 ID (inv_cooked 인덱스)
    int resultItem;
};

// 총 12종 레시피
const int RECIPE_COUNT = 12;
const Recipe RECIPE_TABLE[RECIPE_COUNT] = {
// name                         lv  sell  cost  sta  hp   po  ca  wh  ri  ap  eg  ml  pk  mc  fl  tu  id
{"감자 수프",                    1,   30,  10,  20,  50,   2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
{"당근 달걀 롤",                  1,   75,  39,  20,  80,   0,  2,  0,  0,  0,  1,  0,  0,  0,  0,  0,  1},
{"고등어 당근 조림",               1,   70,  32,  20,  70,   0,  1,  0,  0,  0,  0,  0,  0,  1,  0,  0,  2},
{"돼지고기 감자 짜글이",            2,  390, 270,  20, 120,   2,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  3},
{"감자 밀 부침개",                 2,  130,  60,  20, 100,   1,  0,  2,  0,  0,  1,  0,  0,  0,  0,  0,  4},
{"당근 광어 스테이크",              2,  210, 104,  20, 150,   0,  2,  0,  0,  0,  0,  0,  0,  0,  1,  0,  5},
{"3색 당근 쌀밥",                  3,  220,  89,  30,   0,   0,  2,  0,  2,  0,  1,  0,  0,  0,  0,  0,  6},
{"감자 매쉬드 비프 스테이크",        3,  540, 340,  30, 200,   2,  0,  0,  0,  0,  0,  1,  1,  0,  0,  0,  7},
{"당근 사과 주스 만찬",              3,  320, 154,  30, 180,   0,  2,  0,  0,  2,  0,  1,  0,  0,  0,  0,  8},
{"국왕의 참치 감자 가니쉬 스테이크",  4, 1050, 622,  30,   0,   3,  1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  9},
{"대농장 특선 하이라이스 만찬",       4,  760, 404,  30, 300,   0,  2,  0,  2,  0,  0,  1,  1,  0,  0,  0, 10},
{"특선 사과 감자 무스케이크",         4,  460, 180,  30, 200,   2,  0,  2,  0,  2,  0,  1,  0,  0,  0,  0, 11},
};

// ============================================================
//  건물 상태
// ============================================================
struct BuildingState {
    bool houseBuilt;
    int  kitchenLevel;   // 0=없음, 1~4
    int  workshopLevel;  // 0=없음, 1~3
};

// ============================================================
//  함수 선언
// ============================================================
void enterHouse(Player& player, BuildingState& buildings, int currentDay, int& outNextDay);
void kitchenMenu(Player& player, const BuildingState& buildings);
void workshopMenu(Player& player, BuildingState& buildings);
void buildMenu(Player& player, BuildingState& buildings);
void printPlayerStatus(const Player& player);
bool spendStamina(Player& player, int amount);
void gainExp(Player& player, int amount);
