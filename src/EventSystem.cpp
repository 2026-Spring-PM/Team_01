#include "EventSystem.h"

#include <cstdlib>

EventSystem::EventSystem()
    : cold_chance_(2), drought_chance_(5), drought_days_left_(0) {}

bool EventSystem::is_summer(int day) const {
    return day >= 26 && day <= 50;
}

EventDayReport EventSystem::process_new_day(Player& player, Map& map) {
    EventDayReport report;

    // 1) Existing drought effect is processed first at the start of the new day.
    if (drought_days_left_ > 0) {
        map.kill_unwatered_crops_during_drought();
        --drought_days_left_;
        report.drought_damage_applied = true;
        report.drought_days_left = drought_days_left_;

        if (drought_days_left_ > 0) {
            report.drought_continued = true;
            report.message += "☀️ 가뭄 지속 중입니다. 물을 주지 않은 작물이 말랐습니다. 남은 기간: "
                           + std::to_string(drought_days_left_) + "일. ";
            report.popup_titles.push_back("☀ DROUGHT CONTINUES");
            report.popup_messages.push_back("가뭄이 계속되고 있습니다.\n물을 주지 않은 작물이 말라 죽을 수 있습니다.\n남은 기간: "
                                          + std::to_string(drought_days_left_) + "일\n가뭄 중에는 농지에서 물 주기 옵션이 활성화됩니다.");
        } else {
            report.drought_ended = true;
            report.message += "🌧️ 가뭄이 끝났습니다. 이제 작물이 정상적으로 성장합니다. ";
            report.popup_titles.push_back("🌧 DROUGHT ENDED");
            report.popup_messages.push_back("가뭄이 끝났습니다.\n이제 작물이 정상적으로 성장합니다.\n물 주기 옵션은 다시 비활성화됩니다.");
        }
    }

    // 2) Cold is a one-day event that lowers the maximum stamina for today.
    int cold_roll = std::rand() % 100;
    if (cold_roll < cold_chance_) {
        player.apply_cold_penalty();
        report.cold_started = true;
        report.message += "🤧 감기에 걸려 오늘 최대 스태미나가 절반으로 제한되었습니다. ";
        report.popup_titles.push_back("🤒 COLD EVENT");
        report.popup_messages.push_back("오늘은 감기에 걸렸습니다.\n오늘 하루 최대 스태미나가 절반으로 제한됩니다.\n무리한 행동보다 중요한 작업을 먼저 처리하세요.");
    }

    // 3) A new drought can start in summer only. If drought ended this exact morning,
    // do not immediately start another one on the same day.
    if (is_summer(player.get_day()) && drought_days_left_ == 0 && !report.drought_ended) {
        int drought_roll = std::rand() % 100;
        if (drought_roll < drought_chance_) {
            drought_days_left_ = 4;
            report.drought_started = true;
            report.drought_days_left = drought_days_left_;
            report.message += "🔥 여름 가뭄이 시작되었습니다. 4일 동안 작물에 물을 주어야 합니다. ";
            report.popup_titles.push_back("☀ DROUGHT STARTED");
            report.popup_messages.push_back("여름 가뭄이 시작되었습니다!\n4일 동안 물을 주지 않은 작물이 말라 죽을 수 있습니다.\n농지에서 J를 눌러 물 주기 옵션을 사용하세요.");
        }
    }

    report.drought_days_left = drought_days_left_;
    return report;
}

void EventSystem::process_new_day(Player& player, Map& map, std::string& message) {
    EventDayReport report = process_new_day(player, map);
    message = report.message;
}
