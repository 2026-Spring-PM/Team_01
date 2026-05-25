#include "EventManager.hpp"
#include "Player.hpp"
#include "ConfigManager.hpp"
#include <iostream>
#include <cstdlib>

void EventManager::triggerEvents(Player& player, bool isSummer) {
    int coldChance = ConfigManager::getInstance().getValue<int>("ColdChance");
    int droughtChance = ConfigManager::getInstance().getValue<int>("DroughtChance");

    if (coldChance <= 0) coldChance = 2;
    if (droughtChance <= 0) droughtChance = 5;

    // friend 클래스 권한으로 player의 private 변수에 직접 안전하게 접근합니다.
    if ((rand() % 100) < coldChance) {
        player.maxStamina /= 2;
        if (player.stamina > player.maxStamina) player.stamina = player.maxStamina;
        std::cout << "🤧 [랜덤 이벤트] 에취! 플레이어가 지독한 감기에 걸렸습니다.\n";
        std::cout << "⚠️ 오늘 하루 동안 최대 스태미나가 절반으로 제한됩니다! (현재 최대: " << player.maxStamina << ")\n";
    }

    if (isSummer && droughtTimer == 0) {
        int chance = summerDroughtOccurred ? 1 : droughtChance;
        if ((rand() % 100) < chance) {
            droughtTimer = 4;
            summerDroughtOccurred = true;
            std::cout << "\n🚨 [⚠️대위기 환경 이벤트] 대지에 극심한 가뭄이 찾아왔습니다! 🚨\n";
            std::cout << "☀️ 오늘부터 4일 동안 가뭄 상태가 유지되며, 물을 주지 않은 식물은 내일 사멸합니다!\n\n";
        }
    } else if (droughtTimer > 0) {
        std::cout << "☀️ [환경] 가뭄이 계속 진행 중입니다! (남은 기간: " << droughtTimer << "일)\n";
    }
}

void EventManager::processCrops(std::vector<Crop*>& crops, bool watered) {
    if (droughtTimer > 0) {
        if (!watered) {
            for(auto c : crops) delete c;
            crops.clear();
            std::cout << "🥀 가뭄으로 인해 물을 받지 못한 작물들이 모두 말라 죽었습니다...\n";
        }
        droughtTimer--;
    }
}