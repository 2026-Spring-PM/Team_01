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
    if (target_level <= 5) return target_level * 50;
    if (target_level <= 15) return target_level * target_level * 13;
    if (target_level <= 29) return target_level * target_level * target_level;
    return 30000;
}

void LevelSystem::on_level_up() {
    max_stamina_ += 20;
    if (level_ == 10 || level_ == 20 || level_ == 30) {
        max_stamina_ += 50;
    }
}
