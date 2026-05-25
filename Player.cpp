#include "Player.hpp"
#include "ConfigManager.hpp"
#include <iostream>

// 생성자에 x(5), y(5) 시작 위치 초기화 추가됨
Player::Player() : gold(25), currentToolGrade(0), x(5), y(5) {
    maxStamina = ConfigManager::getInstance().getValue<int>("StartStamina");
    if (maxStamina <= 0) maxStamina = 150; 
    stamina = maxStamina;

    // 🌟 테스트용 초기 아이템 지급
    addItem("감자 씨앗", 3);
    addItem("새끼 닭", 1);
}

bool Player::consumeStamina(int amount) {
    if (stamina >= amount) {
        stamina -= amount;
        return true;
    }
    std::cout << "❌ 스태미나가 부족합니다! (현재: " << stamina << " / 필요: " << amount << ")\n";
    return false;
}

void Player::sleep() {
    stamina = maxStamina;
    std::cout << "🛏️ 잠을 자고 일어났습니다. 스태미나가 모두 회복되었습니다. (" << stamina << "/" << maxStamina << ")\n";
}

void Player::earnXP(int amount) {
    int levelUps = levelSystem.gainXP(amount);
    for (int i = 0; i < levelUps; ++i) {
        int reachedLevel = levelSystem.getLevel() - levelUps + 1 + i;
        handleLevelUp(reachedLevel);
    }
}

void Player::handleLevelUp(int reachedLevel) {
    maxStamina += 20; 
    if (reachedLevel == 10 || reachedLevel == 20 || reachedLevel == 30) {
        maxStamina += 50;
    }
    stamina = maxStamina; 
}

int Player::getStamina() const { return stamina; }
int Player::getMaxStamina() const { return maxStamina; }
int Player::getGold() const { return gold; }

void Player::addGold(int amount) { gold += amount; }

bool Player::spendGold(int amount) {
    if (gold >= amount) {
        gold -= amount;
        return true;
    }
    return false;
}

int Player::getToolGrade() const { return currentToolGrade; }
void Player::setToolGrade(int grade) { currentToolGrade = grade; }
int Player::getLevel() const { return levelSystem.getLevel(); }