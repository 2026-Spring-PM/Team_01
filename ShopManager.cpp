#include "ShopManager.hpp"
#include "Player.hpp"
#include <iomanip> 

ShopManager::ShopManager() : isKrobusHere(false) {
    std::random_device rd;
    gen.seed(rd());

    // 피에르 구매 가능 목록
    pierreItems.push_back({"감자 씨앗", 5});
    pierreItems.push_back({"당근 씨앗", 10});
    pierreItems.push_back({"밀 씨앗", 15});
    pierreItems.push_back({"쌀 씨앗", 25});
    pierreItems.push_back({"사과 묘목", 250});
    pierreItems.push_back({"나무 (건축자재)", 15}); 
    pierreItems.push_back({"새끼 닭", 40});
    pierreItems.push_back({"새끼 돼지", 100});
    pierreItems.push_back({"새끼 소", 500});

    // 판매 가능 목록 (광석 3종 추가 연동)
    sellableItems.push_back({"감자", 12});
    sellableItems.push_back({"당근", 25});
    sellableItems.push_back({"밀", 35});
    sellableItems.push_back({"쌀", 60});
    sellableItems.push_back({"사과", 150});
    sellableItems.push_back({"달걀", 20});
    sellableItems.push_back({"우유", 80});
    sellableItems.push_back({"돼지고기", 300});
    sellableItems.push_back({"고등어", 40});
    sellableItems.push_back({"광어", 75});
    sellableItems.push_back({"참치", 250});
    sellableItems.push_back({"빵", 100});
    sellableItems.push_back({"사과파이", 350});
    sellableItems.push_back({"생선구이", 120});
    sellableItems.push_back({"구리 광석", 15}); // 새로 추가됨
    sellableItems.push_back({"철 광석", 30});   // 새로 추가됨
    sellableItems.push_back({"금 광석", 75});    // 새로 추가됨
}

void ShopManager::startNewDay() {
    isKrobusHere = false;
    krobusItems.clear();
    std::uniform_int_distribution<int> chanceDist(1, 100);
    if (chanceDist(gen) <= 10) isKrobusHere = true;
}

void ShopManager::showBuyMenu(Player& player) const {
    std::cout << "\n==========================================================\n";
    std::cout << "🛒 [피에르의 잡화점 - 구매] 필요한 물건을 골라보세요!\n";
    std::cout << "==========================================================\n";
    std::cout << "💰 보유 골드: " << player.getGold() << " G\n";
    std::cout << "----------------------------------------------------------\n";
    
    for (size_t i = 0; i < pierreItems.size(); ++i) {
        std::string itemName = pierreItems[i].name;
        std::cout << "  (" << i << ") " << itemName << " : " << pierreItems[i].price << " G \t| 🎒 현재 보유: " << player.getItemCount(itemName) << "개\n";
    }
    // 하단에 자재류 보유 현황도 함께 서포트 출력하여 가시성 확보
    std::cout << "----------------------------------------------------------\n";
    std::cout << "💎 광물자재 현황: [구리: " << player.getItemCount("구리 광석") << "개] ";
    std::cout << "[철: " << player.getItemCount("철 광석") << "개] ";
    std::cout << "[금: " << player.getItemCount("금 광석") << "개]\n";
    std::cout << "==========================================================\n";
}

void ShopManager::showSellMenu(Player& player) const {
    std::cout << "\n==========================================================\n";
    std::cout << "💰 [피에르의 잡화점 - 판매] 무엇을 파시겠어요?\n";
    std::cout << "==========================================================\n";
    std::cout << "💰 보유 골드: " << player.getGold() << " G\n";
    std::cout << "----------------------------------------------------------\n";
    
    for (size_t i = 0; i < sellableItems.size(); ++i) {
        std::string itemName = sellableItems[i].name;
        int count = player.getItemCount(itemName);
        if (count > 0) {
            std::cout << "▶ (" << std::setw(2) << i << ") " << std::setw(16) << itemName << " (개당 " << sellableItems[i].price << " G) \t| 🎒 판매 가능: " << count << "개\n";
        } else {
            std::cout << "  (" << std::setw(2) << i << ") " << std::setw(16) << itemName << " (개당 " << sellableItems[i].price << " G) \t| 🎒 보유 없음\n";
        }
    }
    std::cout << "==========================================================\n";
}

bool ShopManager::buyFromPierre(Player& player, int itemIndex, int quantity) {
    if (itemIndex < 0 || itemIndex >= static_cast<int>(pierreItems.size())) return false;
    int totalCost = pierreItems[itemIndex].price * quantity;
    if (player.spendGold(totalCost)) {
        player.addItem(pierreItems[itemIndex].name, quantity);
        std::cout << "💸 [" << pierreItems[itemIndex].name << "] " << quantity << "개를 " << totalCost << " G에 구매했습니다.\n";
        return true;
    }
    std::cout << "❌ 골드가 부족합니다.\n";
    return false;
}

bool ShopManager::sellToPierre(Player& player, int itemIndex, int quantity) {
    if (itemIndex < 0 || itemIndex >= static_cast<int>(sellableItems.size())) return false;
    std::string itemName = sellableItems[itemIndex].name;
    int available = player.getItemCount(itemName);
    if (available < quantity) {
        std::cout << "❌ 인벤토리에 판매할 [" << itemName << "] 수량이 부족합니다!\n";
        return false;
    }
    int revenue = sellableItems[itemIndex].price * quantity;
    player.removeItem(itemName, quantity);
    player.addGold(revenue);
    std::cout << "💰 [" << itemName << "] " << quantity << "개를 판매하여 " << revenue << " G를 획득했습니다!\n";
    return true;
}

bool ShopManager::sellToLewis(Player& player, const std::string& itemName, int quantity, int unitPrice) {
    player.addGold(unitPrice * quantity);
    return true;
}

void ShopManager::showKrobusShop() const {}
bool ShopManager::buyFromKrobus(Player& player, int itemIndex, int quantity) { return false; }