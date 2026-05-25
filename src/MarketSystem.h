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

class MarketSystem {
public:
    MarketSystem();

    void start_new_day();
    void show_buy_shop(Player& player);
    void show_sell_shop(Player& player);

private:
    std::vector<MarketItem> buy_items_;
    std::vector<MarketItem> sell_items_;

    bool buy(Player& player, const MarketItem& item, int qty, std::string& message);
    bool sell(Player& player, const MarketItem& item, int qty, std::string& message);
};
