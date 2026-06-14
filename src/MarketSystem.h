#pragma once

#include "Player.h"
#include "ItemType.h"
#include <string>
#include <vector>

struct MarketItem {
    ItemType item;
    std::string name;
    int buy_price;
    int sell_price;
    int required_level;
};

// 선물 아이템 정보: 주방 레벨별로 하나씩
struct GiftItem {
    ItemType item;
    std::string name;
    int kitchen_level_required; // 해당 음식을 만들려면 필요한 주방 레벨
    int trust_restore;           // 선물 시 회복되는 신용도
};

class MarketSystem {
public:
    MarketSystem();

    void start_new_day();
    void show_buy_shop(Player& player);
    void show_sell_shop(Player& player);

    // 괴식 판매 전용 (신용도 기반 대사 포함)
    void show_monster_food_shop(Player& player, int kitchen_level);

    // 선물하기
    void show_gift_shop(Player& player, int kitchen_level);

    const std::vector<MarketItem>& buy_items() const { return buy_items_; }
    const std::vector<MarketItem>& sell_items() const { return sell_items_; }
    bool buy_item(Player& player, std::size_t index, int qty, std::string& message);
    bool sell_item(Player& player, std::size_t index, int qty, std::string& message);

    static const GiftItem GIFT_ITEMS[3]; // 주방 Lv.2, 3, 4 선물용

private:
    std::vector<MarketItem> buy_items_;
    std::vector<MarketItem> sell_items_;

    bool buy(Player& player, const MarketItem& item, int qty, std::string& message);
    bool sell(Player& player, const MarketItem& item, int qty, std::string& message);

    // 괴식 판매 1개 처리 (대사 반환)
    bool sell_monster_food_one(Player& player, const MarketItem& item, std::string& result_msg);
};
