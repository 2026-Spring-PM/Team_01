#include "EventSystem.h"

#include <cstdlib>
#include <sstream>

EventSystem::EventSystem()
    : drought_days_left_(0), drought_recently_(false), wildlife_recently_(false) {}

bool EventSystem::is_summer(int day) const { return day >= 26 && day <= 50; }
bool EventSystem::is_fall(int day) const   { return day >= 51 && day <= 75; }
bool EventSystem::is_winter(int day) const { return day >= 76 && day <= 100; }

void EventSystem::process_new_day(Player& player, Map& map, std::string& message) {
    std::ostringstream oss;

    // ── 가뭄 진행 ──
    if (drought_days_left_ > 0) {
        map.kill_unwatered_crops_during_drought();
        --drought_days_left_;
        if (drought_days_left_ == 0) drought_recently_ = true; // 종료 후 보정 시작
        oss << "☀️ 가뭄이 계속되어 물을 주지 않은 작물이 말랐습니다. 남은 기간: "
            << drought_days_left_ << "일. ";
    } else {
        drought_recently_ = false; // 가뭄도 없고 보정도 끝난 상태
    }

    int day = player.get_day();

    // ── 감기 ──
    int cold_chance = is_winter(day) ? 5 : 2;
    if (std::rand() % 100 < cold_chance) {
        player.apply_cold_penalty();
        oss << "🤧 감기에 걸렸습니다! 오늘 최대 스태미나가 제한됩니다. ";
    }

    // ── 여름 가뭄 ── (발생 후 종료 직후 1회는 1% 보정)
    if (is_summer(day) && drought_days_left_ == 0) {
        int chance = drought_recently_ ? 1 : 5;
        if (std::rand() % 100 < chance) {
            drought_days_left_ = 3;
            drought_recently_ = false;
            oss << "🔥 여름 가뭄이 시작되었습니다! 3일 동안 작물에 물을 줘야 합니다. ";
        }
    }

    // ── 야생동물 출몰 (가을 5%, 발생 후 1% 보정) ──
    if (is_fall(day)) {
        int wildlife_chance = wildlife_recently_ ? 1 : 5;
        if (std::rand() % 100 < wildlife_chance) {
            wildlife_recently_ = true;
            player.set_wildlife_today(true);
            if (player.has_str_node(2)) {
                oss << "🐺 야생동물이 출몰했습니다! 하지만 당신의 기백으로 가축들을 보호했습니다. ";
            } else {
                oss << "🐺 야생동물이 출몰했습니다! 오늘 닭과 소의 생산물을 수확할 수 없습니다. ";
            }
        } else {
            wildlife_recently_ = false; // 발생 안 했으면 보정 플래그 리셋
        }
    } else {
        wildlife_recently_ = false;
    }

    // ── 별 내리는 밤 ──
    // 매일 1% (LUK 4단계: 10%)
    int star_chance = player.has_luk_node(5) ? 10 : 1;
    if (std::rand() % 100 < star_chance) {
        player.activate_starry_night(2); // 발생 포함 2일
        // 모든 작물 성장 1일 단축은 Map에서 처리 (advance_day에서 double grow)
        oss << "⭐ 별 내리는 밤이 시작되었습니다! 2일간 모든 작물 성장 1일 단축 + 수확량 최대치! ";
    }

    message = oss.str();
}
