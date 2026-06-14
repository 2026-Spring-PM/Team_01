#include "GuiRenderer.h"
#include "ItemType.h"
#include "Season.h"
#include "Tutorial.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <set>
#include <sstream>
#include <vector>


namespace {
std::string sanitize_gui_text(const std::string& input) {
    std::string out;
    for (std::size_t i = 0; i < input.size();) {
        unsigned char c = static_cast<unsigned char>(input[i]);
        unsigned int cp = 0;
        std::size_t len = 1;
        if (c < 0x80) { cp = c; len = 1; }
        else if ((c & 0xE0) == 0xC0 && i + 1 < input.size()) {
            cp = ((c & 0x1F) << 6) | (static_cast<unsigned char>(input[i + 1]) & 0x3F);
            len = 2;
        } else if ((c & 0xF0) == 0xE0 && i + 2 < input.size()) {
            cp = ((c & 0x0F) << 12) |
                 ((static_cast<unsigned char>(input[i + 1]) & 0x3F) << 6) |
                 (static_cast<unsigned char>(input[i + 2]) & 0x3F);
            len = 3;
        } else if ((c & 0xF8) == 0xF0 && i + 3 < input.size()) {
            cp = ((c & 0x07) << 18) |
                 ((static_cast<unsigned char>(input[i + 1]) & 0x3F) << 12) |
                 ((static_cast<unsigned char>(input[i + 2]) & 0x3F) << 6) |
                 (static_cast<unsigned char>(input[i + 3]) & 0x3F);
            len = 4;
        } else {
            ++i;
            continue;
        }

        const bool emoji_or_symbol =
            cp >= 0x1F000 || cp == 0xFE0F || cp == 0x200D ||
            (cp >= 0x2600 && cp <= 0x27BF);
        if (!emoji_or_symbol) out.append(input, i, len);
        i += len;
    }
    return out;
}
}

GuiRenderer::GuiRenderer() = default;

GuiRenderer::~GuiRenderer() {
    clear_text_cache();
    for (auto& pair : textures_) SDL_DestroyTexture(pair.second);
    textures_.clear();
    if (font_) TTF_CloseFont(font_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void GuiRenderer::clear_text_cache() {
    for (auto& pair : text_cache_) {
        if (pair.second.texture) SDL_DestroyTexture(pair.second.texture);
    }
    text_cache_.clear();
}

bool GuiRenderer::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }
    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << "\n";
        return false;
    }
    int img_flags = IMG_INIT_PNG;
    if ((IMG_Init(img_flags) & img_flags) != img_flags) {
        std::cerr << "IMG_Init failed: " << IMG_GetError() << "\n";
        return false;
    }

    window_ = SDL_CreateWindow("농장 마을",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               WINDOW_WIDTH,
                               WINDOW_HEIGHT,
                               SDL_WINDOW_SHOWN);
    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer_) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        return false;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    if (!load_font()) {
        std::cerr << "Failed to load a GUI font. Install fonts-noto-cjk or provide a supported TTF/TTC font.\n";
    }

    const std::string bases[] = {"assets/sprites/", "assets/"};
    const std::pair<std::string, std::string> entries[] = {
        {"player_front", "player_front.png"}, {"player_back", "player_back.png"},
        {"player_left", "player_left.png"}, {"player_right", "player_right.png"},
        {"water", "tile_water.png"}, {"sand", "tile_sand.png"}, {"livestock_tile", "tile_livestock.png"},
        {"farm_tile", "tile_farm.png"}, {"building_tile", "tile_building.png"}, {"path", "tile_path.png"},
        {"bed", "bed.png"}, {"market", "market_2x2.png"}, {"kitchen", "kitchen_2x2.png"}, {"workshop", "workshop_2x2.png"},
        {"build_plot", "build_plot_2x2.png"}, {"storage", "storage_2x2.png"}, {"barn", "barn_2x2.png"}, {"mine", "mine.png"},
        {"sprout", "crop_sprout.png"}, {"potato", "crop_potato.png"}, {"carrot", "crop_carrot.png"},
        {"wheat", "crop_wheat.png"}, {"rice", "crop_rice.png"}, {"apple_sapling", "crop_apple_sapling.png"},
        {"apple_tree", "crop_apple_tree.png"}, {"apple_empty", "crop_apple_empty.png"}, {"watered", "crop_watered.png"},
        {"dead", "crop_dead.png"},
        {"wood_tree", "tree.png"}, {"bush", "bush.png"},
        {"baby_chick", "baby_chick.png"}, {"chicken", "chicken.png"}, {"baby_pig", "baby_pig.png"},
        {"pig", "pig.png"}, {"baby_cow", "baby_cow.png"}, {"cow", "cow.png"},
        {"baby_chick_front", "baby_chick_front.png"}, {"baby_chick_back", "baby_chick_back.png"},
        {"baby_chick_left", "baby_chick_left.png"}, {"baby_chick_right", "baby_chick_right.png"},
        {"chicken_front", "chicken_front.png"}, {"chicken_back", "chicken_back.png"},
        {"chicken_left", "chicken_left.png"}, {"chicken_right", "chicken_right.png"},
        {"baby_pig_front", "baby_pig_front.png"}, {"baby_pig_back", "baby_pig_back.png"},
        {"baby_pig_left", "baby_pig_left.png"}, {"baby_pig_right", "baby_pig_right.png"},
        {"pig_front", "pig_front.png"}, {"pig_back", "pig_back.png"},
        {"pig_left", "pig_left.png"}, {"pig_right", "pig_right.png"},
        {"baby_cow_front", "baby_cow_front.png"}, {"baby_cow_back", "baby_cow_back.png"},
        {"baby_cow_left", "baby_cow_left.png"}, {"baby_cow_right", "baby_cow_right.png"},
        {"cow_front", "cow_front.png"}, {"cow_back", "cow_back.png"},
        {"cow_left", "cow_left.png"}, {"cow_right", "cow_right.png"},
        {"bag", "icon_bag.png"}, {"coin", "icon_coin.png"}, {"energy", "icon_energy.png"},
        {"sleep", "icon_sleep.png"}, {"xp", "icon_xp.png"}, {"level", "icon_level.png"},
        {"cold", "icon_cold.png"}, {"drought", "icon_drought.png"}, {"water_drop", "icon_water.png"},
        {"hand", "icon_hand.png"}, {"shop_icon", "icon_shop.png"},
        {"fish_mackerel", "fish_mackerel.png"}, {"fish_flatfish", "fish_flatfish.png"}, {"fish_tuna", "fish_tuna.png"},
        {"copper", "copper.png"}, {"iron", "iron.png"}, {"gold", "gold.png"},
        {"wood", "wood.png"},
        {"seed_potato", "seed_potato.png"}, {"seed_carrot", "seed_carrot.png"}, {"seed_wheat", "seed_wheat.png"},
        {"seed_rice", "seed_rice.png"}, {"seed_apple", "seed_apple.png"},
        {"item_potato", "item_potato.png"}, {"item_carrot", "item_carrot.png"}, {"item_wheat", "item_wheat.png"},
        {"item_rice", "item_rice.png"}, {"item_apple", "item_apple.png"},
        {"egg", "egg.png"}, {"milk", "milk.png"}, {"pork", "pork.png"},
        {"food_bread", "food_bread.png"}, {"food_grilled_fish", "food_grilled_fish.png"}, {"food_apple_pie", "food_apple_pie.png"},
        {"food_potato_soup", "food_potato_soup.png"}, {"food_carrot_egg_roll", "food_carrot_egg_roll.png"},
        {"food_mackerel_carrot", "food_mackerel_carrot.png"}, {"food_pork_potato", "food_pork_potato.png"},
        {"food_potato_wheat_pancake", "food_potato_wheat_pancake.png"}, {"food_carrot_flatfish", "food_carrot_flatfish.png"},
        {"food_3color_carrot_rice", "food_3color_carrot_rice.png"}, {"food_mashed_beef", "food_mashed_beef.png"},
        {"food_carrot_apple_juice", "food_carrot_apple_juice.png"}, {"food_tuna_potato", "food_tuna_potato.png"},
        {"food_hailight_rice", "food_hailight_rice.png"}, {"food_apple_potato_cake", "food_apple_potato_cake.png"},
        {"monster_food_low", "monster_food_low.png"}, {"monster_food_mid", "monster_food_mid.png"},
        {"monster_food_high", "monster_food_high.png"},
        {"barn_floor", "barn_floor.png"}, {"barn_wall", "barn_wall.png"},
        {"barn_fence_h", "barn_fence_h.png"}, {"barn_fence_v", "barn_fence_v.png"}, {"barn_gate", "barn_gate.png"},
        {"barn_water_trough", "barn_water_trough.png"}, {"barn_feed_trough", "barn_feed_trough.png"},
        {"barn_hay_bale", "barn_hay_bale.png"}, {"barn_hay_roll", "barn_hay_roll.png"},
        {"barn_hay_pile", "barn_hay_pile.png"}, {"barn_hay_low", "barn_hay_low.png"},
        {"barn_barrel", "barn_barrel.png"}, {"barn_sack", "barn_sack.png"},
        {"barn_crate", "barn_crate.png"}, {"barn_bucket", "barn_bucket.png"},
        {"barn_wall_panel", "barn_wall_panel.png"},
        {"npc_seed", "npc_seed.png"}
    };

    for (const auto& e : entries) {
        bool loaded = false;
        for (const std::string& base : bases) {
            if (load_texture(e.first, base + e.second)) {
                loaded = true;
                break;
            }
        }
        if (!loaded) {
            std::cerr << "Failed to load GUI texture: " << e.second << " from assets/sprites/ or assets/\n";
            return false;
        }
    }

    ready_ = true;
    return true;
}

bool GuiRenderer::load_texture(const std::string& key, const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) return false;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_FreeSurface(surface);
    if (!tex) return false;
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    textures_[key] = tex;
    return true;
}

SDL_Texture* GuiRenderer::texture(const std::string& key) const {
    auto it = textures_.find(key);
    return it == textures_.end() ? nullptr : it->second;
}

void GuiRenderer::draw_texture(const std::string& key, const SDL_Rect& dst) {
    SDL_Texture* tex = texture(key);
    if (tex) SDL_RenderCopy(renderer_, tex, nullptr, &dst);
}

std::string GuiRenderer::item_texture_key(ItemType item) const {
    switch (item) {
        case ItemType::SEED_POTATO: return "seed_potato";
        case ItemType::SEED_CARROT: return "seed_carrot";
        case ItemType::SEED_WHEAT: return "seed_wheat";
        case ItemType::SEED_RICE: return "seed_rice";
        case ItemType::SEED_APPLE: return "seed_apple";
        case ItemType::POTATO: return "item_potato";
        case ItemType::CARROT: return "item_carrot";
        case ItemType::WHEAT: return "item_wheat";
        case ItemType::RICE: return "item_rice";
        case ItemType::APPLE: return "item_apple";
        case ItemType::BABY_CHICKEN: return "baby_chick";
        case ItemType::BABY_PIG: return "baby_pig";
        case ItemType::BABY_COW: return "baby_cow";
        case ItemType::EGG: return "egg";
        case ItemType::MILK: return "milk";
        case ItemType::PORK: return "pork";
        case ItemType::WOOD: return "wood";
        case ItemType::COPPER: return "copper";
        case ItemType::IRON: return "iron";
        case ItemType::GOLD: return "gold";
        case ItemType::FISH_MACKEREL: return "fish_mackerel";
        case ItemType::FISH_FLATFISH: return "fish_flatfish";
        case ItemType::FISH_TUNA: return "fish_tuna";
        case ItemType::HOE_COPPER:
        case ItemType::HOE_IRON:
        case ItemType::HOE_GOLD:
        case ItemType::ROD_COPPER:
        case ItemType::ROD_IRON:
        case ItemType::ROD_GOLD:
        case ItemType::PICK_COPPER:
        case ItemType::PICK_IRON:
        case ItemType::PICK_GOLD:
            return "workshop";
        case ItemType::FOOD_POTATO_SOUP: return "food_potato_soup";
        case ItemType::FOOD_CARROT_EGG_ROLL: return "food_carrot_egg_roll";
        case ItemType::FOOD_MACKEREL_CARROT: return "food_mackerel_carrot";
        case ItemType::FOOD_PORK_POTATO: return "food_pork_potato";
        case ItemType::FOOD_POTATO_WHEAT_PANCAKE: return "food_potato_wheat_pancake";
        case ItemType::FOOD_CARROT_FLATFISH: return "food_carrot_flatfish";
        case ItemType::FOOD_3COLOR_CARROT_RICE: return "food_3color_carrot_rice";
        case ItemType::FOOD_MASHED_BEEF: return "food_mashed_beef";
        case ItemType::FOOD_CARROT_APPLE_JUICE: return "food_carrot_apple_juice";
        case ItemType::FOOD_TUNA_POTATO: return "food_tuna_potato";
        case ItemType::FOOD_HAILIGHT_RICE: return "food_hailight_rice";
        case ItemType::FOOD_APPLE_POTATO_CAKE: return "food_apple_potato_cake";
        case ItemType::MONSTER_FOOD_LOW: return "monster_food_low";
        case ItemType::MONSTER_FOOD_MID: return "monster_food_mid";
        case ItemType::MONSTER_FOOD_HIGH: return "monster_food_high";
    }
    return "bag";
}

