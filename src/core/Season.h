#pragma once

#include <string>

// 계절 (게임 디자인 v2: 100일을 25일씩 4계절)
enum class Season {
    SPRING,
    SUMMER,
    FALL,
    WINTER,
};

// 현재 일자로부터 계절 계산
inline Season season_from_day(int day) {
    int idx = ((day - 1) / 25) % 4;
    switch (idx) {
        case 0: return Season::SPRING;
        case 1: return Season::SUMMER;
        case 2: return Season::FALL;
        case 3: return Season::WINTER;
    }
    return Season::SPRING;
}

// 계절 한글 + 이모지
inline std::string season_name(Season s) {
    switch (s) {
        case Season::SPRING: return "🌸 봄";
        case Season::SUMMER: return "☀️ 여름";
        case Season::FALL:   return "🍂 가을";
        case Season::WINTER: return "❄️ 겨울";
    }
    return "?";
}