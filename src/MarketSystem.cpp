#include "MarketSystem.h"
#include "Input.h"
#include "Season.h"
#include <iostream>
#include <cstdlib>

MarketSystem::MarketSystem() {
    buy_items_ = {
        {ItemType::SEED_POTATO,  "감자 씨앗", 5, 0, 1},
        {ItemType::SEED_CARROT,  "당근 씨앗", 10, 0, 1},
        {ItemType::SEED_WHEAT,   "밀 씨앗", 15, 0, 6},
        {ItemType::SEED_RICE,    "쌀 씨앗", 25, 0, 6},
        {ItemType::SEED_APPLE,   "사과 묘목", 250, 0, 16},
        {ItemType::WOOD,         "나무(건축자재)", 15, 0, 1},
        {ItemType::BABY_CHICKEN, "새끼 닭", 40, 0, 1},
        {ItemType::BABY_PIG,     "새끼 돼지", 100, 0, 6},
        {ItemType::BABY_COW,     "새끼 소", 500, 0, 16}
    };

    sell_items_ = {
        {ItemType::POTATO,        "감자", 0, 12, 1},
        {ItemType::CARROT,        "당근", 0, 25, 1},
        {ItemType::WHEAT,         "밀", 0, 35, 1},
        {ItemType::RICE,          "쌀", 0, 60, 1},
        {ItemType::APPLE,         "사과", 0, 150, 1},
        {ItemType::EGG,           "달걀", 0, 20, 1},
        {ItemType::MILK,          "우유", 0, 80, 1},
        {ItemType::PORK,          "돼지고기", 0, 300, 1},
        {ItemType::FISH_MACKEREL, "고등어", 0, 40, 1},
        {ItemType::FISH_FLATFISH, "광어", 0, 75, 1},
        {ItemType::FISH_TUNA,     "참치", 0, 250, 1},
        // 요리 (주방 시스템 연동)
        {ItemType::FOOD_POTATO_SOUP,          "감자 수프", 0, 30, 1},
        {ItemType::FOOD_CARROT_EGG_ROLL,      "당근 달걀 롤", 0, 75, 1},
        {ItemType::FOOD_MACKEREL_CARROT,      "고등어 당근 조림", 0, 70, 1},
        {ItemType::FOOD_PORK_POTATO,          "돼지고기 감자 짜글이", 0, 390, 1},
        {ItemType::FOOD_POTATO_WHEAT_PANCAKE, "감자 밀 부침개", 0, 130, 1},
        {ItemType::FOOD_CARROT_FLATFISH,      "당근 광어 스테이크", 0, 210, 1},
        {ItemType::FOOD_3COLOR_CARROT_RICE,   "3색 당근 쌀밥", 0, 220, 1},
        {ItemType::FOOD_MASHED_BEEF,          "감자 매쉬드 비프", 0, 540, 1},
        {ItemType::FOOD_CARROT_APPLE_JUICE,   "당근 사과 주스 만찬", 0, 320, 1},
        {ItemType::FOOD_TUNA_POTATO,          "국왕의 참치 감자 스테이크", 0, 1050, 1},
        {ItemType::FOOD_HAILIGHT_RICE,        "대농장 특선 하이라이스", 0, 760, 1},
        {ItemType::FOOD_APPLE_POTATO_CAKE,    "특선 사과 감자 무스 케이크", 0, 460, 1},
        {ItemType::MONSTER_FOOD_LOW,   "하급 괴식 (수상한 모방품)", 0, 200, 1},
        {ItemType::MONSTER_FOOD_MID,   "중급 괴식 (그럴싸한 위조 만찬)", 0, 500, 1},
        {ItemType::MONSTER_FOOD_HIGH,  "고급 괴식 (환상의 미식)", 0, 2000, 1},
        // 레거시
        {ItemType::COPPER,        "구리 광석", 0, 15, 1},
        {ItemType::IRON,          "철 광석", 0, 30, 1},
        {ItemType::GOLD,          "금 광석", 0, 75, 1}
    };
}

void MarketSystem::start_new_day() {
    // 향후 Krobus 랜덤 등장 등 일일 상점 처리를 연결할 자리.
}