int GuiRenderer::show_choice(const std::string& title,
                             const std::string& message,
                             const std::vector<std::pair<int, std::string>>& buttons,
                             int cancel_id) {
    if (buttons.empty()) return cancel_id;
    if (!ready_ || !window_ || !renderer_) return cancel_id;

    SDL_SetWindowTitle(window_, title.c_str());

    const int panel_w = 720;
    const int panel_h = 520;
    const int panel_x = (WINDOW_WIDTH - panel_w) / 2;
    const int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
    const int margin = 26;
    const int btn_gap = 10;
    const int columns = buttons.size() > 8 ? 3 : (buttons.size() > 4 ? 2 : 1);
    const int btn_w = (panel_w - margin * 2 - btn_gap * (columns - 1)) / columns;
    const int btn_h = 48;
    const int button_start_y = panel_y + 250;

    auto draw_dialog = [&]() {
        fill_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_Color{0, 0, 0, 105});
        fill_rect(panel_x, panel_y, panel_w, panel_h, SDL_Color{44, 34, 26, 245});
        draw_rect_outline(panel_x, panel_y, panel_w, panel_h, SDL_Color{224, 184, 97, 255});
        draw_rect_outline(panel_x + 4, panel_y + 4, panel_w - 8, panel_h - 8, SDL_Color{96, 70, 40, 255});

        draw_text(title, panel_x + margin, panel_y + 22, SDL_Color{255, 221, 132, 255}, panel_w - margin * 2);
        int y = panel_y + 68;
        for (const auto& line : wrap_text(message, 48)) {
            draw_text(line, panel_x + margin + 180, y, SDL_Color{238, 230, 207, 255}, panel_w - margin * 2);
            y += 24;
            if (y > panel_y + 220) break;
        }

        for (std::size_t i = 0; i < buttons.size(); ++i) {
            int col = static_cast<int>(i) % columns;
            int row = static_cast<int>(i) / columns;
            int x = panel_x + margin + col * (btn_w + btn_gap);
            int by = button_start_y + row * (btn_h + btn_gap);
            if (by + btn_h > panel_y + panel_h - margin) continue;
            SDL_Color base = buttons[i].first == cancel_id ? SDL_Color{92, 70, 62, 255} : SDL_Color{112, 84, 44, 255};
            fill_rect(x, by, btn_w, btn_h, base);
            draw_rect_outline(x, by, btn_w, btn_h, SDL_Color{232, 190, 95, 255});
            std::ostringstream label;
            if (buttons[i].first == cancel_id) label << "0. ";
            else if (i < 9) label << (i + 1) << ". ";
            else label << "· ";
            label << buttons[i].second;
            draw_text(label.str(), x + 14, by + 13, SDL_Color{255, 245, 213, 255}, btn_w - 28);
        }
        SDL_RenderPresent(renderer_);
    };

    while (true) {
        draw_dialog();
        SDL_Event event;
        while (SDL_WaitEvent(&event)) {
            if (event.type == SDL_QUIT) return cancel_id;
            if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;
                SDL_Scancode sc = event.key.keysym.scancode;
                if (key == SDLK_ESCAPE || sc == SDL_SCANCODE_ESCAPE) return cancel_id;
                if ((key == SDLK_RETURN || sc == SDL_SCANCODE_RETURN || sc == SDL_SCANCODE_SPACE) && !buttons.empty()) return buttons.front().first;
                if (key == SDLK_0 || sc == SDL_SCANCODE_0) return cancel_id;
                int idx = -1;
                if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_9) idx = sc - SDL_SCANCODE_1;
                else if (key >= SDLK_1 && key <= SDLK_9) idx = key - SDLK_1;
                if (idx >= 0 && idx < static_cast<int>(buttons.size())) return buttons[static_cast<std::size_t>(idx)].first;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x;
                int my = event.button.y;
                for (std::size_t i = 0; i < buttons.size(); ++i) {
                    int col = static_cast<int>(i) % columns;
                    int row = static_cast<int>(i) / columns;
                    int x = panel_x + margin + col * (btn_w + btn_gap);
                    int by = button_start_y + row * (btn_h + btn_gap);
                    SDL_Rect r{x, by, btn_w, btn_h};
                    SDL_Point pt{mx, my};
                    if (SDL_PointInRect(&pt, &r)) return buttons[i].first;
                }
            }
            break;
        }
    }
}

bool GuiRenderer::confirm(const std::string& title, const std::string& message) {
    return show_choice(title, message, {{1, "예"}, {0, "아니오"}}, 0) == 1;
}

void GuiRenderer::show_notice(const std::string& title, const std::string& message) {
    show_choice(title, message, {{0, "확인"}}, 0);
}

void GuiRenderer::show_event_popup(const std::string& title, const std::string& message) {
    const int panel_w = 760;
    const int panel_h = 430;
    const int panel_x = (WINDOW_WIDTH - panel_w) / 2;
    const int panel_y = (WINDOW_HEIGHT - panel_h) / 2;

    auto draw_dialog = [&]() {
        fill_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_Color{0, 0, 0, 170});
        fill_rect(panel_x, panel_y, panel_w, panel_h, SDL_Color{44, 32, 22, 250});
        draw_rect_outline(panel_x, panel_y, panel_w, panel_h, SDL_Color{255, 210, 96, 255});
        draw_rect_outline(panel_x + 8, panel_y + 8, panel_w - 16, panel_h - 16, SDL_Color{148, 103, 49, 255});

        draw_text("랜덤 이벤트", panel_x + 36, panel_y + 28, SDL_Color{255, 221, 132, 255}, panel_w - 72);
        draw_text(title, panel_x + 36, panel_y + 82, SDL_Color{255, 245, 196, 255}, panel_w - 72);

        int y = panel_y + 158;
        for (const auto& line : wrap_text(message, 48)) {
            draw_text(line, panel_x + 48, y, SDL_Color{238, 230, 207, 255}, panel_w - 96);
            y += 30;
        }

        const int btn_w = 180;
        const int btn_h = 54;
        const int btn_x = panel_x + (panel_w - btn_w) / 2;
        const int btn_y = panel_y + panel_h - 82;
        fill_rect(btn_x, btn_y, btn_w, btn_h, SDL_Color{124, 84, 40, 255});
        draw_rect_outline(btn_x, btn_y, btn_w, btn_h, SDL_Color{255, 210, 96, 255});
        draw_text("확인", btn_x + 66, btn_y + 16, SDL_Color{255, 245, 213, 255}, btn_w - 24);
        SDL_RenderPresent(renderer_);
    };

    draw_dialog();
    SDL_Event event;
    while (SDL_WaitEvent(&event)) {
        if (event.type == SDL_QUIT) return;
        if (event.type == SDL_KEYDOWN &&
            (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE)) return;
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) return;
    }
}

int GuiRenderer::choose_quantity(const std::string& title, const std::string& message, int max_quantity, bool allow_all) {
    if (max_quantity <= 0) return 0;
    if (allow_all) {
        std::vector<std::pair<int, std::string>> buttons;
        buttons.push_back({-2, "직접 입력"});
        buttons.push_back({1, "1개"});
        if (max_quantity >= 5) buttons.push_back({5, "5개"});
        if (max_quantity >= 10) buttons.push_back({10, "10개"});
        if (max_quantity > 1) buttons.push_back({max_quantity, "전부"});
        buttons.push_back({0, "취소"});
        int q = show_choice(title, message, buttons, 0);
        if (q == -2) return prompt_quantity(title, message, max_quantity, std::min(1, max_quantity));
        if (q < 0) return 0;
        return std::min(q, max_quantity);
    }
    return prompt_quantity(title, message, max_quantity, std::min(1, max_quantity));
}

int GuiRenderer::prompt_quantity(const std::string& title, const std::string& message, int max_quantity, int default_quantity) {
    if (max_quantity <= 0) return 0;
    std::string input;
    const int panel_w = 680;
    const int panel_h = 360;
    const int panel_x = (WINDOW_WIDTH - panel_w) / 2;
    const int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
    const int field_x = panel_x + 50;
    const int field_y = panel_y + 190;
    const int field_w = panel_w - 100;
    const int field_h = 54;
    SDL_StartTextInput();

    auto draw_dialog = [&]() {
        fill_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_Color{0, 0, 0, 115});
        fill_rect(panel_x, panel_y, panel_w, panel_h, SDL_Color{44, 34, 26, 248});
        draw_rect_outline(panel_x, panel_y, panel_w, panel_h, SDL_Color{224, 184, 97, 255});
        draw_text(title, panel_x + 32, panel_y + 24, SDL_Color{255, 221, 132, 255}, panel_w - 64);
        int y = panel_y + 72;
        for (const auto& line : wrap_text(message, 52)) {
            draw_text(line, panel_x + 32, y, SDL_Color{238, 230, 207, 255}, panel_w - 64);
            y += 24;
        }
        fill_rect(field_x, field_y, field_w, field_h, SDL_Color{29, 24, 18, 255});
        draw_rect_outline(field_x, field_y, field_w, field_h, SDL_Color{232, 190, 95, 255});
        draw_text(input.empty() ? ("입력 대기 (Enter=" + std::to_string(std::max(1, std::min(default_quantity, max_quantity))) + ")") : input, field_x + 18, field_y + 14, SDL_Color{255, 245, 213, 255}, field_w - 36);
        fill_rect(panel_x + 50, panel_y + 270, 170, 48, SDL_Color{112, 84, 44, 255});
        fill_rect(panel_x + 250, panel_y + 270, 170, 48, SDL_Color{112, 84, 44, 255});
        fill_rect(panel_x + 450, panel_y + 270, 170, 48, SDL_Color{92, 70, 62, 255});
        draw_rect_outline(panel_x + 50, panel_y + 270, 170, 48, SDL_Color{232, 190, 95, 255});
        draw_rect_outline(panel_x + 250, panel_y + 270, 170, 48, SDL_Color{232, 190, 95, 255});
        draw_rect_outline(panel_x + 450, panel_y + 270, 170, 48, SDL_Color{232, 190, 95, 255});
        draw_text("Enter 확인", panel_x + 75, panel_y + 283, SDL_Color{255, 245, 213, 255});
        draw_text("전부", panel_x + 310, panel_y + 283, SDL_Color{255, 245, 213, 255});
        draw_text("Esc 취소", panel_x + 487, panel_y + 283, SDL_Color{255, 245, 213, 255});
        SDL_RenderPresent(renderer_);
    };

    while (true) {
        draw_dialog();
        SDL_Event event;
        while (SDL_WaitEvent(&event)) {
            if (event.type == SDL_QUIT) { SDL_StopTextInput(); return 0; }
            if (event.type == SDL_TEXTINPUT) {
                for (const char* c = event.text.text; *c; ++c) {
                    if (*c >= '0' && *c <= '9' && input.size() < 7) input.push_back(*c);
                }
                break;
            }
            if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_ESCAPE) { SDL_StopTextInput(); return 0; }
                if (key == SDLK_BACKSPACE && !input.empty()) { input.pop_back(); break; }
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    int qty = input.empty() ? std::max(1, std::min(default_quantity, max_quantity)) : std::atoi(input.c_str());
                    SDL_StopTextInput();
                    return std::max(0, std::min(qty, max_quantity));
                }
                // 숫자 입력은 SDL_TEXTINPUT에서만 처리한다.
                // KEYDOWN에서도 숫자를 추가하면 일부 환경에서 1회 입력이 2회 입력되는 문제가 생긴다.
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                SDL_Point pt{event.button.x, event.button.y};
                SDL_Rect ok{panel_x + 50, panel_y + 270, 170, 48};
                SDL_Rect all{panel_x + 250, panel_y + 270, 170, 48};
                SDL_Rect cancel{panel_x + 450, panel_y + 270, 170, 48};
                if (SDL_PointInRect(&pt, &ok)) {
                    int qty = input.empty() ? std::max(1, std::min(default_quantity, max_quantity)) : std::atoi(input.c_str());
                    SDL_StopTextInput();
                    return std::max(0, std::min(qty, max_quantity));
                }
                if (SDL_PointInRect(&pt, &all)) { SDL_StopTextInput(); return max_quantity; }
                if (SDL_PointInRect(&pt, &cancel)) { SDL_StopTextInput(); return 0; }
            }
            break;
        }
    }
}

void GuiRenderer::show_inventory(const Player& player) {
    const int panel_w = 760;
    const int panel_h = 620;
    const int panel_x = (WINDOW_WIDTH - panel_w) / 2;
    const int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
    const int margin = 28;
    auto items = player.inventory().get_all();

    auto draw_dialog = [&]() {
        fill_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_Color{0, 0, 0, 115});
        fill_rect(panel_x, panel_y, panel_w, panel_h, SDL_Color{44, 34, 26, 248});
        draw_rect_outline(panel_x, panel_y, panel_w, panel_h, SDL_Color{224, 184, 97, 255});
        draw_text("인벤토리", panel_x + margin, panel_y + 22, SDL_Color{255, 221, 132, 255}, panel_w - margin * 2);
        std::ostringstream header;
        header << "골드 " << player.get_money() << "G  |  스태미나 " << player.get_stamina() << "/" << player.get_stamina_max()
               << "  |  레벨 " << player.get_level() << "  |  " << player.get_day() << "일차"
               << "  |  인벤토리 " << player.inventory().used_slots() << "/" << player.inventory().max_slots();
        draw_text(header.str(), panel_x + margin, panel_y + 60, SDL_Color{238, 230, 207, 255}, panel_w - margin * 2);

        if (items.empty()) {
            draw_text("가방이 비어 있습니다.", panel_x + margin, panel_y + 125, SDL_Color{238, 230, 207, 255});
        } else {
            const int row_h = 48;
            const int col_w = (panel_w - margin * 2) / 2;
            for (std::size_t i = 0; i < items.size() && i < 20; ++i) {
                int col = static_cast<int>(i) / 10;
                int row = static_cast<int>(i) % 10;
                int x = panel_x + margin + col * col_w;
                int y = panel_y + 110 + row * row_h;
                fill_rect(x, y, col_w - 12, row_h - 6, SDL_Color{61, 48, 35, 210});
                draw_rect_outline(x, y, col_w - 12, row_h - 6, SDL_Color{96, 70, 40, 255});
                SDL_Rect icon{x + 6, y + 4, 36, 36};
                draw_texture(item_texture_key(items[i].first), icon);
                std::ostringstream line;
                line << item_name(items[i].first) << "  x " << items[i].second;
                draw_text(line.str(), x + 52, y + 12, SDL_Color{255, 245, 213, 255}, col_w - 72);
            }
            if (items.size() > 20) draw_text("...더 많은 아이템은 다음 버전에서 페이지로 표시됩니다.", panel_x + margin, panel_y + panel_h - 88, SDL_Color{238, 230, 207, 255});
        }
        fill_rect(panel_x + panel_w - 170, panel_y + panel_h - 68, 130, 42, SDL_Color{92, 70, 62, 255});
        draw_rect_outline(panel_x + panel_w - 170, panel_y + panel_h - 68, 130, 42, SDL_Color{232, 190, 95, 255});
        draw_text("닫기", panel_x + panel_w - 124, panel_y + panel_h - 56, SDL_Color{255, 245, 213, 255});
        SDL_RenderPresent(renderer_);
    };

    while (true) {
        draw_dialog();
        SDL_Event event;
        while (SDL_WaitEvent(&event)) {
            if (event.type == SDL_QUIT) return;
            if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE)) return;
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                SDL_Point pt{event.button.x, event.button.y};
                SDL_Rect close{panel_x + panel_w - 170, panel_y + panel_h - 68, 130, 42};
                if (SDL_PointInRect(&pt, &close)) return;
            }
            break;
        }
    }
}

