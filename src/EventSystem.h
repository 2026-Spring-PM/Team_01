#pragma once

#include "Player.h"
#include "Map.h"
#include <string>
#include <vector>

struct EventDayReport {
    std::string message;
    bool cold_started = false;
    bool drought_started = false;
    bool drought_continued = false;
    bool drought_ended = false;
    bool drought_damage_applied = false;
    bool wildlife_started = false;
    bool wildlife_blocked = false;
    bool starry_night_started = false;
    int drought_days_left = 0;
    std::vector<std::string> popup_titles;
    std::vector<std::string> popup_messages;
};

class EventSystem {
public:
    EventSystem();

    EventDayReport process_new_day(Player& player, Map& map);
    void process_new_day(Player& player, Map& map, std::string& message);
    bool is_drought_active() const { return drought_days_left_ > 0; }
    int drought_days_left() const { return drought_days_left_; }

private:
    int cold_chance_;
    int drought_chance_;
    int drought_days_left_;

    bool is_summer(int day) const;
};