bool MarketSystem::buy_item(Player& player, std::size_t index, int qty, std::string& message) {
    if (index >= buy_items_.size()) {
        message = "잘못된 상품입니다.";
        return false;
    }
    return buy(player, buy_items_[index], qty, message);
}

bool MarketSystem::sell_item(Player& player, std::size_t index, int qty, std::string& message) {
    if (index >= sell_items_.size()) {
        message = "잘못된 상품입니다.";
        return false;
    }
    return sell(player, sell_items_[index], qty, message);
}

void MarketSystem::show_buy_shop(Player& player) {
    while (true) {
        system("clear");
        std::cout << "🏪 [피에르 상점 - 구매] 보유 골드: " << player.get_money() << "G\n";
        std::cout << "───────────────────────────────────────────────\n";
        for (std::size_t i = 0; i < buy_items_.size(); ++i) {
            const auto& item = buy_items_[i];
            std::cout << (i + 1) << ". " << item.name << " - " << item.buy_price << "G"
                      << "   🎒 현재 보유: " << player.count_item(item.item) << "개";
            if (player.get_level() < item.required_level) {
                std::cout << "   🔒 Lv." << item.required_level << " 해금";
            }
            std::cout << "\n";
        }
        std::cout << "0. 나가기\n>> " << std::flush;

        int choice = read_int_safe(-1);
        if (choice == 0) return;
        if (choice < 1 || choice > static_cast<int>(buy_items_.size())) continue;

        std::cout << "구매 수량: " << std::flush;
        int qty = read_int_safe(0);
        std::string message;
        buy(player, buy_items_[choice - 1], qty, message);
        std::cout << "\n" << message << "\n";
        wait_key();
    }
}

void MarketSystem::show_sell_shop(Player& player) {
    // 괴식 ItemType 목록
    static const ItemType MONSTER_FOODS[] = {
        ItemType::MONSTER_FOOD_LOW,
        ItemType::MONSTER_FOOD_MID,
        ItemType::MONSTER_FOOD_HIGH,
    };
    static const int MONSTER_SELL_PRICE[] = {500, 1500, 3000};
    static const char* MONSTER_NAMES[] = {
        "하급 괴식 (수상한 모방품 요리)",
        "중급 괴식 (그럴싸한 위조 만찬)",
        "고급 괴식 (국왕도 속아넘어갈 환상의 미식)",
    };

    while (true) {
        system("clear");
        std::cout << "💰 [피에르 상점 - 판매] 보유 골드: " << player.get_money() << "G\n";
        std::cout << "───────────────────────────────────────────────\n";

        // 보유한 일반 아이템만 표시 (괴식 제외)
        std::vector<std::size_t> display_idx;
        for (std::size_t i = 0; i < sell_items_.size(); ++i) {
            const auto& item = sell_items_[i];
            // 괴식은 아래 별도 처리
            if (item.item == ItemType::MONSTER_FOOD_LOW  ||
                item.item == ItemType::MONSTER_FOOD_MID  ||
                item.item == ItemType::MONSTER_FOOD_HIGH) continue;
            if (player.count_item(item.item) > 0)
                display_idx.push_back(i);
        }

        if (display_idx.empty()) {
            std::cout << "  (판매할 일반 아이템이 없습니다.)\n";
        } else {
            for (std::size_t n = 0; n < display_idx.size(); ++n) {
                const auto& item = sell_items_[display_idx[n]];
                std::cout << (n + 1) << ". " << item.name
                          << "  판매가: " << item.sell_price << "G"
                          << "  보유: " << player.count_item(item.item) << "개\n";
            }
        }

        // 괴식 보유 시 별도 섹션
        bool has_monster = false;
        for (int m = 0; m < 3; ++m)
            if (player.count_item(MONSTER_FOODS[m]) > 0) { has_monster = true; break; }

        int monster_menu_start = static_cast<int>(display_idx.size()) + 1;
        if (has_monster) {
            std::cout << "───────────────────────────────────────────────\n";
            std::cout << "🤢 [괴식 판매 — 성공률 20%, 실패 시 소멸]\n";
            for (int m = 0; m < 3; ++m) {
                int have = player.count_item(MONSTER_FOODS[m]);
                if (have > 0) {
                    std::cout << (monster_menu_start + m) << ". " << MONSTER_NAMES[m]
                              << "  판매가: " << MONSTER_SELL_PRICE[m] << "G"
                              << "  보유: " << have << "개\n";
                }
            }
        }

        std::cout << "0. 나가기\n>> " << std::flush;

        int choice = read_int_safe(-1);
        if (choice == 0) return;

        // ── 일반 아이템 판매 ──
        if (choice >= 1 && choice <= static_cast<int>(display_idx.size())) {
            const auto& item = sell_items_[display_idx[choice - 1]];
            std::cout << "판매 수량 (보유 " << player.count_item(item.item) << "개): " << std::flush;
            int qty = read_int_safe(0);
            std::string message;
            sell(player, item, qty, message);
            std::cout << "\n" << message << "\n";
            wait_key();
            continue;
        }

        // ── 괴식 판매 ──
        if (has_monster) {
            for (int m = 0; m < 3; ++m) {
                if (player.count_item(MONSTER_FOODS[m]) <= 0) continue;
                if (choice != monster_menu_start + m) continue;

                std::cout << "\n🤢 " << MONSTER_NAMES[m] << " 판매 시도...\n";
                std::cout << "  성공 시: +" << MONSTER_SELL_PRICE[m] << "G\n";
                std::cout << "  실패 시: 괴식 소멸\n";
                std::cout << "정말 판매하시겠습니까? [y/n]: " << std::flush;
                char ans = getch();
                if (ans != 'y' && ans != 'Y') { std::cout << "\n취소했습니다.\n"; wait_key(); break; }

                player.remove_item(MONSTER_FOODS[m], 1);
                player.set_sold_monster_food(true);
                int roll = std::rand() % 100;
                if (roll < 20) {
                    player.add_money(MONSTER_SELL_PRICE[m]);
                    std::cout << "\n✅ 판매 성공! +" << MONSTER_SELL_PRICE[m] << "G 획득!\n";
                } else {
                    std::cout << "\n❌ 판매 실패... 상점 주인이 의심스러운 눈초리로 바라봅니다. 괴식이 소멸되었습니다.\n";
                }
                wait_key();
                break;
            }
        }
    }
}

