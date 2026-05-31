#pragma once

#include "Tile.h"
#include <vector>

class Map {
public:
    static constexpr int WIDTH = 12;
    static constexpr int HEIGHT = 12;

    // 침대는 광산 근처에 배치한다.
    static constexpr int BED_X = 10;
    static constexpr int BED_Y = 11;

    // 2x2 상점의 좌상단 좌표. 사용자 표시 좌표 (9,4)를 0-based로 변환한 값이다.
    static constexpr int MARKET_X = 8;
    static constexpr int MARKET_Y = 3;

    // 2x2 건축 부지 3곳의 좌상단 좌표. 축사는 시작 시 고정 설치된다.
    static constexpr int BUILD_PLOT_COUNT = 3;
    static constexpr int BUILD_PLOT_SIZE = 2;

    // 2x2 건축 부지 좌상단 좌표. 오른쪽의 3개 부지는 기존 건물용, 왼쪽 위 부지는 축사용이다.
    static constexpr int BUILD_PLOT_AX0 = 10;
    static constexpr int BUILD_PLOT_AY0 = 0;
    static constexpr int BUILD_PLOT_AX1 = 10;
    static constexpr int BUILD_PLOT_AY1 = 3;
    static constexpr int BUILD_PLOT_AX2 = 10;
    static constexpr int BUILD_PLOT_AY2 = 6;
    // 기본 설치 축사의 좌상단 위치: 기존 메인 맵 사육 공간을 대체한다.
    static constexpr int BARN_X = 3;
    static constexpr int BARN_Y = 0;

    static constexpr int MINE_X = 11;
    static constexpr int MINE_Y = 11;

    Map();

    const Tile& get_tile(int x, int y) const;
    Tile& get_tile(int x, int y);
    bool is_in_bounds(int x, int y) const;

    bool find_build_plot_anchor(int x, int y, int& ax, int& ay) const;
    bool is_build_plot_anchor(int ax, int ay) const;
    bool is_building_door(int x, int y) const;
    const std::string& door_target_at(int x, int y) const;
    bool place_facility_2x2(int ax, int ay, const std::string& name);

    void advance_day();
    void kill_unwatered_crops_during_drought();

private:
    std::vector<std::vector<Tile>> tiles_;
    Tile out_of_bounds_tile_;
};
