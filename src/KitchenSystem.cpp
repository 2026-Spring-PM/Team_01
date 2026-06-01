#include "KitchenSystem.h"
#include "Input.h"

#include <iostream>
#include <algorithm>
#include <map>

// ── 상수 ────────────────────────────────────────────────────
static const int KITCHEN_UPGRADE_COST[]  = {100, 300, 500, 1000}; // Lv0->1, Lv1->2, Lv2->3, Lv3->4 (index = current level)
static const int KITCHEN_UPGRADE_LEVEL[] = {6,   10,  16,  20}; // 각 레벨 개방 플레이어 Lv (index = 목표 레벨-1)

// ── 레시피 테이블 ────────────────────────────────────────────
static const RecipeInfo RECIPES[] = {
    // 주방 Lv.1
    {1, "감자 수프",              ItemType::FOOD_POTATO_SOUP,          30,
        {ItemType::POTATO, ItemType::POTATO}},
    {1, "당근 달걀 롤",           ItemType::FOOD_CARROT_EGG_ROLL,       75,
        {ItemType::CARROT, ItemType::EGG, ItemType::CARROT}},
    {1, "고등어 당근 조림",        ItemType::FOOD_MACKEREL_CARROT,       70,
        {ItemType::FISH_MACKEREL, ItemType::CARROT, ItemType::CARROT}},
    // 주방 Lv.2
    {2, "돼지고기 감자 짜글이",    ItemType::FOOD_PORK_POTATO,          390,
        {ItemType::PORK, ItemType::POTATO, ItemType::POTATO}},
    {2, "감자 밀 부침개",          ItemType::FOOD_POTATO_WHEAT_PANCAKE, 130,
        {ItemType::WHEAT, ItemType::POTATO, ItemType::EGG, ItemType::WHEAT}},
    {2, "당근 광어 스테이크",      ItemType::FOOD_CARROT_FLATFISH,      210,
        {ItemType::FISH_FLATFISH, ItemType::CARROT, ItemType::CARROT}},
    // 주방 Lv.3
    {3, "3색 당근 쌀밥",           ItemType::FOOD_3COLOR_CARROT_RICE,   220,
        {ItemType::RICE, ItemType::CARROT, ItemType::EGG, ItemType::RICE}},
    {3, "감자 매쉬드 비프 스테이크",ItemType::FOOD_MASHED_BEEF,          540,
        {ItemType::MILK, ItemType::PORK, ItemType::POTATO, ItemType::POTATO}},
    {3, "당근 사과 주스 만찬",     ItemType::FOOD_CARROT_APPLE_JUICE,   320,
        {ItemType::APPLE, ItemType::CARROT, ItemType::MILK, ItemType::APPLE}},
    // 주방 Lv.4
    {4, "국왕의 참치 감자 가니쉬 스테이크", ItemType::FOOD_TUNA_POTATO,  1050,
        {ItemType::FISH_TUNA, ItemType::POTATO, ItemType::CARROT, ItemType::POTATO}},
    {4, "대농장 특선 하이라이스 만찬",       ItemType::FOOD_HAILIGHT_RICE, 760,
        {ItemType::RICE, ItemType::PORK, ItemType::CARROT, ItemType::MILK, ItemType::RICE, ItemType::CARROT}},
    {4, "특선 사과 감자 무스 케이크",        ItemType::FOOD_APPLE_POTATO_CAKE, 460,
        {ItemType::APPLE, ItemType::WHEAT, ItemType::POTATO, ItemType::MILK,
         ItemType::APPLE, ItemType::WHEAT, ItemType::POTATO}},
};
static constexpr int RECIPE_COUNT = 12;

// ── 희귀도 ───────────────────────────────────────────────────
static int get_item_rarity(ItemType item) {
    switch (item) {
        case ItemType::POTATO:
        case ItemType::CARROT:
        case ItemType::EGG:
        case ItemType::FISH_MACKEREL:
            return 1;
        case ItemType::WHEAT:
        case ItemType::RICE:
        case ItemType::PORK:
        case ItemType::FISH_FLATFISH:
            return 2;
        default: // APPLE, MILK, FISH_TUNA 등
            return 3;
    }
}

// ── 생성자 ───────────────────────────────────────────────────
KitchenSystem::KitchenSystem() : level_(0) {}

int KitchenSystem::recipe_count() { return RECIPE_COUNT; }

void KitchenSystem::interact(Player& player) {
    show_menu(player);
}