void GuiRenderer::run_tutorial(const Player& player, const Map& map, Tutorial& tutorial) {
    if (!ready_ || !window_ || !renderer_) return;

    const int panel_w = 820;
    const int panel_h = 430;
    const int panel_x = (WINDOW_WIDTH - panel_w) / 2;
    const int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
    const int margin = 34;
    const SDL_Rect next_rect{panel_x + panel_w - 210, panel_y + panel_h - 72, 160, 46};
    const SDL_Rect skip_rect{panel_x + 50, panel_y + panel_h - 72, 160, 46};

    auto draw_dialog = [&]() {
        draw(player, map, "튜토리얼 진행 중...");
        fill_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_Color{0, 0, 0, 145});
        fill_rect(panel_x, panel_y, panel_w, panel_h, SDL_Color{44, 34, 26, 250});
        draw_rect_outline(panel_x, panel_y, panel_w, panel_h, SDL_Color{224, 184, 97, 255});
        draw_rect_outline(panel_x + 6, panel_y + 6, panel_w - 12, panel_h - 12, SDL_Color{96, 70, 40, 255});

        std::ostringstream header;
        header << Tutorial::npc_name() << " - " << tutorial.current_title();
        draw_text(header.str(), panel_x + margin + 180, panel_y + 28, SDL_Color{255, 221, 132, 255}, panel_w - margin * 2 - 180);

        if (texture("npc_seed")) {
            SDL_Rect npc_rect{panel_x + 36, panel_y + 40, 136, 170}; 
            fill_rect(npc_rect.x, npc_rect.y, npc_rect.w, npc_rect.h, SDL_Color{30, 22, 16, 200});
            draw_rect_outline(npc_rect.x, npc_rect.y, npc_rect.w, npc_rect.h, SDL_Color{148, 103, 49, 255});
            draw_texture("npc_seed", npc_rect);
        }

        int y = panel_y + 86;
        for (const auto& line : wrap_text(tutorial.current_dialogue(), 48)) {
            draw_text(line, panel_x + margin + 180, y, SDL_Color{238, 230, 207, 255}, panel_w - margin * 2);
            y += 24;
            if (y > panel_y + panel_h - 116) break;
        }

        std::ostringstream progress;
        progress << (tutorial.current_step() + 1) << " / " << tutorial.total_steps();
        draw_text(progress.str(), panel_x + margin + 180, panel_y + panel_h - 116, SDL_Color{214, 200, 170, 255}, panel_w - margin * 2);

        fill_rect(skip_rect.x, skip_rect.y, skip_rect.w, skip_rect.h, SDL_Color{92, 70, 62, 255});
        draw_rect_outline(skip_rect.x, skip_rect.y, skip_rect.w, skip_rect.h, SDL_Color{232, 190, 95, 255});
        draw_text("건너뛰기", skip_rect.x + 38, skip_rect.y + 13, SDL_Color{255, 245, 213, 255}, skip_rect.w - 76);

        fill_rect(next_rect.x, next_rect.y, next_rect.w, next_rect.h, SDL_Color{112, 84, 44, 255});
        draw_rect_outline(next_rect.x, next_rect.y, next_rect.w, next_rect.h, SDL_Color{232, 190, 95, 255});
        const bool last = tutorial.current_step() + 1 >= tutorial.total_steps();
        draw_text(last ? "시작하기" : "다음", next_rect.x + (last ? 42 : 62), next_rect.y + 13, SDL_Color{255, 245, 213, 255}, next_rect.w - 40);
        SDL_RenderPresent(renderer_);
    };

    while (tutorial.is_active()) {
        draw_dialog();
        SDL_Event event;
        while (SDL_WaitEvent(&event)) {
            if (event.type == SDL_QUIT) {
                while (tutorial.is_active()) tutorial.advance();
                return;
            }
            if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;
                SDL_Scancode sc = event.key.keysym.scancode;
                if (key == SDLK_ESCAPE) {
                    while (tutorial.is_active()) tutorial.advance();
                    return;
                }
                if (key == SDLK_RETURN || sc == SDL_SCANCODE_RETURN || sc == SDL_SCANCODE_SPACE || sc == SDL_SCANCODE_KP_ENTER) {
                    tutorial.advance();
                    break;
                }
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                SDL_Point pt{event.button.x, event.button.y};
                if (SDL_PointInRect(&pt, &skip_rect)) {
                    while (tutorial.is_active()) tutorial.advance();
                    return;
                }
                if (SDL_PointInRect(&pt, &next_rect)) {
                    tutorial.advance();
                    break;
                }
            }
            break;
        }
    }
}

int GuiRenderer::show_shop_selector(const std::string& title,
                                    const std::string& left_title,
                                    const std::string& right_title,
                                    const std::vector<GuiShopEntry>& entries,
                                    const Player& player,
                                    const std::string& info_message,
                                    int cancel_id) {
    if (!ready_ || !window_ || !renderer_) return cancel_id;

    const int panel_w = 1020;
    const int panel_h = 690;
    const int panel_x = (WINDOW_WIDTH - panel_w) / 2;
    const int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
    const int margin = 24;
    const int left_w = 390;
    const int gap = 22;
    const int right_w = panel_w - margin * 2 - left_w - gap;
    const int content_y = panel_y + 118;
    const int content_h = panel_h - 190;
    const int close_w = 150;
    const int close_h = 46;
    const SDL_Rect close_rect{panel_x + panel_w - margin - close_w, panel_y + panel_h - margin - close_h, close_w, close_h};

    const int row_h = 70;
    const int max_scroll = std::max(0, static_cast<int>(entries.size()) * row_h - content_h);
    int scroll_y = 0;
    int selected_index = entries.empty() ? -1 : 0;
    bool dragging_scrollbar = false;
    int drag_anchor_y = 0;
    int drag_anchor_scroll = 0;

    auto draw_dialog = [&]() {
        fill_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_Color{0, 0, 0, 120});
        fill_rect(panel_x, panel_y, panel_w, panel_h, SDL_Color{44, 34, 26, 250});
        draw_rect_outline(panel_x, panel_y, panel_w, panel_h, SDL_Color{224, 184, 97, 255});
        draw_rect_outline(panel_x + 4, panel_y + 4, panel_w - 8, panel_h - 8, SDL_Color{96, 70, 40, 255});

        draw_text(title, panel_x + margin, panel_y + 20, SDL_Color{255, 221, 132, 255}, panel_w - margin * 2);
        std::ostringstream summary;
        summary << "골드 " << player.get_money() << "G  |  스태미나 " << player.get_stamina() << "/" << player.get_stamina_max()
                << "  |  레벨 " << player.get_level() << "  |  " << player.get_day() << "일차";
        draw_text(summary.str(), panel_x + margin, panel_y + 55, SDL_Color{238, 230, 207, 255}, panel_w - margin * 2);
        if (!info_message.empty()) draw_text(info_message, panel_x + margin, panel_y + 84, SDL_Color{214, 200, 170, 255}, panel_w - margin * 2);

        const int left_x = panel_x + margin;
        const int right_x = left_x + left_w + gap;
        fill_rect(left_x, content_y - 38, left_w, content_h + 38, SDL_Color{31, 26, 20, 230});
        fill_rect(right_x, content_y - 38, right_w, content_h + 38, SDL_Color{31, 26, 20, 230});
        draw_rect_outline(left_x, content_y - 38, left_w, content_h + 38, SDL_Color{128, 93, 48, 255});
        draw_rect_outline(right_x, content_y - 38, right_w, content_h + 38, SDL_Color{128, 93, 48, 255});
        draw_text(left_title, left_x + 16, content_y - 30, SDL_Color{255, 221, 132, 255}, left_w - 32);
        draw_text(right_title, right_x + 16, content_y - 30, SDL_Color{255, 221, 132, 255}, right_w - 32);

        auto inv = player.inventory().get_all();
        if (inv.empty()) {
            draw_text("보유 품목이 없습니다.", left_x + 16, content_y + 16, SDL_Color{238, 230, 207, 255}, left_w - 32);
        } else {
            const int inv_row_h = 42;
            for (std::size_t i = 0; i < inv.size() && i < 12; ++i) {
                int y = content_y + static_cast<int>(i) * inv_row_h;
                fill_rect(left_x + 12, y, left_w - 24, inv_row_h - 6, SDL_Color{61, 48, 35, 210});
                SDL_Rect icon{left_x + 18, y + 3, 32, 32};
                draw_texture(item_texture_key(inv[i].first), icon);
                std::ostringstream line;
                line << item_name(inv[i].first) << " x" << inv[i].second;
                draw_text(line.str(), left_x + 58, y + 8, SDL_Color{255, 245, 213, 255}, left_w - 76);
            }
            if (inv.size() > 12) draw_text("...", left_x + 18, content_y + 12 * inv_row_h + 6, SDL_Color{238, 230, 207, 255});
        }

        SDL_Rect clip{right_x + 1, content_y, right_w - 2, content_h};
        SDL_RenderSetClipRect(renderer_, &clip);
        for (std::size_t i = 0; i < entries.size(); ++i) {
            int y = content_y + static_cast<int>(i) * row_h - scroll_y;
            if (y + row_h < content_y || y > content_y + content_h) continue;
            const bool selected = static_cast<int>(i) == selected_index;
            SDL_Color base = entries[i].disabled ? SDL_Color{65, 58, 50, 220} : SDL_Color{71, 52, 34, 235};
            if (selected) base = entries[i].disabled ? SDL_Color{88, 78, 63, 235} : SDL_Color{118, 78, 38, 245};
            fill_rect(right_x + 12, y + 5, right_w - 42, row_h - 10, base);
            draw_rect_outline(right_x + 12, y + 5, right_w - 42, row_h - 10,
                              selected ? SDL_Color{255, 230, 120, 255} : (entries[i].disabled ? SDL_Color{105, 95, 82, 255} : SDL_Color{232, 190, 95, 255}));
            SDL_Rect icon{right_x + 24, y + 14, 44, 44};
            draw_texture(item_texture_key(entries[i].item), icon);
            draw_text(entries[i].title, right_x + 82, y + 14, entries[i].disabled ? SDL_Color{180, 170, 150, 255} : SDL_Color{255, 245, 213, 255}, right_w - 130);
            draw_text(entries[i].subtitle, right_x + 82, y + 40, SDL_Color{214, 200, 170, 255}, right_w - 130);
            if (entries[i].disabled) {
                const int bx = right_x + 12;
                const int by = y + 5;
                const int bw = right_w - 42;
                const int bh = row_h - 10;

                // 잠금 항목은 복잡한 대각선 패턴/문자 대신 반투명 오버레이와
                // 벡터 자물쇠 아이콘만 그린다. 일부 SDL_ttf/폰트 환경에서 LOCK 텍스트와
                // 촘촘한 선 패턴이 깨져 보이던 문제를 피하기 위한 처리다.
                fill_rect(bx, by, bw, bh, SDL_Color{0, 0, 0, 105});
                fill_rect(bx, by, 6, bh, SDL_Color{118, 118, 118, 220});

                const int lx = bx + bw - 54;
                const int ly = by + 15;
                SDL_SetRenderDrawColor(renderer_, 220, 220, 210, 245);
                SDL_Rect shackle_outer{lx + 10, ly, 24, 22};
                SDL_Rect shackle_inner{lx + 15, ly + 6, 14, 17};
                SDL_RenderDrawRect(renderer_, &shackle_outer);
                SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 105);
                SDL_RenderDrawRect(renderer_, &shackle_inner);
                fill_rect(lx + 4, ly + 18, 36, 28, SDL_Color{68, 68, 68, 235});
                draw_rect_outline(lx + 4, ly + 18, 36, 28, SDL_Color{220, 220, 210, 245});
                fill_rect(lx + 20, ly + 29, 5, 9, SDL_Color{220, 220, 210, 245});
            }
        }
        SDL_RenderSetClipRect(renderer_, nullptr);

        if (max_scroll > 0) {
            const int track_x = right_x + right_w - 22;
            const int track_y = content_y;
            const int track_h = content_h;
            fill_rect(track_x, track_y, 10, track_h, SDL_Color{24, 20, 16, 255});
            int thumb_h = std::max(42, content_h * content_h / std::max(content_h, static_cast<int>(entries.size()) * row_h));
            int thumb_y = track_y + (track_h - thumb_h) * scroll_y / max_scroll;
            fill_rect(track_x, thumb_y, 10, thumb_h, SDL_Color{210, 170, 85, 255});
        }

        fill_rect(close_rect.x, close_rect.y, close_rect.w, close_rect.h, SDL_Color{92, 70, 62, 255});
        draw_rect_outline(close_rect.x, close_rect.y, close_rect.w, close_rect.h, SDL_Color{232, 190, 95, 255});
        draw_text("나가기", close_rect.x + 44, close_rect.y + 13, SDL_Color{255, 245, 213, 255});
        SDL_RenderPresent(renderer_);
    };

    while (true) {
        draw_dialog();
        SDL_Event event;
        while (SDL_WaitEvent(&event)) {
            if (event.type == SDL_QUIT) return cancel_id;
            if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;
                SDL_Scancode scancode = event.key.keysym.scancode;
                if (key == SDLK_ESCAPE || key == SDLK_0 || scancode == SDL_SCANCODE_0) return cancel_id;
                if ((key == SDLK_RETURN || key == SDLK_KP_ENTER || scancode == SDL_SCANCODE_SPACE) && selected_index >= 0 && selected_index < static_cast<int>(entries.size())) {
                    if (!entries[static_cast<std::size_t>(selected_index)].disabled) return entries[static_cast<std::size_t>(selected_index)].id;
                    break;
                }
                if ((scancode == SDL_SCANCODE_S || key == SDLK_DOWN) && !entries.empty()) {
                    selected_index = std::min(static_cast<int>(entries.size()) - 1, selected_index + 1);
                    int item_top = selected_index * row_h;
                    if (item_top + row_h > scroll_y + content_h) scroll_y = item_top + row_h - content_h;
                    scroll_y = std::max(0, std::min(scroll_y, max_scroll));
                    break;
                }
                if ((scancode == SDL_SCANCODE_W || key == SDLK_UP) && !entries.empty()) {
                    selected_index = std::max(0, selected_index - 1);
                    int item_top = selected_index * row_h;
                    if (item_top < scroll_y) scroll_y = item_top;
                    scroll_y = std::max(0, std::min(scroll_y, max_scroll));
                    break;
                }
            }
            if (event.type == SDL_MOUSEWHEEL) {
                scroll_y -= event.wheel.y * 54;
                scroll_y = std::max(0, std::min(scroll_y, max_scroll));
                break;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                SDL_Point pt{event.button.x, event.button.y};
                if (SDL_PointInRect(&pt, &close_rect)) return cancel_id;
                const int right_x = panel_x + margin + left_w + gap;
                const SDL_Rect list_rect{right_x, content_y, right_w, content_h};
                if (max_scroll > 0) {
                    const int track_x = right_x + right_w - 22;
                    const SDL_Rect track{track_x, content_y, 10, content_h};
                    if (SDL_PointInRect(&pt, &track)) {
                        dragging_scrollbar = true;
                        drag_anchor_y = pt.y;
                        drag_anchor_scroll = scroll_y;
                        break;
                    }
                }
                if (SDL_PointInRect(&pt, &list_rect)) {
                    int local_y = pt.y - content_y + scroll_y;
                    int idx = local_y / row_h;
                    if (idx >= 0 && idx < static_cast<int>(entries.size())) {
                        selected_index = idx;
                        if (!entries[static_cast<std::size_t>(idx)].disabled) return entries[static_cast<std::size_t>(idx)].id;
                        break;
                    }
                }
            }
            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                dragging_scrollbar = false;
            }
            if (event.type == SDL_MOUSEMOTION && dragging_scrollbar && max_scroll > 0) {
                int dy = event.motion.y - drag_anchor_y;
                int scale_base = std::max(1, content_h - 42);
                scroll_y = drag_anchor_scroll + dy * max_scroll / scale_base;
                scroll_y = std::max(0, std::min(scroll_y, max_scroll));
                break;
            }
            break;
        }
    }
}


