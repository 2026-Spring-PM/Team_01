#pragma once

#include <string>

enum class Season {
    SPRING,
    SUMMER,
    FALL,
    WINTER
};

inline Season season_from_day(int day) {
    if (day <= 25) return Season::SPRING;
    if (day <= 50) return Season::SUMMER;
    if (day <= 75) return Season::FALL;
    return Season::WINTER;
}

inline std::string season_name(Season season) {
    switch (season) {
        case Season::SPRING: return "🌸 봄";
        case Season::SUMMER: return "☀️ 여름";
        case Season::FALL:   return "🍂 가을";
        case Season::WINTER: return "❄️ 겨울";
    }
    return "?";
}