bool MarketSystem::buy(Player& player, const MarketItem& item, int qty, std::string& message) {
    if (qty <= 0) {
        message = "수량이 올바르지 않습니다.";
        return false;
    }
    if (player.get_level() < item.required_level) {
        message = item.name + "은(는) Lv." + std::to_string(item.required_level) + "부터 구매할 수 있습니다.";
        return false;
    }
    int total = item.buy_price * qty;
    if (!player.spend_money(total)) {
        message = "골드가 부족합니다. 필요 골드: " + std::to_string(total) + "G";
        return false;
    }
    if (!player.add_item(item.item, qty)) {
        player.add_money(total);
        message = "가방 공간이 부족하여 구매하지 못했습니다.";
        return false;
    }
    message = item.name + " " + std::to_string(qty) + "개를 구매했습니다. (-" + std::to_string(total) + "G)";
    return true;
}

bool MarketSystem::sell(Player& player, const MarketItem& item, int qty, std::string& message) {
    if (qty <= 0) {
        message = "수량이 올바르지 않습니다.";
        return false;
    }
    if (player.count_item(item.item) < qty) {
        message = "판매할 수량이 부족합니다.";
        return false;
    }
    float multiplier = 1.0f;
    Season season = season_from_day(player.get_day());
    if (item.item == ItemType::RICE && season == Season::SUMMER) multiplier = 1.5f;
    if (item.item == ItemType::APPLE && season == Season::FALL) multiplier = 1.5f;
    if (item.item == ItemType::WHEAT && season == Season::WINTER) multiplier = 1.3f;
    int total = static_cast<int>(item.sell_price * qty * multiplier);
    player.remove_item(item.item, qty);
    player.add_money(total);
    message = item.name + " " + std::to_string(qty) + "개를 판매했습니다. (+" + std::to_string(total) + "G)";
    return true;
}
