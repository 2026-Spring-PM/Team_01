#include "MarketSystem.h"

#include "Input.h"
#include "Season.h"

#include <cstdlib>
#include <iostream>
#include <sstream>

namespace {
bool is_monster_food(ItemType item) {
    return item == ItemType::MONSTER_FOOD_LOW ||
           item == ItemType::MONSTER_FOOD_MID ||
           item == ItemType::MONSTER_FOOD_HIGH;
}
}

MarketSystem::MarketSystem() {
    buy_items_ = {
        {ItemType::SEED_POTATO, "감자 씨앗", 5, 0, 1},
        {ItemType::SEED_CARROT, "당근 씨앗", 10, 0, 1},
        {ItemType::SEED_WHEAT, "밀 씨앗", 15, 0, 6},
        {ItemType::SEED_RICE, "쌀 씨앗", 25, 0, 6},
        {ItemType::SEED_APPLE, "사과 묘목", 250, 0, 16},
        {ItemType::WOOD, "나무", 15, 0, 1},
        {ItemType::BABY_CHICKEN, "병아리", 40, 0, 1},
        {ItemType::BABY_PIG, "새끼 돼지", 100, 0, 6},
        {ItemType::BABY_COW, "송아지", 500, 0, 16},
    };

    sell_items_ = {
        {ItemType::POTATO, "감자", 0, 12, 1},
        {ItemType::CARROT, "당근", 0, 25, 1},
        {ItemType::WHEAT, "밀", 0, 35, 1},
        {ItemType::RICE, "쌀", 0, 60, 1},
        {ItemType::APPLE, "사과", 0, 150, 1},
        {ItemType::EGG, "달걀", 0, 20, 1},
        {ItemType::MILK, "우유", 0, 80, 1},
        {ItemType::PORK, "돼지고기", 0, 300, 1},
        {ItemType::FISH_MACKEREL, "고등어", 0, 40, 1},
        {ItemType::FISH_FLATFISH, "광어", 0, 75, 1},
        {ItemType::FISH_TUNA, "참치", 0, 250, 1},
        {ItemType::FOOD_POTATO_SOUP, "감자 수프", 0, 30, 1},
        {ItemType::FOOD_CARROT_EGG_ROLL, "당근 달걀 롤", 0, 75, 1},
        {ItemType::FOOD_MACKEREL_CARROT, "고등어 당근 조림", 0, 70, 1},
        {ItemType::FOOD_PORK_POTATO, "돼지고기 감자 짜글이", 0, 390, 1},
        {ItemType::FOOD_POTATO_WHEAT_PANCAKE, "감자 밀 부침개", 0, 130, 1},
        {ItemType::FOOD_CARROT_FLATFISH, "당근 광어 스테이크", 0, 210, 1},
        {ItemType::FOOD_3COLOR_CARROT_RICE, "3색 당근 쌀밥", 0, 220, 1},
        {ItemType::FOOD_MASHED_BEEF, "감자 매쉬드 비프", 0, 540, 1},
        {ItemType::FOOD_CARROT_APPLE_JUICE, "당근 사과 주스 만찬", 0, 320, 1},
        {ItemType::FOOD_TUNA_POTATO, "참치 감자 스테이크", 0, 1050, 1},
        {ItemType::FOOD_HAILIGHT_RICE, "특선 하이라이스", 0, 760, 1},
        {ItemType::FOOD_APPLE_POTATO_CAKE, "사과 감자 무스 케이크", 0, 460, 1},
        {ItemType::MONSTER_FOOD_LOW, "하급 몬스터 푸드", 0, 200, 1},
        {ItemType::MONSTER_FOOD_MID, "중급 몬스터 푸드", 0, 500, 1},
        {ItemType::MONSTER_FOOD_HIGH, "고급 몬스터 푸드", 0, 2000, 1},
        {ItemType::COPPER, "구리 광석", 0, 15, 1},
        {ItemType::IRON, "철 광석", 0, 30, 1},
        {ItemType::GOLD, "금 광석", 0, 75, 1},
    };
}

void MarketSystem::start_new_day() {}

bool MarketSystem::buy_item(Player& player, std::size_t index, int qty, std::string& message) {
    if (index >= buy_items_.size()) {
        message = "올바르지 않은 아이템입니다.";
        return false;
    }
    return buy(player, buy_items_[index], qty, message);
}

bool MarketSystem::sell_item(Player& player, std::size_t index, int qty, std::string& message) {
    if (index >= sell_items_.size()) {
        message = "올바르지 않은 아이템입니다.";
        return false;
    }
    return sell(player, sell_items_[index], qty, message);
}

void MarketSystem::show_buy_shop(Player& player) {
    while (true) {
        system("clear");
        std::cout << "[상점 - 구매] 골드: " << player.get_money()
                  << "G | 신뢰도: " << player.get_shop_trust() << "/100\n";
        for (std::size_t i = 0; i < buy_items_.size(); ++i) {
            const auto& item = buy_items_[i];
            int price = player.discounted_market_buy_price(item.buy_price);
            std::cout << (i + 1) << ". " << item.name << " - " << price << "G";
            if (player.get_level() < item.required_level) std::cout << " (Lv." << item.required_level << ")";
            if (player.is_shop_restricted() && item.required_level > 1) std::cout << " (신뢰도 제한)";
            std::cout << "\n";
        }
        std::cout << "0. 뒤로\n>> " << std::flush;

        int choice = read_int_safe(-1);
        if (choice == 0) return;
        if (choice < 1 || choice > static_cast<int>(buy_items_.size())) continue;
        std::cout << "수량: " << std::flush;
        int qty = read_int_safe(0);
        std::string msg;
        buy(player, buy_items_[choice - 1], qty, msg);
        std::cout << "\n" << msg << "\n";
        wait_key();
    }
}

