#include "EventSystem.h"

#include <cstdlib>

EventSystem::EventSystem()
    : cold_chance_(2), drought_chance_(5), drought_days_left_(0) {}

bool EventSystem::is_summer(int day) const {
    return day >= 26 && day <= 50;
}

void EventSystem::process_new_day(Player& player, Map& map, std::string& message) {
    message.clear();

    if (drought_days_left_ > 0) {
        map.kill_unwatered_crops_during_drought();
        --drought_days_left_;
        message += "☀️ 가뭄이 계속되어 물을 주지 않은 작물이 말랐습니다. 남은 기간: "
                 + std::to_string(drought_days_left_) + "일. ";
    }

    int cold_roll = std::rand() % 100;
    if (cold_roll < cold_chance_) {
        player.apply_cold_penalty();
        message += "🤧 감기에 걸려 오늘 최대 스태미나가 절반으로 제한되었습니다. ";
    }

    if (is_summer(player.get_day()) && drought_days_left_ == 0) {
        int drought_roll = std::rand() % 100;
        if (drought_roll < drought_chance_) {
            drought_days_left_ = 4;
            message += "🔥 여름 가뭄이 시작되었습니다. 4일 동안 작물 관리가 필요합니다. ";
        }
    }
}
