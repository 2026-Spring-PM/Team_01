#include "LevelSystem.h"

LevelSystem::LevelSystem()
    : level_(1), exp_(0), max_stamina_(INITIAL_STAMINA) {}

bool LevelSystem::add_exp(int exp) {
    if (exp <= 0 || level_ >= MAX_LEVEL) return false;

    exp_ += exp;
    bool leveled_up = false;

    while (level_ < MAX_LEVEL && exp_ >= required_exp_for_level(level_ + 1)) {
        ++level_;
        on_level_up();
        leveled_up = true;
    }
    return leveled_up;
}

int LevelSystem::get_exp_to_next_level() const {
    if (level_ >= MAX_LEVEL) return 0;
    return required_exp_for_level(level_ + 1) - exp_;
}

int LevelSystem::required_exp_for_level(int target_level) {
if (target_level <= 1) return 0;
    if (target_level <= 4) return target_level * 5;
    if (target_level == 5) return 30;
    if (target_level == 6) return 50;
    if (target_level >= 7 && target_level <= 9) return target_level * 10;
    if (target_level == 10) return 150;
    if (target_level >= 11 && target_level <= 15) return 200;
    if (target_level == 16) return 400;
    if (target_level >= 17 && target_level <= 19) return 500;
    if (target_level == 20) return 1000;
    if (target_level >= 21 && target_level <= 29) return 1500;
    return 30000; // 30 이상
}

void LevelSystem::on_level_up() {
    max_stamina_ += 1; // 기본 +1
    // 특정 레벨에서 보너스 +2 (합쳐서 +3이 되도록 하려면 += 2로, 기획상 총 +2가 되도록 하려면 += 1로 합니다. 기획안의 "보너스 +2"를 추가로 해석하여 +2 반영)
    if (level_ == 6 || level_ == 10 || level_ == 16 || level_ == 20 || level_ == 26 || level_ == 30) {
        max_stamina_ += 2;
    }
}
