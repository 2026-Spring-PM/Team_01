#include "LevelSystem.h"

#include <algorithm>

LevelSystem::LevelSystem()
    : level_(1),
      exp_(0),
      max_stamina_(INITIAL_STAMINA) {
}

bool LevelSystem::add_exp(int exp) {
    if (level_ >= MAX_LEVEL) {
        return false;  // 만렙이면 더 이상 레벨업 X
    }

    exp_ += exp;
    bool leveled_up = false;

    // 한 번에 여러 레벨업도 처리
    while (level_ < MAX_LEVEL && exp_ >= required_exp_for_level(level_ + 1)) {
        ++level_;
        on_level_up();
        leveled_up = true;
    }

    return leveled_up;
}

int LevelSystem::get_exp_to_next_level() const {
    if (level_ >= MAX_LEVEL) {
        return 0;
    }
    return required_exp_for_level(level_ + 1) - exp_;
}

// 누적 EXP
// Lv.1->2: 50, Lv.2->3: 100, ... 단순한 공식으로 근사
int LevelSystem::required_exp_for_level(int target_level) {
    if (target_level <= 1) return 0;
    if (target_level > MAX_LEVEL) return 30000;

    // 누적: Lv 도달 시점의 총 누적 EXP
    // Lv.5 ≈ 300, Lv.15 ≈ 3000, Lv.30 ≈ 28000
    // 단순화: 50 * level * level / 2 정도
    return 25 * target_level * (target_level - 1);
}

void LevelSystem::on_level_up() {
    // 기본 +20
    max_stamina_ += 20;

    // 보너스 구간: 10, 20, 30
    if (level_ == 10 || level_ == 20 || level_ == 30) {
        max_stamina_ += 50;
    }
}