int GuiRenderer::show_action_panel(const std::string& title,
                                   const std::string& status_text,
                                   const std::string& recipe_title,
                                   const std::vector<GuiActionEntry>& actions,
                                   const std::vector<GuiActionEntry>& recipes,
                                   int cancel_id) {
    if (!ready_ || !window_ || !renderer_) return cancel_id;
    (void)recipes;

    const int panel_w = 1040;
    const int panel_h = 680;
    const int panel_x = (WINDOW_WIDTH - panel_w) / 2;
    const int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
    const int margin = 26;
    const int left_w = 520;
    const int right_w = panel_w - margin * 2 - left_w - 24;
    const int right_x = panel_x + margin + left_w + 24;
    const int list_y = panel_y + 176;
    const int row_h = 64;
    const int list_top = list_y + 34;
    const int list_bottom = panel_y + panel_h - margin - 58;
    const int max_visible_actions = std::max(1, (list_bottom - list_top) / row_h);
    int selected = 0;
    int scroll_offset = 0;

    /* unused */
    auto keep_selected_visible_real = [&]() {
        if (actions.empty()) {
            selected = 0;
            scroll_offset = 0;
            return;
        }

        selected = std::max(0, std::min(selected, static_cast<int>(actions.size()) - 1));
        int max_scroll = std::max(0, static_cast<int>(actions.size()) - max_visible_actions);
        if (selected < scroll_offset) scroll_offset = selected;
        if (selected >= scroll_offset + max_visible_actions) {
            scroll_offset = selected - max_visible_actions + 1;
        }
        scroll_offset = std::max(0, std::min(scroll_offset, max_scroll));
    };

    const int list_area_y = list_y - 14;
    const int list_area_h = panel_h - (list_y - panel_y) - margin;
    const int content_y = list_y + 24;
    const int content_h = list_area_y + list_area_h - content_y - 6;
    int max_scroll = std::max(0, static_cast<int>(actions.size()) * row_h - content_h);
    int scroll_y = 0;

    auto draw_locked_overlay = [&](int x, int y, int w, int h) {
        // 폰트/터미널 환경에 따라 LOCK 텍스트와 촘촘한 대각선 패턴이 깨져 보일 수 있어
        // 단순한 반투명 오버레이 + 벡터 자물쇠 아이콘으로 통일한다.
        fill_rect(x, y, w, h, SDL_Color{0, 0, 0, 105});
        fill_rect(x, y, 6, h, SDL_Color{118, 118, 118, 220});

        const int lock_w = 34;
        const int lock_h = 42;
        const int lx = x + w - lock_w - 18;
        const int ly = y + std::max(5, (h - lock_h) / 2);

        SDL_SetRenderDrawColor(renderer_, 220, 220, 210, 245);
        SDL_Rect shackle_outer{lx + 8, ly, 20, 22};
        SDL_Rect shackle_inner{lx + 13, ly + 6, 10, 17};
        SDL_RenderDrawRect(renderer_, &shackle_outer);
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 105);
        SDL_RenderDrawRect(renderer_, &shackle_inner);
        fill_rect(lx, ly + 18, lock_w, 24, SDL_Color{68, 68, 68, 235});
        draw_rect_outline(lx, ly + 18, lock_w, 24, SDL_Color{220, 220, 210, 245});
        fill_rect(lx + 15, ly + 28, 5, 8, SDL_Color{220, 220, 210, 245});
    };

    auto draw_panel = [&]() {
        fill_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_Color{0, 0, 0, 120});
        fill_rect(panel_x, panel_y, panel_w, panel_h, SDL_Color{44, 34, 26, 250});
        draw_rect_outline(panel_x, panel_y, panel_w, panel_h, SDL_Color{224, 184, 97, 255});
        draw_text(title, panel_x + margin, panel_y + 20, SDL_Color{255, 221, 132, 255}, panel_w - margin * 2);

        int sy = panel_y + 62;
        int status_lines = 0;
        for (const auto& line : wrap_text(status_text, 52)) {
            draw_text(line, panel_x + margin, sy, SDL_Color{238, 230, 207, 255}, left_w);
            sy += 24;
            if (++status_lines >= 4) break;
        }

        fill_rect(panel_x + margin, list_area_y, left_w, list_area_h, SDL_Color{31, 26, 20, 230});
        draw_rect_outline(panel_x + margin, list_area_y, left_w, list_area_h, SDL_Color{128, 93, 48, 255});
        draw_text("선택 목록 (W/S, Enter, 마우스)", panel_x + margin + 16, list_y - 6, SDL_Color{255, 221, 132, 255}, left_w - 32);

        SDL_Rect clip{panel_x + margin + 2, content_y, left_w - 4, content_h};
        SDL_RenderSetClipRect(renderer_, &clip);

        for (std::size_t i = 0; i < actions.size(); ++i) {
            int y = content_y + static_cast<int>(i) * row_h - scroll_y;
            if (y + row_h < content_y || y > content_y + content_h) continue;
            bool is_sel = static_cast<int>(i) == selected;
            SDL_Color base = actions[i].disabled ? SDL_Color{65, 58, 50, 220} : SDL_Color{73, 54, 35, 235};
            if (is_sel) base = actions[i].disabled ? SDL_Color{88, 78, 63, 235} : SDL_Color{118, 78, 38, 245};
            int x = panel_x + margin + 14;
            int w = left_w - 28;
            fill_rect(x, y, w, row_h - 8, base);
            draw_rect_outline(x, y, w, row_h - 8, is_sel ? SDL_Color{255, 230, 120, 255} : SDL_Color{232, 190, 95, 255});
            if (!actions[i].icon_key.empty()) {
                SDL_Rect icon{x + 12, y + 12, 34, 34};
                draw_texture(actions[i].icon_key, icon);
            }
            int tx = x + (actions[i].icon_key.empty() ? 16 : 58);
            draw_text(actions[i].title, tx, y + 8, actions[i].disabled ? SDL_Color{180, 170, 150, 255} : SDL_Color{255, 245, 213, 255}, w - (tx - x) - 20);
            if (!actions[i].subtitle.empty()) draw_text(actions[i].subtitle, tx, y + 36, SDL_Color{214, 200, 170, 255}, w - (tx - x) - 20);
            if (actions[i].disabled) draw_locked_overlay(x, y, w, row_h - 8);
        }
        SDL_RenderSetClipRect(renderer_, nullptr);

        if (max_scroll > 0) {
            const int track_x = panel_x + margin + left_w - 12;
            fill_rect(track_x, content_y, 8, content_h, SDL_Color{24, 20, 16, 255});
            int thumb_h = std::max(40, content_h * content_h / std::max(content_h, static_cast<int>(actions.size()) * row_h));
            int thumb_y = content_y + (content_h - thumb_h) * scroll_y / max_scroll;
            fill_rect(track_x, thumb_y, 8, thumb_h, SDL_Color{210, 170, 85, 255});
        }

        fill_rect(right_x, panel_y + 62, right_w, 270, SDL_Color{31, 26, 20, 230});
        draw_rect_outline(right_x, panel_y + 62, right_w, 270, SDL_Color{128, 93, 48, 255});
        draw_text(recipe_title, right_x + 16, panel_y + 74, SDL_Color{255, 221, 132, 255}, right_w - 32);
        
        if (selected >= 0 && selected < static_cast<int>(actions.size())) {
            const auto& r = actions[selected];
            int ry = panel_y + 112;
            if (!r.icon_key.empty()) { SDL_Rect icon{right_x + 16, ry, 64, 64}; draw_texture(r.icon_key, icon); }
            draw_text(r.title, right_x + 90, ry + 4, r.disabled ? SDL_Color{180, 170, 150, 255} : SDL_Color{255, 245, 213, 255}, right_w - 110);
            
            int subtitle_y = ry + 44;
            for (const auto& line : wrap_text(r.subtitle, 36)) {
                draw_text(line, right_x + 90, subtitle_y, SDL_Color{214, 200, 170, 255}, right_w - 110);
                subtitle_y += 24;
            }
            if (r.disabled) draw_locked_overlay(right_x + 10, ry - 10, right_w - 20, std::max(80, subtitle_y - ry + 10));
        }

        fill_rect(right_x, panel_y + 62, right_w, 260, SDL_Color{31, 26, 20, 230});
        draw_rect_outline(right_x, panel_y + 62, right_w, 260, SDL_Color{128, 93, 48, 255});
        draw_text(recipe_title, right_x + 16, panel_y + 74, SDL_Color{255, 221, 132, 255}, right_w - 32);
        if (!actions.empty()) {
            const GuiActionEntry& current = actions[static_cast<std::size_t>(selected)];
            if (!current.icon_key.empty()) {
                SDL_Rect icon{right_x + 18, panel_y + 116, 48, 48};
                draw_texture(current.icon_key, icon);
            }
            const int detail_x = right_x + (current.icon_key.empty() ? 16 : 78);
            draw_text(current.title, detail_x, panel_y + 118,
                      current.disabled ? SDL_Color{180, 170, 150, 255} : SDL_Color{255, 245, 213, 255},
                      right_w - (detail_x - right_x) - 16);
            int dy = panel_y + 154;
            for (const auto& line : wrap_text(current.subtitle.empty() ? "조건 없음" : current.subtitle, 34)) {
                draw_text(line, detail_x, dy, SDL_Color{214, 200, 170, 255}, right_w - (detail_x - right_x) - 16);
                dy += 24;
                if (dy > panel_y + 286) break;
            }
            if (current.disabled) draw_text("잠긴 항목입니다.", right_x + 16, panel_y + 292, SDL_Color{220, 180, 150, 255}, right_w - 32);
        }

        fill_rect(right_x, panel_y + 346, right_w, 128, SDL_Color{31, 26, 20, 230});
        draw_rect_outline(right_x, panel_y + 346, right_w, 128, SDL_Color{128, 93, 48, 255});
        draw_text("안내", right_x + 16, panel_y + 366, SDL_Color{255, 221, 132, 255}, right_w - 32);
        draw_text("조건이 부족한 항목은 쇠사슬처럼 잠겨 선택할 수 없습니다. 오른쪽 정보 영역에서 조건과 효과를 확인하세요.", right_x + 16, panel_y + 402, SDL_Color{238, 230, 207, 255}, right_w - 32);

        SDL_Rect close_rect{right_x + right_w - 150, panel_y + panel_h - margin - 46, 150, 46};
        fill_rect(close_rect.x, close_rect.y, close_rect.w, close_rect.h, SDL_Color{92, 70, 62, 255});
        draw_rect_outline(close_rect.x, close_rect.y, close_rect.w, close_rect.h, SDL_Color{232, 190, 95, 255});
        draw_text("나가기", close_rect.x + 44, close_rect.y + 13, SDL_Color{255, 245, 213, 255});
        SDL_RenderPresent(renderer_);
    };

    while (true) {
        draw_panel();
        SDL_Event event;
        while (SDL_WaitEvent(&event)) {
            if (event.type == SDL_QUIT) return cancel_id;
            if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;
                SDL_Scancode scancode = event.key.keysym.scancode;
                if (key == SDLK_ESCAPE || key == SDLK_0 || scancode == SDL_SCANCODE_0) return cancel_id;
                if ((scancode == SDL_SCANCODE_S || key == SDLK_DOWN) && !actions.empty()) {
                    selected = (selected + 1) % static_cast<int>(actions.size());
                    int item_top = selected * row_h;
                    if (item_top + row_h > scroll_y + content_h) scroll_y = item_top + row_h - content_h;
                    else if (item_top < scroll_y) scroll_y = item_top;
                    scroll_y = std::max(0, std::min(scroll_y, max_scroll));
                    break;
                }
                if ((scancode == SDL_SCANCODE_W || key == SDLK_UP) && !actions.empty()) {
                    selected = (selected - 1 + static_cast<int>(actions.size())) % static_cast<int>(actions.size());
                    int item_top = selected * row_h;
                    if (item_top < scroll_y) scroll_y = item_top;
                    else if (item_top + row_h > scroll_y + content_h) scroll_y = item_top + row_h - content_h;
                    scroll_y = std::max(0, std::min(scroll_y, max_scroll));
                    break;
                }
                if ((scancode == SDL_SCANCODE_SPACE || key == SDLK_RETURN || key == SDLK_KP_ENTER) && selected >= 0 && selected < static_cast<int>(actions.size())) {
                    if (!actions[selected].disabled) return actions[selected].id;
                    break;
                }
            }
            if (event.type == SDL_MOUSEWHEEL && !actions.empty()) {
                int max_scroll = std::max(0, static_cast<int>(actions.size()) - max_visible_actions);
                scroll_offset = std::max(0, std::min(scroll_offset - event.wheel.y, max_scroll));
                selected = std::max(scroll_offset, std::min(selected, scroll_offset + max_visible_actions - 1));
                break;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                SDL_Point pt{event.button.x, event.button.y};
                SDL_Rect close_rect{right_x + right_w - 150, panel_y + panel_h - margin - 46, 150, 46};
                if (SDL_PointInRect(&pt, &close_rect)) return cancel_id;
                int visible_end = std::min(static_cast<int>(actions.size()), scroll_offset + max_visible_actions);
                for (int i = scroll_offset; i < visible_end; ++i) {
                    int y = list_top + (i - scroll_offset) * row_h;
                    SDL_Rect r{panel_x + margin + 14, y, left_w - 28, row_h - 8};
                    if (SDL_PointInRect(&pt, &r)) {
                        selected = i;
                        if (!actions[i].disabled) return actions[i].id;
                        break;
                    }
                }
            }
            break;
        }
    }
}

