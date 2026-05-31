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

    bool is_summer(int day) const;
    bool is_fall(int day) const;
    bool is_winter(int day) const;
};