// ── 메인 메뉴 ────────────────────────────────────────────────
void KitchenSystem::show_menu(Player& player) {
    std::vector<ItemType> selected;
    std::string msg = "";

    while (true) {
        system("clear");
        std::cout << "🍳 ======= 주방 =======\n";
        std::cout << "현재 주방 레벨: Lv." << level_ << " / " << MAX_LEVEL << "\n";
        std::cout << "💰 보유 골드: " << player.get_money() << "G"
                  << "  ⚡ 스태미나: " << player.get_stamina() << "/" << player.get_stamina_max() << "\n";
        
        std::cout << "───────────────────────────────────────────────\n";

        // ── 업그레이드 항목 ──
        if (level_ < MAX_LEVEL) {
            int need_plv = KITCHEN_UPGRADE_LEVEL[level_]; // level_이 목표 레벨-1 인덱스
            int cost     = KITCHEN_UPGRADE_COST[level_];
            bool can     = (player.get_level() >= need_plv) && (player.get_money() >= cost);
            std::cout << "1. 주방 업그레이드  Lv." << level_ << " → Lv." << (level_+1)
                      << "  (필요: 플레이어 Lv." << need_plv << " / " << cost << "G)";
            if (can)             std::cout << "  ✅ 가능";
            else if (player.get_level() < need_plv)
                                 std::cout << "  🔒 레벨 부족";
            else                 std::cout << "  💸 골드 부족";
            std::cout << "\n";
        }

        // ── 해금된 레시피 목록 ──
        if (level_ > 0) {
            std::cout << "\n[📖 레시피]\n";
            for (int lvl = 1; lvl <= level_; ++lvl) {
                std::cout << "  ─ 주방 Lv." << lvl << "\n";
                for (int i = 0; i < RECIPE_COUNT; ++i) {
                    const auto& r = RECIPES[i];
                    if (r.kitchen_level != lvl) continue;
                    std::cout << "    " << r.name << "  [판매: " << r.sell_price << "G]  재료: ";
                    std::map<ItemType,int> cnt;
                    for (auto ing : r.ingredients) cnt[ing]++;
                    bool first = true;
                    for (auto& p : cnt) {
                        if (!first) std::cout << " + ";
                        std::cout << item_name(p.first) << "×" << p.second;
                        first = false;
                    }
                    std::cout << "\n";
                }
            }
        } else {
            std::cout << "\n  (주방을 업그레이드하면 요리를 배울 수 있습니다.)\n";
        }

        // ── 재료 선택 안내 ──
        std::cout << "\n[🧺 재료 선택 — 키를 눌러 슬롯에 추가, 최대 6칸]\n";
        struct KeyItem { char key; ItemType type; const char* label; };
        static const KeyItem KEY_MAP[] = {
            {'a', ItemType::POTATO,        "감자"},
            {'b', ItemType::CARROT,        "당근"},
            {'c', ItemType::WHEAT,         "밀"},       // 'c'는 초기화
            {'d', ItemType::RICE,          "쌀"},
            {'e', ItemType::EGG,           "달걀"},
            {'f', ItemType::MILK,          "우유"},
            {'g', ItemType::PORK,          "돼지고기"},
            {'h', ItemType::APPLE,         "사과"},
            {'i', ItemType::FISH_MACKEREL, "고등어"},
            {'j', ItemType::FISH_FLATFISH, "광어"},
            {'k', ItemType::FISH_TUNA,     "참치"},
        };
        for (const auto& ki : KEY_MAP) {
            int have = player.count_item(ki.type);
            if (have > 0)
                std::cout << "  [" << ki.key << "] " << ki.label << "(" << have << "개)";
        }
        std::cout << "\n";

        // ── 현재 선택된 재료 슬롯 ──
        std::cout << "\n선택한 재료 슬롯 (" << selected.size() << "/6): ";
        if (selected.empty()) {
            std::cout << "(없음)";
        } else {
            std::map<ItemType,int> sel_cnt;
            for (auto i : selected) sel_cnt[i]++;
            bool first = true;
            for (auto& p : sel_cnt) {
                if (!first) std::cout << " + ";
                std::cout << item_name(p.first) << "×" << p.second;
                first = false;
            }

            std::string preview = "";
            bool is_unlocked = false;

            // 전체 레시피를 돌며 현재 선택한 재료와 일치하는 레시피를 탐색
            std::vector<ItemType> sorted_sel = selected;
            std::sort(sorted_sel.begin(), sorted_sel.end());

            for (int i = 0; i < RECIPE_COUNT; ++i) {
                std::vector<ItemType> r_ings = RECIPES[i].ingredients;
                std::sort(r_ings.begin(), r_ings.end());
                
                if (sorted_sel == r_ings) {
                    // 일치하는 레시피를 찾았을 때, 내 주방 레벨 이하(해금됨)인 경우에만 이름을 오픈!
                    if (RECIPES[i].kitchen_level <= level_) {
                        preview = RECIPES[i].name;
                        is_unlocked = true;
                    }
                    break;
                }
            }

            // UI 출력 분기 처리
            if (is_unlocked && !preview.empty()) {
                // 해금된 정상 레시피일 때만 진짜 이름 출력
                std::cout << "  →  \"" << preview << "\"";
            } else if (selected.size() >= 2) {
                // 레시피가 없거나, 존재하더라도 아직 주방 레벨이 낮아 미해금 상태라면 무조건 "???" 처리
                std::cout << "  →  \"???\"";
            }
        }

        std::cout << "\n";
        std::cout << "───────────────────────────────────────────────\n";
        std::cout << "[x] 요리 시작  [z] 슬롯 초기화  [0] 나가기\n>> " << std::flush;

        char input = getch();

        // ── 나가기 ──
        if (input == '0') return;

        // ── 업그레이드 ──
        if (input == '1') {
            if (level_ >= MAX_LEVEL) {
                std::cout << "\n이미 최고 레벨 주방입니다.\n";
                wait_key();
                continue;
            }
            int need_plv = KITCHEN_UPGRADE_LEVEL[level_];
            int cost     = KITCHEN_UPGRADE_COST[level_];
            system("clear");
            std::cout << "🍳 [주방 업그레이드]\n\n";
            std::cout << "  현재 레벨  : Lv." << level_ << "\n";
            std::cout << "  업그레이드 : Lv." << (level_+1) << "\n";
            std::cout << "  필요 플레이어 레벨 : Lv." << need_plv
                      << "  (현재: Lv." << player.get_level() << ")\n";
            std::cout << "  필요 골드  : " << cost << "G"
                      << "  (보유: " << player.get_money() << "G)\n\n";
            std::cout << "업그레이드 하시겠습니까? [y/n]: " << std::flush;
            char ans = getch();
            if (ans == 'y' || ans == 'Y') {
                std::string msg;
                upgrade(player, msg);
                std::cout << "\n" << msg << "\n";
            } else {
                std::cout << "\n취소했습니다.\n";
            }
            wait_key();
            continue;
        }

        // ── 슬롯 초기화 ──
        if (input == 'z' || input == 'Z') { selected.clear(); continue; }

        // ── Enter → 요리 시도 ──
        if (input == 'x' || input == 'X') {
            if (selected.size() < 2) {
                std::cout << "\n재료를 최소 2개 이상 선택해야 합니다.\n";
                wait_key();
                continue;
            }

            std::string preview_name = "";
            std::vector<ItemType> sorted_sel = selected;
            std::sort(sorted_sel.begin(), sorted_sel.end());
            
            for (int i = 0; i < RECIPE_COUNT; ++i) {
                std::vector<ItemType> r_ings = RECIPES[i].ingredients;
                std::sort(r_ings.begin(), r_ings.end());
                // 조합도 맞고, 내 주방 레벨 이하로 '해금'까지 완벽히 된 경우에만 이름 오픈
                if (sorted_sel == r_ings && RECIPES[i].kitchen_level <= level_) {
                    preview_name = RECIPES[i].name;
                    break;
                }
            }
            
            // 미해금 레시피이거나 틀린 조합(괴식)이면 무조건 "???"로 표시 처리 (요청 사양 반영)
            std::string label = preview_name.empty() ? "???" : preview_name;
            
            // 2. [UI 개선] 화면을 더럽히지 않고 맨 밑에 질문을 띄우기 위해 힌트 출력 후 바로 대기
            std::cout << "요리할까요? [" << label << "] [y/n]: " << std::flush;
            
            // 버퍼 꼬임 방지를 위해 getch()로 즉시 한 글자만 입력받음
            char ans = getch();
            if (ans == 'y' || ans == 'Y') {
                cook(player, selected, msg);
                selected.clear();

                if (!msg.empty()) {
                    std::cout << "\n📢 " << msg << "\n";
                    msg = ""; // 출력했으니 비워주기
                    wait_key(); // 유저가 결과를 다 읽고 키를 누르면 그때 주방 UI가 갱신됨
                }
            } else {
                std::cout << "\n요리를 취소했습니다.\n";
                wait_key();
            }
            continue;
        }

        // ── 재료 추가 ──
        if (selected.size() >= 6) {
            std::cout << "\n슬롯이 가득 찼습니다. [c]로 초기화하거나 [Enter]로 요리하세요.\n";
            wait_key();
            continue;
        }
        auto item = map_key_to_item(input);
        if (item.has_value()) {
            // 현재 슬롯에 담긴 수 + 1이 보유량 초과하면 거부
            int already = 0;
            for (auto s : selected) if (s == item.value()) ++already;
            if (player.count_item(item.value()) <= already) {
                std::cout << "\n" << item_name(item.value()) << "이(가) 부족합니다.\n";
                wait_key();
            } else {
                selected.push_back(item.value());
            }
        }
    }
}