int GuiRenderer::show_skill_tree_panel(const std::string& title,
                                       const std::string& status_text,
                                       const std::vector<GuiSkillNode>& nodes,
                                       int cancel_id) {
    if (!ready_ || !window_ || !renderer_ || nodes.empty()) return cancel_id;

    const int panel_w = 1040;
    const int panel_h = 680;
    const int panel_x = (WINDOW_WIDTH - panel_w) / 2;
    const int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
    const int margin = 26;
    const int tree_x = panel_x + margin;
    const int tree_y = panel_y + 116;
    const int tree_w = 650;
    const int tree_h = 504;
    const int detail_x = tree_x + tree_w + 24;
    const int detail_w = panel_w - margin * 2 - tree_w - 24;
    const int detail_y = tree_y;
    const int detail_h = tree_h;
    const int node_w = 128;
    const int node_h = 58;
    int selected = 0;

    auto node_at = [&](int id) -> const GuiSkillNode* {
        for (const auto& n : nodes) {
            if (n.id == id) return &n;
        }
        return nullptr;
    };

    auto node_rect = [&](const GuiSkillNode& n) {
        return SDL_Rect{tree_x + n.x, tree_y + n.y, node_w, node_h};
    };

    auto selected_node = [&]() -> const GuiSkillNode& {
        selected = std::max(0, std::min(selected, static_cast<int>(nodes.size()) - 1));
        return nodes[static_cast<std::size_t>(selected)];
    };

    auto draw_thick_line = [&](int x1, int y1, int x2, int y2, SDL_Color color) {
        SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
        for (int offset = -2; offset <= 2; ++offset) {
            SDL_RenderDrawLine(renderer_, x1 + offset, y1, x2 + offset, y2);
            SDL_RenderDrawLine(renderer_, x1, y1 + offset, x2, y2 + offset);
        }
    };

    auto draw_locked_overlay = [&](int x, int y, int w, int h) {
        fill_rect(x, y, w, h, SDL_Color{0, 0, 0, 105});
        const int lx = x + w - 28;
        const int ly = y + 10;
        SDL_SetRenderDrawColor(renderer_, 226, 218, 190, 245);
        SDL_Rect shackle{lx + 4, ly, 16, 18};
        SDL_RenderDrawRect(renderer_, &shackle);
        fill_rect(lx, ly + 16, 24, 22, SDL_Color{63, 63, 58, 245});
        draw_rect_outline(lx, ly + 16, 24, 22, SDL_Color{226, 218, 190, 245});
    };

    auto index_for_id = [&](int id) {
        for (std::size_t i = 0; i < nodes.size(); ++i) {
            if (nodes[i].id == id) return static_cast<int>(i);
        }
        return -1;
    };

    auto shares_parent = [&](const GuiSkillNode& a, const GuiSkillNode& b) {
        for (int ap : a.parents) {
            for (int bp : b.parents) {
                if (ap == bp) return true;
            }
        }
        return false;
    };

    auto child_indices = [&](const GuiSkillNode& parent) {
        std::vector<int> result;
        for (std::size_t i = 0; i < nodes.size(); ++i) {
            if (std::find(nodes[i].parents.begin(), nodes[i].parents.end(), parent.id) != nodes[i].parents.end()) {
                result.push_back(static_cast<int>(i));
            }
        }
        std::sort(result.begin(), result.end(), [&](int a, int b) {
            if (nodes[a].y != nodes[b].y) return nodes[a].y < nodes[b].y;
            return nodes[a].x < nodes[b].x;
        });
        return result;
    };

    auto best_by_direction = [&](const std::vector<int>& candidates, int dx, int dy) {
        const GuiSkillNode& cur = selected_node();
        int best = -1;
        int best_score = 1000000;
        for (int idx : candidates) {
            if (idx < 0 || idx >= static_cast<int>(nodes.size()) || idx == selected) continue;
            int vx = nodes[idx].x - cur.x;
            int vy = nodes[idx].y - cur.y;
            if (dx != 0 && vx * dx < 0) continue;
            if (dy != 0 && vy * dy < 0) continue;
            int score = vx * vx + vy * vy;
            if (dx != 0) score -= std::abs(vx) * 6;
            if (dy != 0) score -= std::abs(vy) * 6;
            if (score < best_score) {
                best_score = score;
                best = idx;
            }
        }
        return best;
    };

    auto move_selection = [&](int dx, int dy) {
        const GuiSkillNode& cur = selected_node();

        if (dy > 0) {
            std::vector<int> children = child_indices(cur);
            int child = best_by_direction(children, dx, 1);
            if (child >= 0) {
                selected = child;
                return;
            }
        }

        if (dy < 0 && !cur.parents.empty()) {
            std::vector<int> parents;
            for (int parent_id : cur.parents) {
                int idx = index_for_id(parent_id);
                if (idx >= 0) parents.push_back(idx);
            }
            int parent = best_by_direction(parents, dx, -1);
            if (parent >= 0) {
                selected = parent;
                return;
            }
        }

        if (dx != 0 && dy == 0) {
            std::vector<int> same_row;
            for (std::size_t i = 0; i < nodes.size(); ++i) {
                if (static_cast<int>(i) == selected) continue;
                if (std::abs(nodes[i].y - cur.y) <= 28 && (nodes[i].x - cur.x) * dx > 0) {
                    same_row.push_back(static_cast<int>(i));
                }
            }
            int row_target = best_by_direction(same_row, dx, 0);
            if (row_target >= 0) {
                selected = row_target;
                return;
            }

            std::vector<int> sibling_targets;
            for (std::size_t i = 0; i < nodes.size(); ++i) {
                if (static_cast<int>(i) == selected) continue;
                if (shares_parent(cur, nodes[i]) && (nodes[i].x - cur.x) * dx > 0) {
                    sibling_targets.push_back(static_cast<int>(i));
                }
            }
            int sibling = best_by_direction(sibling_targets, dx, 0);
            if (sibling >= 0) {
                selected = sibling;
                return;
            }

            std::vector<int> children = child_indices(cur);
            int branch_child = best_by_direction(children, dx, 1);
            if (branch_child >= 0) {
                selected = branch_child;
                return;
            }
        }

        int cur_cx = cur.x + node_w / 2;
        int cur_cy = cur.y + node_h / 2;
        int best = selected;
        int best_score = 1000000;
        for (std::size_t i = 0; i < nodes.size(); ++i) {
            if (static_cast<int>(i) == selected) continue;
            int nx = nodes[i].x + node_w / 2;
            int ny = nodes[i].y + node_h / 2;
            int vx = nx - cur_cx;
            int vy = ny - cur_cy;
            if (dx != 0 && vx * dx <= 0) continue;
            if (dy != 0 && vy * dy <= 0) continue;
            if (dx != 0 && dy == 0 && std::abs(vx) < std::abs(vy) / 2) continue;
            if (dy != 0 && dx == 0 && std::abs(vy) < std::abs(vx) / 3) continue;
            int score = vx * vx + vy * vy;
            if (dx != 0 && dy != 0) score += std::abs(std::abs(vx) - std::abs(vy)) * 3;
            if (score < best_score) {
                best_score = score;
                best = static_cast<int>(i);
            }
        }
        selected = best;
    };

    auto move_from_pressed_keys = [&]() {
        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        int dx = 0;
        int dy = 0;
        if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) --dx;
        if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) ++dx;
        if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) --dy;
        if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) ++dy;
        if (dx != 0 || dy != 0) move_selection(dx, dy);
    };

    auto draw_panel = [&]() {
        fill_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_Color{0, 0, 0, 120});
        fill_rect(panel_x, panel_y, panel_w, panel_h, SDL_Color{42, 35, 27, 250});
        draw_rect_outline(panel_x, panel_y, panel_w, panel_h, SDL_Color{224, 184, 97, 255});
        draw_text(title, panel_x + margin, panel_y + 20, SDL_Color{255, 221, 132, 255}, panel_w - margin * 2);

        int sy = panel_y + 60;
        for (const auto& line : wrap_text(status_text, 72)) {
            draw_text(line, panel_x + margin, sy, SDL_Color{238, 230, 207, 255}, panel_w - margin * 2);
            sy += 24;
            if (sy > tree_y - 8) break;
        }

        fill_rect(tree_x, tree_y, tree_w, tree_h, SDL_Color{26, 29, 24, 238});
        draw_rect_outline(tree_x, tree_y, tree_w, tree_h, SDL_Color{116, 88, 47, 255});
        draw_text("스킬 가지", tree_x + 18, tree_y + 14, SDL_Color{255, 221, 132, 255}, tree_w - 36);

        for (const auto& child : nodes) {
            SDL_Rect cr = node_rect(child);
            int c_cx = cr.x + cr.w / 2;
            int c_cy = cr.y + cr.h / 2;
            for (int parent_id : child.parents) {
                const GuiSkillNode* parent = node_at(parent_id);
                if (!parent) continue;
                SDL_Rect pr = node_rect(*parent);
                int p_cx = pr.x + pr.w / 2;
                int p_cy = pr.y + pr.h / 2;
                SDL_Color branch = parent->learned ? SDL_Color{158, 126, 67, 255} : SDL_Color{91, 72, 48, 255};
                draw_thick_line(p_cx, p_cy, c_cx, c_cy, branch);
            }
        }

        for (std::size_t i = 0; i < nodes.size(); ++i) {
            const auto& n = nodes[i];
            SDL_Rect r = node_rect(n);
            bool is_sel = static_cast<int>(i) == selected;
            SDL_Color base = n.learned ? SDL_Color{54, 102, 73, 245}
                             : n.disabled ? SDL_Color{62, 59, 52, 235}
                             : SDL_Color{104, 73, 38, 245};
            SDL_Color outline = is_sel ? SDL_Color{255, 232, 125, 255}
                                : n.learned ? SDL_Color{142, 209, 151, 255}
                                : SDL_Color{190, 150, 78, 255};
            fill_rect(r.x, r.y, r.w, r.h, base);
            draw_rect_outline(r.x, r.y, r.w, r.h, outline);
            if (!n.icon_key.empty()) {
                SDL_Rect icon{r.x + 10, r.y + 12, 34, 34};
                draw_texture(n.icon_key, icon);
            }
            int tx = r.x + (n.icon_key.empty() ? 12 : 50);
            draw_text(n.title, tx, r.y + 9, n.disabled ? SDL_Color{186, 178, 158, 255} : SDL_Color{255, 246, 218, 255}, r.w - (tx - r.x) - 10);
            draw_text(n.learned ? "습득 완료" : (n.disabled ? "잠김" : "투자 가능"),
                      tx, r.y + 33,
                      n.learned ? SDL_Color{194, 246, 196, 255} : SDL_Color{222, 207, 173, 255},
                      r.w - (tx - r.x) - 10);
            if (n.disabled && !n.learned) draw_locked_overlay(r.x, r.y, r.w, r.h);
        }

        fill_rect(detail_x, detail_y, detail_w, detail_h, SDL_Color{31, 26, 20, 238});
        draw_rect_outline(detail_x, detail_y, detail_w, detail_h, SDL_Color{128, 93, 48, 255});
        const GuiSkillNode& n = selected_node();
        draw_text("선택한 스킬", detail_x + 18, detail_y + 18, SDL_Color{255, 221, 132, 255}, detail_w - 36);
        if (!n.icon_key.empty()) {
            SDL_Rect icon{detail_x + 18, detail_y + 58, 52, 52};
            draw_texture(n.icon_key, icon);
        }
        draw_text(n.title, detail_x + 84, detail_y + 62, SDL_Color{255, 246, 218, 255}, detail_w - 102);
        std::string state = n.learned ? "상태: 습득 완료" : (n.disabled ? "상태: 선행 스킬 필요" : "상태: 투자 가능");
        draw_text(state, detail_x + 84, detail_y + 90,
                  n.disabled ? SDL_Color{210, 185, 160, 255} : SDL_Color{194, 246, 196, 255},
                  detail_w - 102);

        int dy = detail_y + 136;
        draw_text("효과", detail_x + 18, dy, SDL_Color{255, 221, 132, 255}, detail_w - 36);
        dy += 34;
        for (const auto& line : wrap_text(n.detail, 34)) {
            draw_text(line, detail_x + 18, dy, SDL_Color{238, 230, 207, 255}, detail_w - 36);
            dy += 27;
            if (dy > detail_y + detail_h - 124) break;
        }

        SDL_Rect invest_rect{detail_x + 18, detail_y + detail_h - 102, detail_w - 36, 46};
        SDL_Color invest_color = (!n.disabled && !n.learned) ? SDL_Color{112, 82, 42, 255} : SDL_Color{72, 64, 54, 255};
        fill_rect(invest_rect.x, invest_rect.y, invest_rect.w, invest_rect.h, invest_color);
        draw_rect_outline(invest_rect.x, invest_rect.y, invest_rect.w, invest_rect.h, SDL_Color{232, 190, 95, 255});
        draw_text(n.learned ? "이미 습득한 스킬" : "투자하기 (Enter)", invest_rect.x + 52, invest_rect.y + 13,
                  (!n.disabled && !n.learned) ? SDL_Color{255, 245, 213, 255} : SDL_Color{190, 180, 160, 255},
                  invest_rect.w - 104);

        SDL_Rect close_rect{detail_x + 18, detail_y + detail_h - 46, detail_w - 36, 40};
        fill_rect(close_rect.x, close_rect.y, close_rect.w, close_rect.h, SDL_Color{92, 70, 62, 255});
        draw_rect_outline(close_rect.x, close_rect.y, close_rect.w, close_rect.h, SDL_Color{232, 190, 95, 255});
        draw_text("나가기", close_rect.x + 86, close_rect.y + 10, SDL_Color{255, 245, 213, 255}, close_rect.w - 172);
        SDL_RenderPresent(renderer_);
    };

    while (true) {
        draw_panel();
        SDL_Event event;
        while (SDL_WaitEvent(&event)) {
            if (event.type == SDL_QUIT) return cancel_id;
            if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_ESCAPE || key == SDLK_0) return cancel_id;
                if (key == SDLK_a || key == SDLK_LEFT ||
                    key == SDLK_d || key == SDLK_RIGHT ||
                    key == SDLK_w || key == SDLK_UP ||
                    key == SDLK_s || key == SDLK_DOWN) {
                    move_from_pressed_keys();
                    break;
                }
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE) {
                    const GuiSkillNode& n = selected_node();
                    if (!n.disabled && !n.learned) return n.id;
                    break;
                }
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                SDL_Point pt{event.button.x, event.button.y};
                SDL_Rect invest_rect{detail_x + 18, detail_y + detail_h - 102, detail_w - 36, 46};
                SDL_Rect close_rect{detail_x + 18, detail_y + detail_h - 46, detail_w - 36, 40};
                if (SDL_PointInRect(&pt, &close_rect)) return cancel_id;
                if (SDL_PointInRect(&pt, &invest_rect)) {
                    const GuiSkillNode& n = selected_node();
                    if (!n.disabled && !n.learned) return n.id;
                    break;
                }
                for (std::size_t i = 0; i < nodes.size(); ++i) {
                    SDL_Rect r = node_rect(nodes[i]);
                    if (SDL_PointInRect(&pt, &r)) {
                        selected = static_cast<int>(i);
                        break;
                    }
                }
            }
            break;
        }
    }
}

