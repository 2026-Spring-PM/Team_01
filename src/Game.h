#pragma once

#include "Map.h"
#include "Player.h"
#include "Renderer.h"
#include "MarketSystem.h"
#include "KitchenSystem.h"
#include "WorkshopSystem.h"
#include "EventSystem.h"
#include <string>

#ifdef FARM_GUI
#include "GuiRenderer.h"
#endif

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
#ifdef FARM_GUI
    bool gui_mode_ = false;
    GuiRenderer gui_renderer_;
#endif

    void handle_input(char input);
    void handle_action();
    void handle_farm(Tile& tile);
    void handle_livestock(Tile& tile);
    void handle_building(Tile& tile);
    void advance_day();
    void show_inventory();
    void quit();
    void check_game_end();
    bool has_built_facility(const std::string& name) const;
    bool build_storage(Tile& tile, std::string& message);
    bool upgrade_storage(std::string& message);
    void remove_crop_from_tile(Tile& tile, std::string& message);

#ifdef FARM_GUI
    void handle_gui_market();
    void handle_gui_farm(Tile& tile);
    void handle_gui_livestock(Tile& tile);
    void handle_gui_building(Tile& tile);
    void handle_gui_kitchen();
    void handle_gui_workshop();
    void handle_gui_storage();
    void show_gui_inventory();
#endif
};
