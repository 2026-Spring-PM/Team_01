#ifndef LEVELSYSTEM_HPP
#define LEVELSYSTEM_HPP

class LevelSystem {
private:
    int level;
    int currentXP;
    int maxXP;

    // 프롬프트에 명시된 레벨별 곡선형 경험치 계산 공식
    int calculateMaxXP(int targetLevel) const;

public:
    LevelSystem();

    // 경험치를 획득하고, 레벨업 한 횟수를 반환합니다.
    int gainXP(int amount);

    // Getter
    int getLevel() const;
    int getCurrentXP() const;
    int getMaxXP() const;
};

#endif // LEVELSYSTEM_HPP