void MarketSystem::show_sell_shop(Player& player) {
    while (true) {
        system("clear");
        std::cout << "[상점 - 판매] 골드: " << player.get_money()
                  << "G | 신뢰도: " << player.get_shop_trust()
                  << "/100 | 몬스터 푸드 성공률: " << player.monster_food_success_chance() << "%\n";
        for (std::size_t i = 0; i < sell_items_.size(); ++i) {
            const auto& item = sell_items_[i];
            int have = player.count_item(item.item);
            if (have <= 0) continue;
            int price = is_monster_food(item.item) ? player.monster_food_sell_price(item.sell_price) : item.sell_price;
            std::cout << (i + 1) << ". " << item.name << " - " << price << "G"
                      << " (보유 " << have << ")\n";
        }
        std::cout << "0. 뒤로\n>> " << std::flush;

        int choice = read_int_safe(-1);
        if (choice == 0) return;
        if (choice < 1 || choice > static_cast<int>(sell_items_.size())) continue;
        const auto& item = sell_items_[choice - 1];
        std::cout << "수량: " << std::flush;
        int qty = read_int_safe(0);
        std::string msg;
        sell(player, item, qty, msg);
        std::cout << "\n" << msg << "\n";
        wait_key();
    }
}

bool MarketSystem::buy(Player& player, const MarketItem& item, int qty, std::string& message) {
    if (qty <= 0) {
        message = "수량은 1 이상이어야 합니다.";
        return false;
    }
    if (player.get_level() < item.required_level) {
        message = item.name + "은(는) 플레이어 Lv." + std::to_string(item.required_level) + "이 필요합니다.";
        return false;
    }
    if (player.is_shop_restricted() && item.required_level > 1) {
        message = "상점 신뢰도가 낮습니다. 회복 전까지 기본 품목만 구매할 수 있습니다.";
        return false;
    }
    int unit_price = player.discounted_market_buy_price(item.buy_price);
    int total = unit_price * qty;
    if (!player.spend_money(total)) {
        message = "골드가 부족합니다. 필요: " + std::to_string(total) + "G";
        return false;
    }
    if (!player.add_item(item.item, qty)) {
        player.add_money(total);
        message = "인벤토리 공간이 부족합니다.";
        return false;
    }
    message = item.name + " x" + std::to_string(qty) + " 구매 완료. 비용 "
            + std::to_string(total) + "G.";
    if (unit_price < item.buy_price) message += " INT 할인 적용.";
    return true;
}

bool MarketSystem::sell(Player& player, const MarketItem& item, int qty, std::string& message) {
    if (qty <= 0) {
        message = "수량은 1 이상이어야 합니다.";
        return false;
    }
    if (player.count_item(item.item) < qty) {
        message = "아이템 수량이 부족합니다.";
        return false;
    }

    if (is_monster_food(item.item)) {
        int success = 0;
        int fail = 0;
        int protected_fail = 0;
        int total = 0;
        int price = player.monster_food_sell_price(item.sell_price);
        int chance = player.monster_food_success_chance();

        for (int i = 0; i < qty; ++i) {
            player.remove_item(item.item, 1);
            player.set_sold_monster_food(true);
            if (std::rand() % 100 < chance) {
                ++success;
                total += price;
            } else {
                ++fail;
                if (player.protects_monster_food_failure()) {
                    ++protected_fail;
                } else {
                    player.lower_shop_trust(15);
                }
            }
        }

        if (total > 0) {
            player.add_money(total);
            player.record_sale_income(total);
            player.add_exp(success * 2);
        }

        std::ostringstream oss;
        oss << item.name << " 위험 거래 결과: 성공 " << success
            << ", 실패 " << fail << ", 골드 +" << total
            << "G, 신뢰도 " << player.get_shop_trust() << "/100.";
        if (protected_fail > 0) oss << " 능수능란한 언변으로 실패 " << protected_fail << "회를 막았습니다.";
        message = oss.str();
        return true;
    }

    float multiplier = 1.0f;
    Season season = season_from_day(player.get_day());
    if (item.item == ItemType::RICE && season == Season::SUMMER) multiplier = 1.5f;
    if (item.item == ItemType::APPLE && season == Season::FALL) multiplier = 1.5f;
    if (item.item == ItemType::WHEAT && season == Season::WINTER) multiplier = 1.3f;

    int total = static_cast<int>(item.sell_price * qty * multiplier);
    player.remove_item(item.item, qty);
    player.add_money(total);
    player.record_sale_income(total);
    player.add_exp(qty);
    message = item.name + " x" + std::to_string(qty) + " 판매 완료. 골드 +"
            + std::to_string(total) + "G.";
    return true;
}
