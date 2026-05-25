#include "Kitchen.hpp"
#include "Player.hpp"
#include <iostream>

void Kitchen::interact(Player& player) {
    bool inKitchen = true;
    while(inKitchen) {
        system("clear");
        std::cout << "\n🍳 [주방] 요리를 시작합니다. (현재 레벨: " << kitchenLevel << ")\n";
        std::cout << "1. 주방 업그레이드 (비용 500G)\n";
        std::cout << "2. 요리 만들기\n";
        std::cout << "0. 맵으로 나가기\n>> ";
        int choice;
        std::cin >> choice;

        if (choice == 0) {
            inKitchen = false;
        } else if (choice == 1) {
            if (kitchenLevel >= 3) {
                std::cout << "이미 최고 레벨입니다.\n";
            } else if (player.getGold() >= 500) {
                player.spendGold(500);
                kitchenLevel++;
                std::cout << "🎉 주방 업그레이드 성공! (현재 레벨: " << kitchenLevel << ")\n";
            } else {
                std::cout << "❌ 골드가 부족합니다.\n";
            }
            std::cout << "[아무 숫자나 입력 후 엔터키를 눌러 돌아가기]"; 
            int dummy; std::cin >> dummy;
        } else if (choice == 2) {
            std::cout << "\n[레시피 목록]\n";
            // 🌟 추가됨: 각 레시피 옆에 현재 보유 중인 재료 표시
            std::cout << "1. 🍞 빵 (밀 2개 / 체력 10) \t\t| 🎒 내 식재료: [밀: " << player.getItemCount("밀") << "개]\n";
            std::cout << "2. 🐟 생선구이 (고등어 1개 / 체력 15) \t| 🎒 내 식재료: [고등어: " << player.getItemCount("고등어") << "개]\n";
            std::cout << "3. 🥧 사과파이 (밀 2개, 사과 3개 / 체력 20) \t| 🎒 내 식재료: [밀: " << player.getItemCount("밀") << "개, 사과: " << player.getItemCount("사과") << "개]\n";
            std::cout << ">> 선택: ";
            int r; std::cin >> r;

            if (r == 1) {
                if (player.getItemCount("밀") >= 2 && player.getStamina() >= 10) {
                    player.removeItem("밀", 2);
                    player.consumeStamina(10);
                    player.addItem("빵", 1);
                    std::cout << "🍞 빵을 완성했습니다! (인벤토리에 추가됨)\n";
                    player.earnXP(20);
                } else std::cout << "❌ 재료나 체력이 부족합니다.\n";
            } else if (r == 2) {
                if (player.getItemCount("고등어") >= 1 && player.getStamina() >= 15) {
                    player.removeItem("고등어", 1);
                    player.consumeStamina(15);
                    player.addItem("생선구이", 1);
                    std::cout << "🐟 생선구이를 완성했습니다! (인벤토리에 추가됨)\n";
                    player.earnXP(30);
                } else std::cout << "❌ 재료나 체력이 부족합니다.\n";
            } else if (r == 3) {
                if (kitchenLevel < 2) {
                    std::cout << "❌ 주방 레벨 2가 필요합니다.\n";
                } else if (player.getItemCount("밀") >= 2 && player.getItemCount("사과") >= 3 && player.getStamina() >= 20) {
                    player.removeItem("밀", 2);
                    player.removeItem("사과", 3);
                    player.consumeStamina(20);
                    player.addItem("사과파이", 1);
                    std::cout << "🥧 사과파이를 완성했습니다! (인벤토리에 추가됨)\n";
                    player.earnXP(50);
                } else std::cout << "❌ 재료나 체력이 부족합니다.\n";
            }
            std::cout << "[아무 숫자나 입력 후 엔터키를 눌러 돌아가기]"; 
            int dummy; std::cin >> dummy;
        }
    }
}