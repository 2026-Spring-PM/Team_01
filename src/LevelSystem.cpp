#include "LevelSystem.h"

LevelSystem::LevelSystem()
    : level_(1), exp_(0), max_stamina_(INITIAL_STAMINA) {}

bool LevelSystem::add_exp(int exp) {
    if (exp <= 0 || level_ >= MAX_LEVEL) return false;

    exp_ += exp;
    bool leveled_up = false;

    while (level_ < MAX_LEVEL) {
        int needed = required_exp_for_level(level_ + 1);
        if (exp_ >= needed) {
            exp_ -= needed;   // ← 차감!
            ++level_;
            on_level_up();
            leveled_up = true;
        } else {
            break;
        }
    }

    return leveled_up;
}

int LevelSystem::get_exp_to_next_level() const {
    if (level_ >= MAX_LEVEL) return 0;
    return required_exp_for_level(level_ + 1) - exp_;
}

// 경험치 테이블: 다음 레벨 요구 경험치 = target_level * 10
int LevelSystem::required_exp_for_level(int target_level) {
    if (target_level <= 1) return 0;
    return target_level * 10;
}

bool LevelSystem::is_bonus_level(int level) {
    return level == 6 || level == 10 || level == 16 ||
           level == 20 || level == 26 || level == 30;
}

void LevelSystem::on_level_up() {
    // 기본 +1, 보너스 레벨에서 추가 +1
    max_stamina_ += 1;
    if (is_bonus_level(level_)) {
        max_stamina_ += 1;
    }
}
