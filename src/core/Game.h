#pragma once

#include "Map.h"
#include "Player.h"
#include "Renderer.h"

// 게임 전체 흐름을 제어하는 메인 클래스
class Game {
public:
    static constexpr int MAX_DAY = 100;

    Game();

    void run();

private:
    Map map_;
    Player player_;
    Renderer renderer_;
    bool is_running_;

    void handle_input(char input);
    void show_inventory();
    void quit();

    // 100일 경과 시 게임 종료
    void check_game_end();
};