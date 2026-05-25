#include "Workshop.hpp"
#include "Player.hpp"
#include <iostream>

void Workshop::interact(Player& player) {
    bool inWorkshop = true;
    while(inWorkshop) {
        system("clear");
        std::cout << "\n🔨 [작업장] 무엇을 하시겠습니까? (현재 레벨: " << workshopLevel << ")\n";
        std::cout << "1. 작업장 증축\n";
        std::cout << "2. 농기구 업그레이드 (현재 농기구 등급: " << player.getToolGrade() << ")\n";
        std::cout << "0. 맵으로 나가기\n>> ";
        int choice;
        std::cin >> choice;

        if (choice == 0) {
            inWorkshop = false;
        } else if (choice == 1) {
            int cost = (workshopLevel == 0) ? 200 : (workshopLevel == 1 ? 600 : 2000);
            int sCost = (workshopLevel == 0) ? 80 : (workshopLevel == 1 ? 150 : 300);
            int reqLv = (workshopLevel == 0) ? 6 : (workshopLevel == 1 ? 10 : 20);

            std::cout << "\n[증축 조건] 레벨 " << reqLv << " / 골드 " << cost << "G / 체력 " << sCost << "\n";
            if (workshopLevel >= 2) {
                std::cout << "최고 레벨입니다.\n";
            } else if (player.getLevel() < reqLv || player.getGold() < cost || player.getStamina() < sCost) {
                std::cout << "❌ 조건이 부족합니다.\n";
            } else {
                player.spendGold(cost);
                player.consumeStamina(sCost);
                workshopLevel++;
                std::cout << "🎉 증축 성공! (현재 레벨: " << workshopLevel << ")\n";
            }
            std::cout << "[아무 숫자나 입력 후 엔터키를 눌러 돌아가기]"; 
            int dummy; std::cin >> dummy;
        } else if (choice == 2) {
            int tg = player.getToolGrade();
            int target = tg + 1;
            if (target > 3) {
                std::cout << "이미 최고 등급(금속)입니다.\n";
            } else if (workshopLevel < target) {
                std::cout << "❌ 작업장 레벨이 낮아 업그레이드할 수 없습니다. (필요: " << target << ")\n";
            } else {
                int reqGold = (target == 1) ? 100 : (target == 2 ? 300 : 800);
                int reqWood = (target == 1) ? 5 : (target == 2 ? 10 : 20);
                
                std::string oreName = (target == 1) ? "구리 광석" : (target == 2 ? "철 광석" : "금 광석");
                int reqOre = (target == 1) ? 3 : (target == 2 ? 4 : 2);

                std::cout << "\n[업그레이드 필요 조건] " << reqGold << "G / 나무 " << reqWood << "개 / " << oreName << " " << reqOre << "개 / 체력 30\n";
                // 🌟 추가됨: 현재 내 가방에 있는 보유 재료량 표시
                std::cout << "🎒 현재 내 보유 재료: [나무: " << player.getItemCount("나무 (건축자재)") << "개] / [" << oreName << ": " << player.getItemCount(oreName) << "개]\n";
                
                if (player.getGold() >= reqGold && player.getItemCount("나무 (건축자재)") >= reqWood && player.getItemCount(oreName) >= reqOre && player.getStamina() >= 30) {
                    player.spendGold(reqGold);
                    player.removeItem("나무 (건축자재)", reqWood);
                    player.removeItem(oreName, reqOre);
                    player.consumeStamina(30);
                    player.setToolGrade(target);
                    player.earnXP(target * 200);
                    std::cout << "🎉 농기구 업그레이드 완료! (수확 시 소모되는 스태미나가 줄어듭니다!)\n";
                } else {
                    std::cout << "❌ 재료, 골드 또는 스태미나가 부족합니다.\n";
                }
            }
            std::cout << "[아무 숫자나 입력 후 엔터키를 눌러 돌아가기]"; 
            int dummy; std::cin >> dummy;
        }
    }
}