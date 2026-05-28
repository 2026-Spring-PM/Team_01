#pragma once

#include "Tile.h"
#include "Map.h"
#include "Player.h"

#include <string>

class Renderer {
public:
    Renderer() = default;

    void draw_title_screen();
    void draw(const Player& player, const Map& map);
    void draw_inventory_screen(const Player& player);
    void draw_ending_screen(const Player& player);
    void add_message(const std::string& msg);

private:
    std::string last_message_;

    void clear_screen();
    void draw_status_bar(const Player& player);
    void draw_map(const Player& player, const Map& map);
    void draw_menu();
    void draw_message();

    std::string current_zone_name(const Player& player, const Map& map) const;
    std::string tile_to_emoji(TileType type) const;
};