bool GuiRenderer::poll_input(char& input) {
    SDL_Event event;
    Uint32 now = SDL_GetTicks();
    const Uint32 move_interval_ms = 145;

    auto movement_from_key = [](SDL_Keycode key) -> char {
        switch (key) {
            case SDLK_w: return 'w';
            case SDLK_s: return 's';
            case SDLK_a: return 'a';
            case SDLK_d: return 'd';
            default: return 0;
        }
    };

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { input = 'q'; return true; }
        if (event.type == SDL_KEYDOWN) {
            SDL_Keycode key = event.key.keysym.sym;
            char move = movement_from_key(key);
            if (move != 0 && now - last_move_event_ticks_ >= move_interval_ms) {
                input = move;
                last_move_event_ticks_ = now;
                return true;
            }
            if (!event.key.repeat) {
                SDL_Scancode scancode = event.key.keysym.scancode;
                if (scancode == SDL_SCANCODE_J || scancode == SDL_SCANCODE_RETURN || scancode == SDL_SCANCODE_KP_ENTER || scancode == SDL_SCANCODE_SPACE) { input = 'j'; return true; }
                if (scancode == SDL_SCANCODE_T) { input = 't'; return true; }
                if (scancode == SDL_SCANCODE_E) { input = 'e'; return true; }
                if (scancode == SDL_SCANCODE_P) { input = 'p'; return true; }
                if (scancode == SDL_SCANCODE_I) { input = 'i'; return true; }
                if (scancode == SDL_SCANCODE_U) { input = 'u'; return true; }
                if (scancode == SDL_SCANCODE_5 || key == SDLK_5) { input = '5'; return true; }
                if (scancode == SDL_SCANCODE_Q || key == SDLK_ESCAPE) { input = 'q'; return true; }
            }
        }
    }

    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    now = SDL_GetTicks();
    if (now - last_move_event_ticks_ < move_interval_ms) return false;

    // 이동은 WASD만 허용한다. 화살표/숫자패드 방향키는 키보드 환경별 오입력 문제가 있어 사용하지 않는다.
    if (keys[SDL_SCANCODE_W]) { input = 'w'; last_move_event_ticks_ = now; return true; }
    if (keys[SDL_SCANCODE_S]) { input = 's'; last_move_event_ticks_ = now; return true; }
    if (keys[SDL_SCANCODE_A]) { input = 'a'; last_move_event_ticks_ = now; return true; }
    if (keys[SDL_SCANCODE_D]) { input = 'd'; last_move_event_ticks_ = now; return true; }
    return false;
}

void GuiRenderer::fill_rect(int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    SDL_Rect r{x, y, w, h};
    SDL_RenderFillRect(renderer_, &r);
}

void GuiRenderer::draw_rect_outline(int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    SDL_Rect r{x, y, w, h};
    SDL_RenderDrawRect(renderer_, &r);
}

bool GuiRenderer::load_font() {
    const char* candidates[] = {
        // Cross-platform bundled font
        "assets/NanumGothic.ttf",
        // macOS CJK Fonts
        "/System/Library/Fonts/Supplemental/AppleGothic.ttf",
        "/System/Library/Fonts/AppleSDGothicNeo.ttc",
        // Windows CJK Fonts
        "C:\\Windows\\Fonts\\malgun.ttf",
        "C:\\Windows\\Fonts\\msgothic.ttc",
        // Linux CJK Fonts
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/nanum/NanumGothic.ttf",
        // Fallbacks
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf"
    };
    for (const char* path : candidates) {
        font_ = TTF_OpenFont(path, 18);
        if (font_) return true;
    }
    return false;
}

std::vector<std::string> GuiRenderer::wrap_text(const std::string& text, int max_chars_per_line) const {
    std::vector<std::string> lines;
    std::string current;
    for (char ch : text) {
        if (ch == '\n') {
            lines.push_back(current);
            current.clear();
            continue;
        }
        current.push_back(ch);
        if (static_cast<int>(current.size()) >= max_chars_per_line && ch == ' ') {
            lines.push_back(current);
            current.clear();
        }
    }
    if (!current.empty()) lines.push_back(current);
    if (lines.empty()) lines.push_back("");
    return lines;
}

void GuiRenderer::draw_text(const std::string& text, int x, int y, SDL_Color color, int max_width) {
    if (!font_ || text.empty()) return;
    const std::string safe_text = sanitize_gui_text(text);
    if (safe_text.empty()) return;
    const std::string key = safe_text + "|" + std::to_string(color.r) + "," +
                            std::to_string(color.g) + "," + std::to_string(color.b) + "," +
                            std::to_string(color.a);
    auto it = text_cache_.find(key);
    if (it == text_cache_.end()) {
        if (text_cache_.size() > 1024) clear_text_cache();
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font_, safe_text.c_str(), color);
        if (!surface) return;
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer_, surface);
        if (!tex) { SDL_FreeSurface(surface); return; }
        CachedTextTexture cached{tex, surface->w, surface->h};
        SDL_FreeSurface(surface);
        it = text_cache_.emplace(key, cached).first;
    }

    SDL_Rect dst{x, y, it->second.width, it->second.height};
    if (max_width > 0 && dst.w > max_width) {
        double scale = static_cast<double>(max_width) / std::max(1, dst.w);
        dst.w = max_width;
        dst.h = std::max(1, static_cast<int>(dst.h * scale));
    }
    SDL_RenderCopy(renderer_, it->second.texture, nullptr, &dst);
}

void GuiRenderer::draw_bar(int x, int y, int w, int h, double ratio, SDL_Color fill) {
    ratio = std::max(0.0, std::min(1.0, ratio));
    fill_rect(x, y, w, h, SDL_Color{61, 48, 35, 255});
    fill_rect(x + 2, y + 2, static_cast<int>((w - 4) * ratio), h - 4, fill);
}

