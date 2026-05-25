#ifndef SHOPMANAGER_HPP
#define SHOPMANAGER_HPP

#include <string>
#include <vector>
#include <random>
#include <iostream>

class Player;

struct ShopItem {
    std::string name;
    int price;
};

class ShopManager {
private:
    std::vector<ShopItem> pierreItems; 
    std::vector<ShopItem> sellableItems; 
    std::mt19937 gen;

    bool isKrobusHere;
    std::vector<ShopItem> krobusItems;

public:
    ShopManager();

    void startNewDay();
    
    // 🌟 분리된 상점 UI
    void showBuyMenu(Player& player) const;
    void showSellMenu(Player& player) const;
    
    // 메뉴 출력을 위한 개수 반환
    int getBuyItemCount() const { return pierreItems.size(); }
    int getSellItemCount() const { return sellableItems.size(); }

    bool buyFromPierre(Player& player, int itemIndex, int quantity);
    bool sellToPierre(Player& player, int itemIndex, int quantity);
    
    bool sellToLewis(Player& player, const std::string& itemName, int quantity, int unitPrice);

    void showKrobusShop() const;
    bool buyFromKrobus(Player& player, int itemIndex, int quantity);
};

#endif // SHOPMANAGER_HPP