// ── 업그레이드 ───────────────────────────────────────────────
bool KitchenSystem::upgrade(Player& player, std::string& message) {
    if (level_ >= MAX_LEVEL) {
        message = "이미 최고 레벨 주방입니다.";
        return false;
    }
    int need_plv = KITCHEN_UPGRADE_LEVEL[level_];
    int cost     = KITCHEN_UPGRADE_COST[level_];
    if (player.get_level() < need_plv) {
        message = "플레이어 레벨이 부족합니다. (필요 Lv." + std::to_string(need_plv) + ")";
        return false;
    }
    if (!player.spend_money(cost)) {
        message = "골드가 부족합니다. (필요: " + std::to_string(cost) + "G)";
        return false;
    }
    ++level_;
    message = "✅ 주방을 Lv." + std::to_string(level_) + "로 업그레이드했습니다!";
    return true;
}

// ── 요리 ────────────────────────────────────────────────────
bool KitchenSystem::cook(Player& player, std::vector<ItemType> inputs, std::string& message) {
    if (inputs.size() < 2) {
        message = "재료를 최소 2개 이상 선택해야 합니다.";
        return false;
    }

    // 보유량 검사
    std::map<ItemType,int> required;
    for (auto item : inputs) required[item]++;
    for (const auto& p : required) {
        if (player.count_item(p.first) < p.second) {
            message = "가방에 " + item_name(p.first) + "이(가) 부족합니다.";
            return false;
        }
    }

    // 스태미나 검사
    if (!player.use_stamina(2)) {
        message = "스태미나가 부족합니다. (필요: 2)";
        return false;
    }

    // 정상 레시피 매칭
    std::vector<ItemType> sorted_inputs = inputs;
    std::sort(sorted_inputs.begin(), sorted_inputs.end());

    for (int i = 0; i < RECIPE_COUNT; ++i) {
        std::vector<ItemType> r_ings = RECIPES[i].ingredients;
        std::sort(r_ings.begin(), r_ings.end());
        
        // 1. 우선 조합법이 맞는지 확인
        if (sorted_inputs == r_ings) {
            // 2. 조합은 맞는데 주방 레벨이 부족한 경우 (★ 이 부분 수정)
            if (level_ < RECIPES[i].kitchen_level) {
                player.restore_stamina(2); // 스태미나 환불
                message = "❌ 해금되지 않은 레시피입니다. (요구 주방 Lv." + std::to_string(RECIPES[i].kitchen_level) + ")";
                return false; // ◀ 여기서 false를 리턴해 가방에서 재료가 사라지지 않고 요리가 중단됩니다!
            }
            
            // 3. 주방 레벨까지 완벽해서 요리에 성공한 경우
            for (auto ing : inputs) player.remove_item(ing, 1);
            player.add_item(RECIPES[i].result, 1);
            player.add_exp(10);
            
            // ★ 성공 시 무엇이 만들어졌는지 메시지에 명확하게 저장
            message = "🎉 요리 성공! [" + RECIPES[i].name + "]을(를) 만들었습니다! (판매가: "
                      + std::to_string(RECIPES[i].sell_price) + "G / XP +10)";
            return true;
        }
    }

    // ── 괴식 ──
    int rarity = 0;
    for (auto i : inputs) rarity += get_item_rarity(i);

    ItemType monster_type;
    std::string monster_name;
    int monster_sell;
    if (rarity <= 5) {
        monster_type = ItemType::MONSTER_FOOD_LOW;
        monster_name = "하급 괴식 (수상한 모방품 요리)";
        monster_sell = 500;
    } else if (rarity <= 10) {
        monster_type = ItemType::MONSTER_FOOD_MID;
        monster_name = "중급 괴식 (그럴싸한 위조 만찬)";
        monster_sell = 1500;
    } else {
        monster_type = ItemType::MONSTER_FOOD_HIGH;
        monster_name = "고급 괴식 (국왕도 속아넘어갈 환상의 미식)";
        monster_sell = 3000;
    }

    for (auto ing : inputs) player.remove_item(ing, 1);
    player.add_item(monster_type, 1);
    player.add_exp(10);
    message = "레시피가 없군요... 기묘한 " + monster_name + "이(가) 완성되었습니다!\n"
              "  ※ 상점에 가서 판매를 시도할 수 있습니다. (기본 판매가: "
              + std::to_string(monster_sell) + "G, 성공률 20% / XP +10)";
    return true;
}

// ── 레시피 이름 조회 ──────────────────────────────────────────
std::string KitchenSystem::get_recipe_name(std::vector<ItemType> inputs) {
    std::vector<ItemType> sorted = inputs;
    std::sort(sorted.begin(), sorted.end());
    for (int i = 0; i < RECIPE_COUNT; ++i) {
        std::vector<ItemType> r = RECIPES[i].ingredients;
        std::sort(r.begin(), r.end());
        if (sorted == r) return RECIPES[i].name;
    }
    return "";
}
