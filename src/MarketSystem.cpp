#include "MarketSystem.h"
#include "Input.h"
#include "Season.h"
#include <iostream>

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

        {ItemType::COPPER,        "구리 광석", 0, 15, 1},
        {ItemType::IRON,          "철 광석", 0, 30, 1},
        {ItemType::GOLD,          "금 광석", 0, 75, 1}
    };
}

void MarketSystem::start_new_day() {
    // 향후 Krobus 랜덤 등장 등 일일 상점 처리를 연결할 자리.
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
    while (true) {
        system("clear");
        std::cout << "💰 [판매 상점] 보유 골드: " << player.get_money() << "G\n";
        std::cout << "───────────────────────────────────────────────\n";
        for (std::size_t i = 0; i < sell_items_.size(); ++i) {
            const auto& item = sell_items_[i];
            std::cout << (i + 1) << ". " << item.name << " - " << item.sell_price << "G"
                      << "   🎒 현재 보유: " << player.count_item(item.item) << "개\n";
        }
        std::cout << "0. 나가기\n>> " << std::flush;

        int choice = read_int_safe(-1);
        if (choice == 0) return;
        if (choice < 1 || choice > static_cast<int>(sell_items_.size())) continue;

        std::cout << "판매 수량: " << std::flush;
        int qty = read_int_safe(0);
        std::string message;
        sell(player, sell_items_[choice - 1], qty, message);
        std::cout << "\n" << message << "\n";
        wait_key();
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
