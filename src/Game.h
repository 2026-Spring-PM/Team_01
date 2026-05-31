#pragma once

#include "Map.h"
#include "Player.h"
#include "Renderer.h"
#include "MarketSystem.h"
#include "KitchenSystem.h"
#include "WorkshopSystem.h"
#include "EventSystem.h"
#include "BarnSystem.h"
#include <string>
#include <vector>

#ifdef FARM_GUI
#include "GuiRenderer.h"
#endif

enum class GameArea { FARM, BARN };

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
    BarnSystem barn_;
    GameArea current_area_;
    int farm_return_x_;
    int farm_return_y_;
    bool is_running_;
#ifdef FARM_GUI
    bool gui_mode_ = false;
    GuiRenderer gui_renderer_;
#endif

    void handle_input(char input);
    void handle_farm_input(char input);
    void handle_barn_input(char input);
    void handle_action();
    void handle_barn_action();
    void handle_barn_quick_interaction();
    void enter_barn();
    void leave_barn();
    void handle_farm(Tile& tile);
    void handle_livestock(Tile& tile);
    void handle_building(Tile& tile);
    void handle_facility_by_name(const std::string& name);
    bool place_facility_on_current_plot(const std::string& name, std::string& message);
    void sync_facility_level(const std::string& name, int level);
    void handle_bed_interaction();
    void advance_day();
    void show_inventory();
    void quit();
    void check_game_end();
    bool has_built_facility(const std::string& name) const;
    bool build_storage(Tile& tile, std::string& message);
    bool build_barn(std::string& message);
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
    std::vector<std::string> current_event_status_lines() const;
    void sync_gui_event_status();
#endif
};
