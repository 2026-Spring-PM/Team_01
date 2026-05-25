#include "LevelSystem.hpp"

LevelSystem::LevelSystem() : level(1), currentXP(0) {
    maxXP = calculateMaxXP(level);
}

int LevelSystem::calculateMaxXP(int targetLevel) const {
    if (targetLevel >= 30) {
        return 30000; // 30레벨 이상은 요구 경험치 고정
    }
    
    if (targetLevel <= 5) {
        return targetLevel * 50;
    } else if (targetLevel <= 15) {
        return targetLevel * targetLevel * 13;
    } else { // 16 ~ 29
        return targetLevel * targetLevel * targetLevel * 1;
    }
}

int LevelSystem::gainXP(int amount) {
    currentXP += amount;
    int levelUpCount = 0;

    // 한 번에 많은 경험치를 얻어 여러 번 레벨업 할 경우를 대비한 while 루프
    while (currentXP >= maxXP) {
        currentXP -= maxXP;
        level++;
        maxXP = calculateMaxXP(level);
        levelUpCount++;
    }

    return levelUpCount;
}

int LevelSystem::getLevel() const { return level; }
int LevelSystem::getCurrentXP() const { return currentXP; }
int LevelSystem::getMaxXP() const { return maxXP; }