void GuiRenderer::draw_tile_base(const Tile& tile, int x, int y) {
    SDL_Rect dst{x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    switch (tile.type) {
        case TileType::WATER: draw_texture("water", dst); break;
        case TileType::SAND: draw_texture("sand", dst); break;
        case TileType::LIVESTOCK: draw_texture("livestock_tile", dst); break;
        case TileType::FARM: draw_texture("farm_tile", dst); break;
        case TileType::BUILDING: draw_texture("building_tile", dst); break;
        case TileType::BED: draw_texture("building_tile", dst); break;
        case TileType::MARKET: draw_texture("building_tile", dst); break;
        case TileType::KITCHEN: draw_texture("building_tile", dst); break;
        case TileType::WORKSHOP: draw_texture("building_tile", dst); break;
        case TileType::TREE: draw_texture("path", dst); break;
        case TileType::PATH: default: draw_texture("path", dst); break;
    }
}

void GuiRenderer::draw_tile_object(const Tile& tile, int x, int y) {
    SDL_Rect dst{x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};

    if (!tile.building_name.empty()) {
        if (!tile.building_origin) return;
        SDL_Rect big_dst{x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE * 2, TILE_SIZE * 2};
        if (tile.building_name == "상점") draw_texture("market", big_dst);
        else if (tile.building_name == "주방") draw_texture("kitchen", big_dst);
        else if (tile.building_name == "작업장" || tile.building_name == "Workshop") draw_texture("workshop", big_dst);
        else if (tile.building_name == "저장소") draw_texture("storage", big_dst);
        else if (tile.building_name == "축사") draw_texture("barn", big_dst);
        else draw_texture("build_plot", big_dst);

        // 건물 레벨에 따른 외형 변화: 주방/작업장/저장소는 레벨 배지와 밝은 테두리를 추가한다.
        if (tile.building_name != "상점") {
            const int lv = std::max(1, tile.building_level);
            if (lv >= 2) draw_rect_outline(big_dst.x + 2, big_dst.y + 2, big_dst.w - 4, big_dst.h - 4, SDL_Color{232, 190, 95, 255});
            if (lv >= 3) draw_rect_outline(big_dst.x + 6, big_dst.y + 6, big_dst.w - 12, big_dst.h - 12, SDL_Color{255, 232, 120, 255});
            fill_rect(big_dst.x + big_dst.w - 48, big_dst.y + 8, 40, 24, SDL_Color{44, 34, 26, 220});
            draw_rect_outline(big_dst.x + big_dst.w - 48, big_dst.y + 8, 40, 24, SDL_Color{232, 190, 95, 255});
            draw_text("Lv" + std::to_string(lv), big_dst.x + big_dst.w - 44, big_dst.y + 13, SDL_Color{255, 245, 213, 255});
        }
        return;
    }

    // 비어 있는 2x2 건축 부지도 한 장짜리 2x2 스프라이트로 표시한다.
    if (tile.type == TileType::BUILDING && tile.building_name.empty()) {
        if (!tile.building_origin) return;
        SDL_Rect big_dst{x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE * 2, TILE_SIZE * 2};
        draw_texture("build_plot", big_dst);
        return;
    }

    if (!tile.door_target_name.empty()) {
        // 문 앞 타일을 살짝 강조해서 상호작용 위치를 알 수 있게 한다.
        fill_rect(dst.x + 18, dst.y + 46, 28, 10, SDL_Color{160, 104, 52, 230});
        draw_rect_outline(dst.x + 18, dst.y + 46, 28, 10, SDL_Color{70, 45, 25, 255});
    }

    switch (tile.type) {
        case TileType::BED: draw_texture("bed", dst); return;
        case TileType::MARKET: draw_texture("market", dst); return;
        case TileType::KITCHEN: draw_texture("kitchen", dst); return;
        case TileType::WORKSHOP: draw_texture("workshop", dst); return;
        case TileType::TREE:
            {
                SDL_Rect tree_dst{x * TILE_SIZE + 2, y * TILE_SIZE - 8, TILE_SIZE - 4, TILE_SIZE + 8};
                draw_texture("wood_tree", tree_dst);
            }
            return;
        case TileType::BUILDING: draw_texture("build_plot", dst); break;
        default: break;
    }

    if (tile.has_crop()) {
        std::string key = "sprout";
        const bool ready = tile.crop_progress >= tile.crop_max_progress && tile.crop_harvest_cooldown == 0;
        if (ready) {
            if (tile.crop_id == 1) key = "potato";
            else if (tile.crop_id == 2) key = "carrot";
            else if (tile.crop_id == 3) key = "wheat";
            else if (tile.crop_id == 4) key = "rice";
            else if (tile.crop_id == 5) key = "apple_tree";
        } else if (tile.crop_id == 5) {
            key = tile.crop_progress >= tile.crop_max_progress ? "apple_empty" : "apple_sapling";
        }
        SDL_Rect crop_dst{x * TILE_SIZE + 4, y * TILE_SIZE + 4, TILE_SIZE - 8, TILE_SIZE - 8};
        draw_texture(key, crop_dst);
        if (tile.watered_today) {
            SDL_Rect water_dst{x * TILE_SIZE + 34, y * TILE_SIZE + 34, 24, 24};
            draw_texture("water_drop", water_dst);
        }
    }

    if (tile.has_animal()) {
        const bool adult = tile.animal_age >= tile.animal_adult_age;
        std::string key;
        if (tile.animal_type == 1) key = adult ? "chicken" : "baby_chick";
        else if (tile.animal_type == 2) key = adult ? "pig" : "baby_pig";
        else if (tile.animal_type == 3) key = adult ? "cow" : "baby_cow";
        if (!key.empty()) {
            SDL_Rect animal_dst{x * TILE_SIZE + 2, y * TILE_SIZE + 2, TILE_SIZE - 4, TILE_SIZE - 4};
            draw_texture(key, animal_dst);
        }
    }
}

void GuiRenderer::draw_side_panel(const Player& player, const std::string& message) {
    const int ox = Map::WIDTH * TILE_SIZE;
    fill_rect(ox, 0, PANEL_WIDTH, WINDOW_HEIGHT, SDL_Color{40, 32, 24, 255});
    fill_rect(ox + 16, 16, PANEL_WIDTH - 32, WINDOW_HEIGHT - 32, SDL_Color{74, 58, 39, 255});
    draw_rect_outline(ox + 16, 16, PANEL_WIDTH - 32, WINDOW_HEIGHT - 32, SDL_Color{122, 94, 54, 255});

    draw_text("농장 마을", ox + 34, 28, SDL_Color{255, 221, 132, 255}, PANEL_WIDTH - 68);
    
    fill_rect(ox + 24, 66, PANEL_WIDTH - 48, 126, SDL_Color{30, 24, 20, 225});
    draw_rect_outline(ox + 24, 66, PANEL_WIDTH - 48, 126, SDL_Color{122, 94, 54, 255});
    draw_text("메시지", ox + 36, 78, SDL_Color{255, 221, 132, 255});
    int y = 110;
    for (const auto& line : wrap_text(message.empty() ? "WASD 이동 / J 상호작용 / E 인벤토리" : message, 28)) {
        draw_text(line, ox + 36, y, SDL_Color{238, 230, 207, 255}, PANEL_WIDTH - 72);
        y += 22;
        if (y > 178) break;
    }

    draw_text("일차 / 계절", ox + 34, 210, SDL_Color{214, 197, 166, 255});
    std::ostringstream day_season;
    day_season << player.get_day() << "  " << season_name(season_from_day(player.get_day()));
    draw_text(day_season.str(), ox + 150, 210, SDL_Color{255, 245, 213, 255});

    SDL_Rect coin{ox + 28, 246, 34, 34}; draw_texture("coin", coin);
    draw_text("골드", ox + 72, 250, SDL_Color{214, 197, 166, 255});
    draw_text(std::to_string(player.get_money()) + " G", ox + 150, 250, SDL_Color{255, 245, 213, 255});

    SDL_Rect level{ox + 28, 292, 34, 34}; draw_texture("level", level);
    draw_text("레벨", ox + 72, 296, SDL_Color{214, 197, 166, 255});
    draw_text("Lv." + std::to_string(player.get_level()), ox + 150, 296, SDL_Color{255, 245, 213, 255});

    SDL_Rect energy{ox + 28, 342, 34, 34}; draw_texture("energy", energy);
    std::ostringstream stamina_text;
    stamina_text << "스태미나  " << player.get_stamina() << "/" << player.get_stamina_max();
    draw_text(stamina_text.str(), ox + 72, 338, SDL_Color{255, 245, 213, 255}, 190);
    draw_bar(ox + 72, 364, 190, 20,
             static_cast<double>(player.get_stamina()) / std::max(1, player.get_stamina_max()),
             SDL_Color{244, 190, 61, 255});

    SDL_Rect xp_icon{ox + 28, 402, 34, 34}; draw_texture("xp", xp_icon);
    int exp_to_next = player.get_exp_to_next();
    std::ostringstream xp_text;
    xp_text << "XP  " << player.get_exp() << " / " << (player.get_exp() + exp_to_next);
    draw_text(xp_text.str(), ox + 72, 398, SDL_Color{255, 245, 213, 255}, 190);
    draw_bar(ox + 72, 424, 190, 20,
             exp_to_next <= 0 ? 1.0 : static_cast<double>(player.get_exp()) / std::max(1, player.get_exp() + exp_to_next),
             SDL_Color{91, 181, 255, 255});

    SDL_Rect bag{ox + 28, 462, 34, 34}; draw_texture("bag", bag);
    std::ostringstream inv_text;
    inv_text << "인벤토리  " << player.inventory().used_slots() << "/" << player.inventory().max_slots();
    draw_text(inv_text.str(), ox + 72, 466, SDL_Color{255, 245, 213, 255}, 190);
    draw_bar(ox + 72, 492, 190, 18,
             static_cast<double>(player.inventory().used_slots()) / std::max(1, player.inventory().max_slots()),
             SDL_Color{117, 199, 121, 255});

    draw_text("이벤트", ox + 34, 538, SDL_Color{255, 221, 132, 255});
    bool any_event = false;
    int event_y = 574;
    if (player.is_sick_today()) {
        SDL_Rect cold{ox + 34, event_y, 42, 42}; draw_texture("cold", cold);
        draw_text("감기: 스태미나 50%", ox + 86, event_y + 10, SDL_Color{238, 230, 207, 255}, 180);
        event_y += 52;
        any_event = true;
    }
    for (const auto& line : event_status_lines_) {
        const bool drought_line = line.find("가뭄") != std::string::npos || line.find("Drought") != std::string::npos;
        if (drought_line) {
            SDL_Rect drought{ox + 34, event_y, 42, 42}; draw_texture("drought", drought);
            draw_text(line, ox + 86, event_y + 8, SDL_Color{238, 230, 207, 255}, 178);
            event_y += 54;
        } else {
            draw_text(line, ox + 34, event_y + 8, SDL_Color{238, 230, 207, 255}, 220);
            event_y += 30;
        }
        any_event = true;
    }
    if (!any_event) draw_text("없음", ox + 34, 574, SDL_Color{188, 176, 150, 255});
}
void GuiRenderer::update_window_title(const Player& player, const std::string& message) {
    (void)player;
    (void)message;
    SDL_SetWindowTitle(window_, "농장 마을");
}

bool GuiRenderer::needs_animation_frame(const Player& player) const {
    if (!ready_ || !visual_position_initialized_) return true;
    const double target_x = static_cast<double>(player.get_x());
    const double target_y = static_cast<double>(player.get_y());
    return std::fabs(visual_x_ - target_x) > 0.01 || std::fabs(visual_y_ - target_y) > 0.01;
}

void GuiRenderer::draw(const Player& player, const Map& map, const std::string& message) {
    if (!ready_) return;
    SDL_SetRenderDrawColor(renderer_, 18, 24, 20, 255);
    SDL_RenderClear(renderer_);

    // 1-pass: draw only the ground/base tiles first.
    // In v12, 2x2 building sprites are larger than one tile.
    // Drawing a 2x2 sprite while still drawing later base tiles caused the
    // bottom/right half of the building to be overwritten, which looked like
    // the GUI was broken.  Therefore base tiles and objects must be rendered
    // in separate passes.
    for (int y = 0; y < Map::HEIGHT; ++y) {
        for (int x = 0; x < Map::WIDTH; ++x) {
            draw_tile_base(map.get_tile(x, y), x, y);
        }
    }

    // 2-pass: draw tile objects after every base tile is already finished.
    // This keeps 2x2 buildings, 2x2 build plots, crops, animals, mine and
    // door markers from being overwritten by neighboring tile bases.
    for (int y = 0; y < Map::HEIGHT; ++y) {
        for (int x = 0; x < Map::WIDTH; ++x) {
            const Tile& tile = map.get_tile(x, y);
            if (x == Map::MINE_X && y == Map::MINE_Y) {
                SDL_Rect mine_dst{x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                draw_texture("mine", mine_dst);
            }
            draw_tile_object(tile, x, y);
        }
    }

    Uint32 now_ticks = SDL_GetTicks();
    if (!visual_position_initialized_) {
        visual_x_ = static_cast<double>(player.get_x());
        visual_y_ = static_cast<double>(player.get_y());
        last_frame_ticks_ = now_ticks;
        visual_position_initialized_ = true;
    }
    double dt = std::min(0.05, static_cast<double>(now_ticks - last_frame_ticks_) / 1000.0);
    last_frame_ticks_ = now_ticks;
    const double target_x = static_cast<double>(player.get_x());
    const double target_y = static_cast<double>(player.get_y());
    if (std::fabs(visual_x_ - target_x) > 4.0 || std::fabs(visual_y_ - target_y) > 4.0) {
        visual_x_ = target_x;
        visual_y_ = target_y;
    }
    const double speed_tiles_per_second = 6.0;
    auto move_towards = [dt, speed_tiles_per_second](double current, double target) {
        double diff = target - current;
        double step = speed_tiles_per_second * dt;
        if (std::fabs(diff) <= step) return target;
        return current + (diff > 0.0 ? step : -step);
    };
    visual_x_ = move_towards(visual_x_, target_x);
    visual_y_ = move_towards(visual_y_, target_y);

    SDL_Rect player_dst{static_cast<int>(visual_x_ * TILE_SIZE) + 8,
                        static_cast<int>(visual_y_ * TILE_SIZE) + 2,
                        TILE_SIZE - 16,
                        TILE_SIZE - 4};
    std::string player_key = "player_front";
    if (player.get_facing() == 'b') player_key = "player_back";
    else if (player.get_facing() == 'l') player_key = "player_left";
    else if (player.get_facing() == 'r') player_key = "player_right";
    draw_texture(player_key, player_dst);

    draw_side_panel(player, message);
    SDL_RenderPresent(renderer_);
}

void GuiRenderer::draw_barn(const Player& player, const BarnSystem& barn, const std::string& message) {
    SDL_SetRenderDrawColor(renderer_, 22, 20, 18, 255);
    SDL_RenderClear(renderer_);

    for (int y = 0; y < BarnSystem::HEIGHT; ++y) {
        for (int x = 0; x < BarnSystem::WIDTH; ++x) {
            SDL_Rect dst{x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            const bool wall = (x == 0 || y == 0 || x == BarnSystem::WIDTH - 1 || y == BarnSystem::HEIGHT - 1);
            if (wall) {
                if (texture("barn_wall")) draw_texture("barn_wall", dst);
                else fill_rect(dst.x, dst.y, dst.w, dst.h, SDL_Color{82, 50, 30, 255});
            } else {
                if (texture("barn_floor")) draw_texture("barn_floor", dst);
                else fill_rect(dst.x, dst.y, dst.w, dst.h, SDL_Color{128, 88, 50, 255});
            }
        }
    }

    SDL_Rect exit_rect{BarnSystem::EXIT_X * TILE_SIZE, BarnSystem::EXIT_Y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    fill_rect(exit_rect.x, exit_rect.y, exit_rect.w, exit_rect.h, SDL_Color{190, 150, 92, 255});
    if (texture("barn_gate")) draw_texture("barn_gate", exit_rect);
    draw_rect_outline(exit_rect.x, exit_rect.y, exit_rect.w, exit_rect.h, SDL_Color{255, 230, 130, 255});
    draw_text("출구", exit_rect.x + 12, exit_rect.y + 22, SDL_Color{50, 32, 20, 255}, TILE_SIZE - 8);

    auto tex = [&](const std::string& key, int tx, int ty, int w = TILE_SIZE, int h = TILE_SIZE, int ox = 0, int oy = 0) {
        SDL_Rect r{tx * TILE_SIZE + ox, ty * TILE_SIZE + oy, w, h};
        draw_texture(key, r);
    };

    SDL_Rect panel{2 * TILE_SIZE, 1 * TILE_SIZE, 8 * TILE_SIZE, TILE_SIZE};
    if (texture("barn_wall_panel")) draw_texture("barn_wall_panel", panel);
    else {
        fill_rect(panel.x, panel.y + 8, panel.w, panel.h - 16, SDL_Color{122, 64, 36, 255});
        draw_rect_outline(panel.x, panel.y + 8, panel.w, panel.h - 16, SDL_Color{230, 180, 78, 255});
        draw_text("BARN", 5 * TILE_SIZE + 14, 1 * TILE_SIZE + 22, SDL_Color{255, 236, 180, 255}, 150);
    }

    tex("barn_feed_trough", 3, 2, TILE_SIZE * 3, TILE_SIZE, 0, 2);
    tex("barn_water_trough", 6, 2, TILE_SIZE * 3, TILE_SIZE, 0, 2);
    tex("barn_hay_bale", 1, 2);
    tex("barn_hay_roll", 1, 3);
    tex("barn_hay_pile", 10, 2);
    tex("barn_hay_low", 10, 3);
    tex("barn_fence_v", 2, 5, TILE_SIZE, TILE_SIZE * 2, 0, 0);
    tex("barn_fence_v", 9, 5, TILE_SIZE, TILE_SIZE * 2, 0, 0);
    tex("barn_fence_h", 4, 8, TILE_SIZE * 2, TILE_SIZE, 0, -8);
    tex("barn_fence_h", 6, 8, TILE_SIZE * 2, TILE_SIZE, 0, -8);
    tex("barn_barrel", 10, 8);
    tex("barn_sack", 9, 9);
    tex("barn_crate", 7, 9);
    tex("barn_crate", 8, 9);
    tex("barn_bucket", 10, 9);

    for (const auto& a : barn.animals()) {
        std::string base;
        const bool adult = a.tile.animal_age >= a.tile.animal_adult_age;
        if (a.tile.animal_type == 1) base = adult ? "chicken" : "baby_chick";
        else if (a.tile.animal_type == 2) base = adult ? "pig" : "baby_pig";
        else if (a.tile.animal_type == 3) base = adult ? "cow" : "baby_cow";
        std::string suffix = "front";
        if (a.facing == 'b') suffix = "back";
        else if (a.facing == 'l') suffix = "left";
        else if (a.facing == 'r') suffix = "right";
        SDL_Rect dst{static_cast<int>(a.visual_x * TILE_SIZE) + 8,
                     static_cast<int>(a.visual_y * TILE_SIZE) + 8,
                     TILE_SIZE - 16,
                     TILE_SIZE - 16};
        draw_texture(base + "_" + suffix, dst);
    }

    Uint32 now = SDL_GetTicks();
    if (last_frame_ticks_ == 0) last_frame_ticks_ = now;
    double dt = static_cast<double>(now - last_frame_ticks_) / 1000.0;
    last_frame_ticks_ = now;
    if (dt <= 0.0 || dt > 0.1) dt = 0.016;

    const double target_x = static_cast<double>(player.get_x());
    const double target_y = static_cast<double>(player.get_y());
    if (!visual_position_initialized_ || std::fabs(visual_x_ - target_x) > 4.0 || std::fabs(visual_y_ - target_y) > 4.0) {
        visual_x_ = target_x;
        visual_y_ = target_y;
        visual_position_initialized_ = true;
    }
    const double speed_tiles_per_second = 10.0;
    auto move_towards = [dt, speed_tiles_per_second](double current, double target) {
        double diff = target - current;
        double step = speed_tiles_per_second * dt;
        if (std::fabs(diff) <= step) return target;
        return current + (diff > 0.0 ? step : -step);
    };
    visual_x_ = move_towards(visual_x_, target_x);
    visual_y_ = move_towards(visual_y_, target_y);

    std::string player_key = "player_front";
    if (player.get_facing() == 'b') player_key = "player_back";
    else if (player.get_facing() == 'l') player_key = "player_left";
    else if (player.get_facing() == 'r') player_key = "player_right";
    SDL_Rect pdst{static_cast<int>(visual_x_ * TILE_SIZE) + 8,
                  static_cast<int>(visual_y_ * TILE_SIZE) + 4,
                  TILE_SIZE - 16,
                  TILE_SIZE - 8};
    draw_texture(player_key, pdst);

    draw_side_panel(player, std::string("[축사] ") + message + "\nWASD 이동 / T 가축 상호작용 / J 축사 관리 / 출구에서 J로 나가기");
    SDL_RenderPresent(renderer_);
}

int GuiRenderer::fishing_hit_width_for_level(int level) const {
    if (level >= 20) return 3;
    if (level >= 10) return 2;
    return 1;
}

void GuiRenderer::draw_fishing_frame(int rod_grade, int pos, int great_start, int great_end, int perfect_pos, bool has_perfect, int bar_width) {
    SDL_SetRenderDrawColor(renderer_, 14, 25, 38, 255);
    SDL_RenderClear(renderer_);
    const int box_x = 120;
    const int box_y = 250;
    const int cell = 34;
    const int bar_h = 54;
    fill_rect(box_x - 22, box_y - 118, bar_width * cell + 44, 270, SDL_Color{38, 45, 58, 240});
    SDL_Rect fish{box_x + 60, box_y - 96, 96, 48}; draw_texture("fish_mackerel", fish);
    SDL_Rect tuna{box_x + 520, box_y - 96, 108, 54}; draw_texture("fish_tuna", tuna);

    std::ostringstream info;
    info << "낚싯대 레벨 " << rod_grade << "  |  ";
    if (has_perfect) info << "완벽 판정: 중앙 1칸 / ";
    else info << "완벽 판정: 구리 낚싯대부터 해금 / ";
    info << "성공 판정: " << great_start << "~" << great_end;
    draw_text("스페이스/엔터/마우스 클릭으로 바늘을 멈추세요.", box_x, box_y - 38, SDL_Color{255, 245, 213, 255}, bar_width * cell);
    draw_text(info.str(), box_x, box_y - 12, SDL_Color{214, 200, 170, 255}, bar_width * cell);

    for (int i = 0; i < bar_width; ++i) {
        SDL_Color c = (i >= great_start && i <= great_end) ? SDL_Color{80, 210, 95, 255} : SDL_Color{204, 198, 170, 255};
        if (has_perfect && i == perfect_pos) c = SDL_Color{255, 196, 61, 255};
        fill_rect(box_x + i * cell, box_y, cell - 2, bar_h, c);
    }
    if (has_perfect) {
        draw_text("P", box_x + perfect_pos * cell + 10, box_y + 14, SDL_Color{50, 32, 20, 255}, cell - 8);
    }
    fill_rect(box_x + pos * cell + 6, box_y - 34, cell - 12, bar_h + 62, SDL_Color{255, 236, 68, 210});

    double guide_ratio = static_cast<double>(great_end - great_start + 1) / static_cast<double>(bar_width);
    draw_bar(box_x, box_y + 78, bar_width * cell, 18, guide_ratio, SDL_Color{91, 181, 255, 255});
    SDL_RenderPresent(renderer_);
}

bool GuiRenderer::play_fishing(Player& player, std::string& message) {
    int stamina_cost = player.get_fishing_stamina_cost();
    if (!confirm("낚시", "스태미나 " + std::to_string(stamina_cost) + "을 사용해 낚시 미니게임을 시작할까요?\n스페이스/엔터/마우스 클릭으로 바늘을 멈춥니다.")) {
        message = "낚시를 취소했습니다.";
        return false;
    }
    if (!player.use_stamina(stamina_cost)) {
        message = "스태미나가 부족합니다. (필요: " + std::to_string(stamina_cost) + ")";
        show_notice("낚시 실패", message);
        return false;
    }

    const int BAR_WIDTH = 20;
    const int center = BAR_WIDTH / 2;
    const int great_radius = player.get_rod_grade() >= 2 ? 3 : 2;
    const int great_start = center - great_radius;
    const int great_end = center + great_radius;
    const bool has_perfect = player.get_rod_grade() > 0;
    const int perfect_pos = center;

    int pos = 0, dir = 1;
    bool stopped = false;
    Uint32 last = SDL_GetTicks();

    while (!stopped) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { message = "낚시가 중단되었습니다."; return false; }
            if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN) stopped = true;
        }
        Uint32 now = SDL_GetTicks();
        if (now - last >= 35) {
            pos += dir;
            if (pos >= BAR_WIDTH - 1) { pos = BAR_WIDTH - 1; dir = -1; }
            else if (pos <= 0) { pos = 0; dir = 1; }
            last = now;
        }
        draw_fishing_frame(player.get_rod_grade(), pos, great_start, great_end, perfect_pos, has_perfect, BAR_WIDTH);
        SDL_Delay(8);
    }

    const bool perfect = has_perfect && pos == perfect_pos;
    const bool great = pos >= great_start && pos <= great_end;
    if (perfect || great) {
        int luck_bonus = 0;
        if (player.has_luk_node(3)) luck_bonus += 10;
        if (player.has_luk_node(6)) luck_bonus += 10;

        std::vector<std::string> rewards;
        auto add_reward = [&](ItemType item, int qty, const std::string& label) {
            if (player.add_item(item, qty)) rewards.push_back(label);
            else rewards.push_back(item_name(item) + " 수령 실패(가방 가득 참)");
        };

        add_reward(ItemType::FISH_MACKEREL, 1, "고등어 1마리");
        if (std::rand() % 100 < 30 + luck_bonus) add_reward(ItemType::FISH_MACKEREL, 1, "고등어 추가 1마리");
        if (std::rand() % 100 < 30 + luck_bonus) add_reward(ItemType::FISH_FLATFISH, 1, "광어 1마리");
        if (std::rand() % 100 < 5 + luck_bonus) add_reward(ItemType::FISH_TUNA, 1, "참치 1마리");
        if (perfect && player.has_luk_node(6) && std::rand() % 100 < 10) {
            add_reward(ItemType::FISH_MACKEREL, 1, "황금 어장: 고등어 1마리");
            add_reward(ItemType::FISH_FLATFISH, 1, "황금 어장: 광어 1마리");
            add_reward(ItemType::FISH_TUNA, 1, "황금 어장: 참치 1마리");
        }

        const int xp = perfect ? 10 : 6;
        player.add_exp(xp);

        std::ostringstream oss;
        oss << (perfect ? "완벽! " : "성공! ");
        for (std::size_t i = 0; i < rewards.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << rewards[i];
        }
        oss << " 획득 / XP +" << xp;
        message = oss.str();
        show_notice(perfect ? "낚시 완벽" : "낚시 성공", message);
        return true;
    }
    message = "앗! 물고기가 도망쳤습니다. XP +3";
    player.add_exp(3);
    show_notice("낚시 실패", message);
    return false;
}

bool GuiRenderer::play_blackjack_mining(Player& player, std::string& message) {
    int stamina_cost = player.get_mining_stamina_cost();
    if (!confirm("블랙잭 광산", "스태미나 " + std::to_string(stamina_cost) + "을 사용해 광산 채굴을 시작할까요?\n붕괴 압력에 닿기 전에 멈춰 탈출하면 채광 횟수에 따라 광석을 얻습니다.")) {
        message = "광산 탐험을 취소했습니다.";
        return false;
    }
    if (!player.use_stamina(stamina_cost)) {
        message = "스태미나가 부족합니다. (필요: " + std::to_string(stamina_cost) + ")";
        show_notice("광산 실패", message);
        return false;
    }

    int collapse_pressure = 16 + std::rand() % 15;
    int pressure = 0;
    int digs = 0;
    bool busted = false;
    bool str_double_reward = false;
    while (true) {
        std::ostringstream oss;
        oss << "현재 붕괴 압력: " << collapse_pressure << "\n"
            << "채광 횟수: " << digs << " / 5\n";
        int margin = collapse_pressure - pressure;
        if (player.get_pick_grade() >= 3) {
            oss << "현재 누적 압력: " << pressure << " (다음 상승 1~10)\n";
        } else if (player.get_pick_grade() >= 2) {
            int low = std::max(0, pressure - (std::rand() % 3));
            oss << "곡괭이 감지 범위: " << low << "~" << (low + 5) << "\n";
        } else if (player.get_pick_grade() >= 1) {
            if (margin >= 11) oss << "곡괭이 힌트: 갱도가 아주 조용합니다.\n";
            else if (margin >= 6) oss << "곡괭이 힌트: 천장에서 흙먼지가 떨어집니다.\n";
            else if (margin >= 1) oss << "곡괭이 힌트: 갱도가 심하게 흔들립니다!\n";
            else oss << "곡괭이 힌트: 붕괴 직전입니다!\n";
        }
        oss << "\n보상은 누적됩니다: 1회 구리 1개, 2회 구리 2개, 3회 철 1개, 4회 철 2개, 5회 금 1개";

        int action = show_choice("블랙잭 광산", oss.str(), {{1, "더 판다"}, {2, "탈출한다"}, {0, "취소"}}, 0);
        if (action == 0) { message = "광산 탐험을 취소했습니다."; return false; }
        if (action == 2) break;

        int rise_max = player.get_pick_grade() >= 3 ? 10 : 11;
        int rise = 1 + std::rand() % rise_max;
        pressure += rise;
        ++digs;
        if (pressure >= collapse_pressure) {
            if (player.has_str_node(6)) {
                pressure -= rise;
                str_double_reward = true;
                show_notice("헤라클레스", "압력 초과! 마지막 채굴을 무효화했습니다.\n현재 누적 압력: " + std::to_string(pressure) + "\n이번 탈출 보상이 2배가 됩니다.");
                break;
            }
            if (player.has_str_node(3) && std::rand() % 100 < 30) {
                --digs;
                show_notice("채굴 집중", "갱도가 무너지려 했지만 힘으로 버텼습니다.\n지금 탈출하면 이전 채광 보상을 지킵니다.");
                break;
            }
            busted = true;
            break;
        }
        if (digs >= 5) break;
    }

    std::ostringstream result;
    bool success = false;
    if (busted) {
        player.add_exp(2);
        result << "광산이 붕괴했습니다. 보상은 없습니다. XP +2";
    } else {
        int multiplier = str_double_reward ? 2 : 1;
        result << "채광 횟수: " << digs << "회\n";
        if (digs <= 0) {
            int exp = 1;
            player.add_exp(exp);
            result << "아무 광석도 캐지 못했습니다. XP +" << exp;
            message = result.str();
            show_notice("블랙잭 광산 결과", message);
            return false;
        }

        int copper_total = 0;
        int iron_total = 0;
        int gold_total = 0;
        int exp_total = 0;
        for (int step = 1; step <= digs && step <= 5; ++step) {
            if (step <= 2) {
                copper_total += step;
                exp_total += 4 * step;
            } else if (step <= 4) {
                iron_total += step - 2;
                exp_total += 6 * step;
            } else {
                gold_total += 1;
                exp_total += 30;
            }
        }

        auto add_cumulative_reward = [&](ItemType item, int qty) {
            if (qty <= 0) return;
            if (player.add_item(item, qty)) {
                if (success) result << ", ";
                result << item_name(item) << " " << qty << "개";
                success = true;
            } else {
                if (success) result << "\n";
                result << "인벤토리 공간이 부족해 " << item_name(item) << "을(를) 받을 수 없습니다.";
            }
        };

        result << "누적 보상: ";
        add_cumulative_reward(ItemType::COPPER, copper_total * multiplier);
        add_cumulative_reward(ItemType::IRON, iron_total * multiplier);
        add_cumulative_reward(ItemType::GOLD, gold_total * multiplier);
        if (success) {
            player.add_exp(exp_total);
            result << " 획득! XP +" << exp_total;
        }
        if (multiplier > 1) result << "\n헤라클레스 보너스로 보상이 2배가 되었습니다.";
        message = result.str();
        show_notice("블랙잭 광산 결과", message);
        return success;
    }

    message = result.str();
    show_notice("블랙잭 광산 결과", message);
    return success;
}
