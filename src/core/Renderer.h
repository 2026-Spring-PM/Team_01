#pragma once

#include "Tile.h"
#include "Map.h"
#include "Player.h"

#include <string>

// 화면 출력 담당 클래스
// 모든 std::cout 호출은 이 클래스를 통해서만 함
class Renderer {
public:
    Renderer() = default;
    
    // 시작 화면 (이름 입력 받기 전)
    void draw_title_screen();

    // 전체 화면 한 번에 그리기
    void draw(const Player& player, const Map& map);

    // 인벤토리 화면 표시 (I 키 누르면 호출됨)
    void draw_inventory_screen(const Player& player);

    // 게임 종료 화면 (Day 100 도달 시)
    void draw_ending_screen(const Player& player);

    // 메시지 큐에 메시지 추가
    void add_message(const std::string& msg);

private:
    std::string last_message_;

    void clear_screen();
    void draw_status_bar(const Player& player);
    void draw_map(const Player& player, const Map& map);
    void draw_menu();
    void draw_message();

    // 현재 위치한 구역 이름 반환
    std::string current_zone_name(const Player& player, const Map& map) const;
    std::string tile_to_emoji(TileType type) const;
};