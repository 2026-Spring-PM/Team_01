#include "Livestock.hpp"
#include "Player.hpp"

Livestock::Livestock(std::string n, int aAge) 
    : name(n), age(0), adultAge(aAge), isSlaughtered(false), hasProducedToday(false) {}

Livestock::~Livestock() {}

std::string Livestock::getName() const { return name; }
bool Livestock::getIsAdult() const { return age >= adultAge; }
bool Livestock::getIsSlaughtered() const { return isSlaughtered; }
bool Livestock::getHasProducedToday() const { return hasProducedToday; }
void Livestock::setHasProducedToday(bool state) { hasProducedToday = state; }

void Livestock::grow() {
    age++;
    hasProducedToday = false; // 🌟 핵심: 하룻밤 자고 나면 내일 다시 상호작용할 수 있게 초기화
}

Chicken::Chicken() : Livestock("닭", 3) {}
std::pair<ProductType, int> Chicken::produce(Player& player) {
    if (!getIsAdult() || hasProducedToday || !player.consumeStamina(10)) return {ProductType::None, 0};
    hasProducedToday = true; // 상호작용 완료 처리
    player.earnXP(10);
    return {ProductType::Egg, 1};
}

Pig::Pig() : Livestock("돼지", 5) {}
std::pair<ProductType, int> Pig::produce(Player& player) {
    if (!getIsAdult() || hasProducedToday || !player.consumeStamina(20)) return {ProductType::None, 0};
    hasProducedToday = true; // 상호작용 완료 처리
    isSlaughtered = true; 
    player.earnXP(30);
    return {ProductType::Pork, 2};
}

Cow::Cow() : Livestock("소", 4) {}
std::pair<ProductType, int> Cow::produce(Player& player) {
    if (!getIsAdult() || hasProducedToday || !player.consumeStamina(15)) return {ProductType::None, 0};
    hasProducedToday = true; // 상호작용 완료 처리
    player.earnXP(20);
    return {ProductType::Milk, 1};
}