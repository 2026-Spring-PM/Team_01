#pragma once

#include "Map.h"
#include "Player.h"
#include "Tile.h"
#include "ItemType.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <map>
#include <string>
#include <utility>
#include <vector>
#include <set>


struct GuiShopEntry {
    int id;
    ItemType item;
    std::string title;
    std::string subtitle;
    bool disabled = false;
};

struct GuiActionEntry {
    int id;
    std::string title;
    std::string subtitle;
    std::string icon_key;
    bool disabled = false;
};

class GuiRenderer {
public:
    GuiRenderer();
    ~GuiRenderer();

    bool initialize();
    bool is_ready() const { return ready_; }
    bool poll_input(char& input);
    void draw(const Player& player, const Map& map, const std::string& message);

    int show_choice(const std::string& title,
                    const std::string& message,
                    const std::vector<std::pair<int, std::string>>& buttons,
                    int cancel_id = 0);
    bool confirm(const std::string& title, const std::string& message);
    void show_notice(const std::string& title, const std::string& message);
    int choose_quantity(const std::string& title, const std::string& message, int max_quantity, bool allow_all);
    int prompt_quantity(const std::string& title, const std::string& message, int max_quantity, int default_quantity = 1);
    void show_inventory(const Player& player);
    int show_shop_selector(const std::string& title,
                           const std::string& left_title,
                           const std::string& right_title,
                           const std::vector<GuiShopEntry>& entries,
                           const Player& player,
                           const std::string& info_message = "",
                           int cancel_id = 0);
    int show_action_panel(const std::string& title,
                          const std::string& status_text,
                          const std::string& recipe_title,
                          const std::vector<GuiActionEntry>& actions,
                          const std::vector<GuiActionEntry>& recipes,
                          int cancel_id = 0);
    int show_lotto_number_picker(const std::set<int>& chosen, int cancel_id = -1);

    bool play_fishing(Player& player, std::string& message);
    bool play_lotto_mining(Player& player, std::string& message);

private:
    static constexpr int TILE_SIZE = 64;
    static constexpr int PANEL_WIDTH = 300;
    static constexpr int WINDOW_WIDTH = Map::WIDTH * TILE_SIZE + PANEL_WIDTH;
    static constexpr int WINDOW_HEIGHT = Map::HEIGHT * TILE_SIZE;

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    TTF_Font* font_ = nullptr;
    bool ready_ = false;
    bool visual_position_initialized_ = false;
    double visual_x_ = 0.0;
    double visual_y_ = 0.0;
    Uint32 last_frame_ticks_ = 0;
    Uint32 last_move_event_ticks_ = 0;
    std::map<std::string, SDL_Texture*> textures_;

    bool load_texture(const std::string& key, const std::string& path);
    SDL_Texture* texture(const std::string& key) const;
    void draw_texture(const std::string& key, const SDL_Rect& dst);
    std::string item_texture_key(ItemType item) const;
    void draw_tile_base(const Tile& tile, int x, int y);
    void draw_tile_object(const Tile& tile, int x, int y);
    void draw_side_panel(const Player& player, const std::string& message);
    void fill_rect(int x, int y, int w, int h, SDL_Color color);
    void draw_rect_outline(int x, int y, int w, int h, SDL_Color color);
    void draw_text(const std::string& text, int x, int y, SDL_Color color, int max_width = 0);
    std::vector<std::string> wrap_text(const std::string& text, int max_chars_per_line) const;
    bool load_font();
    void draw_bar(int x, int y, int w, int h, double ratio, SDL_Color fill);
    void update_window_title(const Player& player, const std::string& message);
    int fishing_hit_width_for_level(int level) const;
    void draw_fishing_frame(int level, int pos, int hit_start, int hit_end, int bar_width);
};
