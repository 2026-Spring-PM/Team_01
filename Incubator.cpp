#include "Incubator.hpp"
#include "Livestock.hpp" // 🌟 추가됨: Chicken 객체 생성을 위해 필요
#include <iostream>      // 🌟 추가됨: std::cout 출력을 위해 필요

Incubator::Incubator() {
    std::random_device rd;
    gen.seed(rd());
}

Chicken* Incubator::tryHatchEgg() {
    // 65% 확률로 0(실패), 35% 확률로 1(성공) 반환
    std::discrete_distribution<int> hatchDist{65, 35};
    bool isSuccess = hatchDist(gen); 

    if (isSuccess) {
        std::cout << "🐣 [부화기] 삐약! 35%의 확률을 뚫고 새끼 닭이 태어났습니다! (40원 이득)\n";
        return new Chicken();
    } else {
        std::cout << "🍳 [부화기] 아쉽게도 달걀 부화에 실패했습니다...\n";
        return nullptr;
    }
}