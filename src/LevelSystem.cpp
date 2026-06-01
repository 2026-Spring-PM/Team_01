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

// 경험치 테이블 (PDF 기준, 1/10 스케일)
// Lv1-4: level*5
// Lv5: 30
// Lv6: 50
// Lv7-9: level*10
// Lv10: 150
// Lv11-15: 200
// Lv16: 400
// Lv17-19: 500
// Lv20: 1000
// Lv21-29: 1500
// Lv30: 3000
int LevelSystem::required_exp_for_level(int target_level) {
    if (target_level <= 1) return 0;
    if (target_level <= 4) return target_level * 5;
    if (target_level == 5) return 30;
    if (target_level == 6) return 50;
    if (target_level <= 9) return target_level * 10;
    if (target_level == 10) return 150;
    if (target_level <= 15) return 200;
    if (target_level == 16) return 400;
    if (target_level <= 19) return 500;
    if (target_level == 20) return 1000;
    if (target_level <= 29) return 1500;
    return 3000; // Lv30
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
