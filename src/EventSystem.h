#pragma once

#include "Player.h"
#include "Map.h"
#include <string>

class EventSystem {
public:
    EventSystem();

    void process_new_day(Player& player, Map& map, std::string& message);
    bool is_drought_active() const { return drought_days_left_ > 0; }

private:
    int drought_days_left_;
    bool drought_recently_;   // 가뭄 발생 후 1% 보정 플래그
    bool wildlife_recently_;  // 야생동물 발생 후 1% 보정 플래그

    bool is_summer(int day) const;
    bool is_fall(int day) const;
    bool is_winter(int day) const;
};
