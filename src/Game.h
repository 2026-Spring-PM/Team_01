#pragma once

#include "Map.h"
#include "Player.h"
#include "Renderer.h"
#include "MarketSystem.h"
#include "KitchenSystem.h"
#include "WorkshopSystem.h"
#include "EventSystem.h"

class Game {
public:
    static constexpr int MAX_DAY = 100;

    Game();
    void run();

private:
    Map map_;
    Player player_;
    Renderer renderer_;
    MarketSystem market_;
    KitchenSystem kitchen_;
    WorkshopSystem workshop_;
    EventSystem event_system_;
    bool is_running_;

    void handle_input(char input);
    void handle_action();
    void handle_farm(Tile& tile);
    void handle_livestock(Tile& tile);
    void handle_building(Tile& tile);
    void advance_day();
    void show_inventory();
    void quit();
    void check_game_end();
};
