#ifndef INCUBATOR_HPP
#define INCUBATOR_HPP

#include <random> // 🌟 추가됨: mt19937 등을 사용하기 위한 헤더

class Chicken; // 전방 선언

class Incubator {
private:
    std::mt19937 gen;

public:
    Incubator();
    Chicken* tryHatchEgg();
};

#endif // INCUBATOR_HPP