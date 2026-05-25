#ifndef CROP_HPP
#define CROP_HPP

#include <string>
#include <random>
#include <iostream>

// 전방 선언 (Player 클래스의 스태미나 차감/경험치 연동을 위함)
class Player;

// 최상위 작물 기본(Base) 클래스
class Crop {
protected:
    std::string name;
    int growthTime;       // 성장에 필요한 총 일수
    int currentGrowth;    // 현재 자라난 일수
    int basePrice;        // 개당 판매가
    bool isPerennial;     // 영구 작물 여부 (사과나무 등)
    bool isReadyToHarvest;// 수확 가능 상태
    
    std::mt19937 gen;     // 수확 확률 계산용 난수 엔진

public:
    Crop(std::string n, int gt, int bp, bool perennial = false);
    virtual ~Crop() = default;

    // 매일 밤 자고 일어날 때 호출되는 성장 함수
    virtual void grow(); 
    
    // 수확 시 얻는 개수를 반환하는 순수 가상 함수 (자식 클래스에서 필수 구현)
    virtual int calculateYield() = 0; 

    // 수확 행동 처리 (다형성 활용)
    virtual int harvest(Player& player);

    // Getter
    std::string getName() const { return name; }
    int getBasePrice() const { return basePrice; }
    bool getIsReady() const { return isReadyToHarvest; }
    bool getIsPerennial() const { return isPerennial; }
};

// --- 파생(Derived) 클래스들 ---

class Potato : public Crop {
public:
    Potato();
    int calculateYield() override;
};

class Carrot : public Crop {
public:
    Carrot();
    int calculateYield() override;
};

class Wheat : public Crop {
public:
    Wheat();
    int calculateYield() override;
};

class Rice : public Crop {
public:
    Rice();
    int calculateYield() override;
};

class AppleTree : public Crop {
private:
    int harvestCooldown; // 사과나무 전용 수확 주기 카운터 (2일)
public:
    AppleTree();
    void grow() override;             // 사과나무만의 독자적인 성장 로직
    int calculateYield() override;
    int harvest(Player& player) override; // 파괴되지 않는 특수 수확 로직
};

#endif // CROP_HPP