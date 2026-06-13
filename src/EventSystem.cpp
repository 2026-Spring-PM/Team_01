#include "EventSystem.h"

#include <cstdlib>

EventSystem::EventSystem()
    : cold_chance_(2), drought_chance_(5), drought_days_left_(0) {}

bool EventSystem::is_summer(int day) const {
    return day >= 26 && day <= 50;
}

EventDayReport EventSystem::process_new_day(Player& player, Map& map) {
    EventDayReport report;

    if (drought_days_left_ > 0) {
        map.kill_unwatered_crops_during_drought();
        --drought_days_left_;
        report.drought_damage_applied = true;
        report.drought_days_left = drought_days_left_;

        if (drought_days_left_ > 0) {
            report.drought_continued = true;
            report.message += "가뭄이 계속됩니다. 물을 주지 않은 작물은 말라 죽을 수 있습니다. 남은 일수: "
                           + std::to_string(drought_days_left_) + ". ";
            report.popup_titles.push_back("가뭄 지속");
            report.popup_messages.push_back("가뭄이 계속됩니다.\n밭의 작물에 물을 줘서 보호하세요.");
        } else {
            report.drought_ended = true;
            report.message += "가뭄이 끝났습니다. ";
            report.popup_titles.push_back("가뭄 종료");
            report.popup_messages.push_back("가뭄이 끝났습니다. 작물이 다시 정상적으로 자랍니다.");
        }
    }

    if (std::rand() % 100 < cold_chance_) {
        player.apply_cold_penalty();
        report.cold_started = true;
        report.message += "감기 이벤트: 오늘 최대 스태미나가 감소합니다. ";
        report.popup_titles.push_back("감기 이벤트");
        report.popup_messages.push_back("추운 날씨로 오늘 최대 스태미나가 감소했습니다.\nSTR 2번 스킬은 페널티를 줄입니다.");
    }

    if (is_summer(player.get_day()) && drought_days_left_ == 0 && !report.drought_ended) {
        if (std::rand() % 100 < drought_chance_) {
            drought_days_left_ = 4;
            report.drought_started = true;
            report.drought_days_left = drought_days_left_;
            report.message += "여름 가뭄이 4일 동안 시작됩니다. ";
            report.popup_titles.push_back("가뭄 시작");
            report.popup_messages.push_back("여름 가뭄이 시작되었습니다.\n매일 작물에 물을 줘서 보호하세요.");
        }
    }

    if (std::rand() % 100 < 3) {
        if (player.has_str_node(2)) {
            report.wildlife_blocked = true;
            report.message += "압도적인 기백으로 야생동물 출몰을 막았습니다. ";
        } else {
            player.set_wildlife_today(true);
            report.wildlife_started = true;
            report.message += "오늘 야생동물이 농장 근처에 출몰했습니다. ";
            report.popup_titles.push_back("야생동물 출몰");
            report.popup_messages.push_back("오늘 야생동물이 농장 근처에 있습니다.\nSTR 2번 스킬은 이 이벤트를 무효화합니다.");
        }
    }

    if (player.should_trigger_starry_night()) {
        player.activate_starry_night(1);
        report.starry_night_started = true;
        report.message += "별 내리는 밤의 축복이 오늘 활성화됩니다. ";
        report.popup_titles.push_back("별 내리는 밤");
        report.popup_messages.push_back("희귀한 별 내리는 밤의 축복이 오늘 활성화됩니다.\nLUK 5번 스킬은 발생 확률을 1%에서 10%로 올립니다.");
    }

    report.drought_days_left = drought_days_left_;
    return report;
}

void EventSystem::process_new_day(Player& player, Map& map, std::string& message) {
    EventDayReport report = process_new_day(player, map);
    message = report.message;
}
