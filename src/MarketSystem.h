#ifndef MARKET_SYSTEM_H
#define MARKET_SYSTEM_H

#include "MarketData.h"
#include <iostream>
#include <vector>
#include <map>
#include <random>
#include <iomanip>

using namespace std;

class Inventory {
public:
    map<ItemID, int> items;

    void addItem(ItemID id, int quantity) {
        items[id] += quantity;
    }

    bool removeItem(ItemID id, int quantity) {
        if (items[id] >= quantity) {
            items[id] -= quantity;
            if (items[id] == 0) items.erase(id);
            return true;
        }
        return false;
    }

    int getQuantity(ItemID id) {
        return items.count(id) ? items[id] : 0;
    }
};


class Player {
private: 
    int gold; // Now PRIVATE for safety!

public:
    Inventory inventory;
    Player(int startingGold) : gold(startingGold) {}

    // --> NEW: Friend Class! <--
    // The Renderer is allowed to access the private 'gold' variable to draw the screen.
    friend class ConsoleRenderer; 

    // We also need a public getter so shops can process transactions
    int getGold() const { return gold; }
    void deductGold(int amount) { gold -= amount; }
    void addGold(int amount) { gold += amount; }
};


class ConsoleRenderer {
public:
    // Draws the Player's current status (uses the Friend permission to see 'gold')
    void drawHUD(const Player& player, int currentDay) {
        cout << "\n========================================\n";
        cout << " DAY " << currentDay << " | Wallet: " << player.getGold() << "g \n";
        cout << "========================================\n";
    }

    // --> NEW: Template Function! <--
    // This template can draw a boxed menu for a vector of ANYTHING (Items, Strings, etc.)
    template <typename T>
    void drawMenu(string title, const vector<T>& items) {
        cout << "\n--- [ " << title << " ] ---\n";
        for (int i = 0; i < items.size(); i++) {
            // This works magically because we overloaded the << operator for Item!
            cout << "[" << i << "] " << items[i] << "\n";
        }
        cout << "----------------------------------------\n";
    }
};

// ... (Keep the Shop, PierreShop, LewisShop, and KrobusShop classes the same, 
// just remember to use player.getGold() and player.deductGold() inside Pierre and Lewis!)
// Base Shop Class
class Shop {
protected:
    string shopName;
public:
    vector<Item> stock;
    Shop(string name) : shopName(name) {}
    string getName() { return shopName; }
    
    // --> NEW: Virtual Function to demonstrate polymorphism! <--
    virtual void greetPlayer() {
        cout << "Welcome to " << shopName << "!\n";
    }
};

// NPC A: Pierre
class PierreShop : public Shop {
public:
    PierreShop() : Shop("Pierre's Seed & Animal Shop") {
        stock = {
            {ITEM_POTATO_SEED, "Potato Seed", 5, 0},
            {ITEM_CARROT_SEED, "Carrot Seed", 10, 0},
            {ITEM_WHEAT_SEED, "Wheat Seed", 15, 0},
            {ITEM_RICE_SEED, "Rice Seed", 25, 0},
            {ITEM_APPLE_SAPLING, "Apple Sapling", 250, 0},
            {ITEM_WOOD, "Wood", 15, 0}
        };
    }

    // Overriding the virtual function
    void greetPlayer() override {
        cout << "Pierre: Need some seeds? You came to the right place!\n";
    }

    void buyItem(Player& player, int stockIndex, int quantity) {
        if (stockIndex < 0 || stockIndex >= stock.size()) return;
        
        Item selected = stock[stockIndex];
        int totalCost = selected.buyPrice * quantity;

        if (player.getGold() >= totalCost) {
            player.deductGold(totalCost);
            player.inventory.addItem(selected.id, quantity);
            cout << "Bought " << quantity << "x " << selected.name << " for " << totalCost << "g.\n";
        } else {
            cout << "Not enough gold!\n";
        }
    }
};

// NPC B: Lewis
class LewisShop : public Shop {
private:
    map<ItemID, Item> priceDatabase;
public:
    LewisShop() : Shop("Lewis's Produce Exchange") {
        priceDatabase[ITEM_POTATO] = {ITEM_POTATO, "Potato", 0, 5};
        priceDatabase[ITEM_CARROT] = {ITEM_CARROT, "Carrot", 0, 12};
        priceDatabase[ITEM_WHEAT] = {ITEM_WHEAT, "Wheat", 0, 20};
    }

    // Overriding the virtual function
    void greetPlayer() override {
        cout << "Lewis: Drop your goods in the bin, I'll pay top dollar.\n";
    }

    void sellItem(Player& player, ItemID id, int quantity) {
        if (player.inventory.getQuantity(id) >= quantity) {
            if (priceDatabase.count(id)) {
                int earnings = priceDatabase[id].sellPrice * quantity;
                player.inventory.removeItem(id, quantity);
                player.addGold(earnings);
                cout << "Sold " << quantity << "x " << priceDatabase[id].name << " for " << earnings << "g.\n";
            }
        } else {
            cout << "You don't have enough of that item to sell.\n";
        }
    }
};

// Special NPC: Krobus
class KrobusShop : public Shop {
public:
    bool isPresent;

    KrobusShop() : Shop("Krobus's Black Market"), isPresent(false) {}

    // Overriding the virtual function
    void greetPlayer() override {
        cout << "Krobus: ...Don't tell anyone I'm here. Look at these discounts...\n";
    }

    void rollDailyAppearance(const vector<Item>& pierreStock) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> spawnChance(1, 100);

        if (spawnChance(gen) <= 10) {
            isPresent = true;
            stock.clear();
            
            uniform_int_distribution<> itemPick(0, pierreStock.size() - 1);
            uniform_int_distribution<> discountPick(2, 4);
            int numItems = (spawnChance(gen) > 50) ? 2 : 1; 

            for (int i = 0; i < numItems; i++) {
                Item discountedItem = pierreStock[itemPick(gen)];
                int discountPercent = discountPick(gen) * 10;
                discountedItem.buyPrice = discountedItem.buyPrice * (100 - discountPercent) / 100;
                stock.push_back(discountedItem);
            }
            cout << "\n*** Krobus has appeared today! ***\n";
        } else {
            isPresent = false;
        }
    }
};

#endif
