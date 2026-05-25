#ifndef MARKET_DATA_H
#define MARKET_DATA_H

#include <string>
#include <iomanip>

using namespace std; 

enum ItemID {
    ITEM_POTATO_SEED, ITEM_CARROT_SEED, ITEM_WHEAT_SEED, ITEM_RICE_SEED, ITEM_APPLE_SAPLING,
    ITEM_CHICKEN, ITEM_PIG, ITEM_COW, ITEM_WOOD,
    ITEM_POTATO, ITEM_CARROT, ITEM_WHEAT, ITEM_RICE, ITEM_APPLE,
    ITEM_EGG, ITEM_MEAT, ITEM_MILK,
    ITEM_MACKEREL, ITEM_FLATFISH, ITEM_TUNA,
    ITEM_COPPER, ITEM_IRON, ITEM_GOLD,
    ITEM_BREAD, ITEM_FISH_KEBAB, ITEM_MEAT_PIE, ITEM_CAKE, ITEM_APPLE_PIE
};

struct Item {
    ItemID id;
    string name;
    int buyPrice;
    int sellPrice;
    friend ostream& operator<<(ostream& os, const Item& item) {
        os << left << setw(15) << item.name 
           << " | Buy: " << setw(4) << item.buyPrice << "g"
           << " | Sell: " << setw(4) << item.sellPrice << "g";
        return os;
    }
};

#endif