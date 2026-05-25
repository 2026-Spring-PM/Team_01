#include "Crop.hpp"
#include "Player.hpp" // 스태미나 및 경험치 함수 호출용

Crop::Crop(std::string n, int gt, int bp, bool perennial) 
    : name(n), growthTime(gt), currentGrowth(0), basePrice(bp), isPerennial(perennial), isReadyToHarvest(false) {
    std::random_device rd;
    gen.seed(rd());
}

void Crop::grow() {
    if (!isReadyToHarvest) {
        currentGrowth++;
        if (currentGrowth >= growthTime) {
            isReadyToHarvest = true;
        }
    }
}

int Crop::harvest(Player& player) {
    if (!isReadyToHarvest) {
        std::cout << "🌱 아직 [" << name << "]이(가) 다 자라지 않았습니다.\n";
        return 0;
    }
    
    // 농기구 등급에 따른 스태미나 경감 로직
    int staminaCost = 10;
    int toolGrade = player.getToolGrade();
    if (toolGrade == 1) staminaCost = 7;
    else if (toolGrade == 2) staminaCost = 5;
    else if (toolGrade == 3) staminaCost = 2;

    // 플레이어 체력 차감 시도
    if (!player.consumeStamina(staminaCost)) {
        return 0; // 스태미나 부족 시 수확 취소
    }

    int yield = calculateYield();
    std::cout << "🌾 [" << name << "] 수확 완료! (" << yield << "개 획득) / 체력 -" << staminaCost << "\n";
    
    // 경험치 지급 (기획서 밸런스 기준)
    if (name == "밀" || name == "쌀") player.earnXP(20);
    else player.earnXP(5); // 감자, 당근

    return yield; // 수확된 개수를 반환 (인벤토리에 추가해야 함)
}


// ==========================================
// 파생 클래스들의 개별 수확량 확률(가중치) 로직
// ==========================================

// 감자 (1일 / 2~5개 로또성)
Potato::Potato() : Crop("감자", 1, 5) {}
int Potato::calculateYield() {
    std::discrete_distribution<int> dist{40, 30, 20, 10}; // 2개(40%), 3개(30%), 4개(20%), 5개(10%)
    return dist(gen) + 2;
}

// 당근 (1일 / 1~3개 안정성)
Carrot::Carrot() : Crop("당근", 1, 12) {}
int Carrot::calculateYield() {
    std::discrete_distribution<int> dist{20, 40, 40}; // 1개(20%), 2개(40%), 3개(40%)
    return dist(gen) + 1;
}

// 밀 (2일 / 2~4개)
Wheat::Wheat() : Crop("밀", 2, 20) {}
int Wheat::calculateYield() {
    std::discrete_distribution<int> dist{30, 40, 30}; 
    return dist(gen) + 2;
}

// 쌀 (3일 / 4~6개)
Rice::Rice() : Crop("쌀", 3, 25) {}
int Rice::calculateYield() {
    std::discrete_distribution<int> dist{30, 40, 30}; 
    return dist(gen) + 4;
}


// ==========================================
// 🍎 사과나무 (영구 작물) 특수 오버라이딩 로직
// ==========================================
AppleTree::AppleTree() : Crop("사과나무", 5, 30, true), harvestCooldown(0) {}

void AppleTree::grow() {
    if (!isReadyToHarvest) {
        if (currentGrowth < growthTime) {
            currentGrowth++; // 최초 5일 자라기
            if (currentGrowth == growthTime) isReadyToHarvest = true;
        } else {
            // 다 자란 후에는 2일마다 사과가 열림
            harvestCooldown++; 
            if (harvestCooldown >= 2) {
                isReadyToHarvest = true;
                harvestCooldown = 0;
            }
        }
    }
}

int AppleTree::calculateYield() {
    std::discrete_distribution<int> dist{20, 40, 30, 10}; // 3, 4, 5, 6개
    return dist(gen) + 3;
}

int AppleTree::harvest(Player& player) {
    if (!isReadyToHarvest) {
        std::cout << "🌳 아직 사과가 열리지 않았습니다.\n";
        return 0;
    }
    
    // 사과 수확도 농기구 효과를 받도록 스태미나 계산
    int staminaCost = 10;
    int toolGrade = player.getToolGrade();
    if (toolGrade == 1) staminaCost = 7;
    else if (toolGrade == 2) staminaCost = 5;
    else if (toolGrade == 3) staminaCost = 2;

    if (!player.consumeStamina(staminaCost)) return 0;

    int yield = calculateYield();
    std::cout << "🍎 [사과] 수확 완료! (" << yield << "개 획득) / 체력 -" << staminaCost << "\n";
    player.earnXP(40); // 사과는 고급 작물이므로 40 XP 부여
    
    // 부모 클래스와 핵심 차이: 영구 작물이므로 객체가 소멸하지 않고 다시 열매 맺을 준비(false)로 변경됨
    isReadyToHarvest = false; 
    return yield;
}