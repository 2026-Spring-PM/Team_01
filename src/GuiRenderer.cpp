#include "GuiRenderer.h"
#include "ItemType.h"
#include "Season.h"

#include <SDL.h>

#include <SDL_image.h>

#include <SDL_ttf.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdlib>
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
    for (auto& pair : textures_) SDL_DestroyTexture(pair.second);
    textures_.clear();
    if (font_) TTF_CloseFont(font_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

bool GuiRenderer::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) return false;
    if (TTF_Init() != 0) return false;
    int img_flags = IMG_INIT_PNG;
    if ((IMG_Init(img_flags) & img_flags) != img_flags) return false;

    window_ = SDL_CreateWindow("Farm Village",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               WINDOW_WIDTH,
                               WINDOW_HEIGHT,
                               SDL_WINDOW_SHOWN);
    if (!window_) return false;

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer_) return false;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    load_font();

    const std::string base = "assets/sprites/";
    const std::pair<std::string, std::string> entries[] = {
        {"player_front", "player_front.png"}, {"player_back", "player_back.png"},
        {"player_left", "player_left.png"}, {"player_right", "player_right.png"},
        {"water", "tile_water.png"}, {"sand", "tile_sand.png"}, {"livestock_tile", "tile_livestock.png"},
        {"farm_tile", "tile_farm.png"}, {"building_tile", "tile_building.png"}, {"path", "tile_path.png"},
        {"bed", "bed.png"}, {"market", "market.png"}, {"kitchen", "kitchen.png"}, {"workshop", "workshop.png"},
        {"build_plot", "build_plot.png"}, {"storage", "chest.png"}, {"mine", "mine.png"},
        {"sprout", "crop_sprout.png"}, {"potato", "crop_potato.png"}, {"carrot", "crop_carrot.png"},
        {"wheat", "crop_wheat.png"}, {"rice", "crop_rice.png"}, {"apple_sapling", "crop_apple_sapling.png"},
        {"apple_tree", "crop_apple_tree.png"}, {"apple_empty", "crop_apple_empty.png"}, {"watered", "crop_watered.png"},
        {"dead", "crop_dead.png"},
        {"baby_chick", "baby_chick.png"}, {"chicken", "chicken.png"}, {"baby_pig", "baby_pig.png"},
        {"pig", "pig.png"}, {"baby_cow", "baby_cow.png"}, {"cow", "cow.png"},
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
        {"food_bread", "food_bread.png"}, {"food_grilled_fish", "food_grilled_fish.png"}, {"food_apple_pie", "food_apple_pie.png"}
    };

    for (const auto& e : entries) {
        if (!load_texture(e.first, base + e.second)) return false;
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
        case ItemType::BREAD: return "food_bread";
        case ItemType::GRILLED_FISH: return "food_grilled_fish";
        case ItemType::APPLE_PIE: return "food_apple_pie";
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
            draw_text(line, panel_x + margin, y, SDL_Color{238, 230, 207, 255}, panel_w - margin * 2);
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
                if (event.key.keysym.sym == SDLK_ESCAPE) return cancel_id;
                if (event.key.keysym.sym == SDLK_RETURN && !buttons.empty()) return buttons.front().first;
                if (event.key.keysym.sym == SDLK_0) return cancel_id;
                int idx = -1;
                if (event.key.keysym.sym >= SDLK_1 && event.key.keysym.sym <= SDLK_9) idx = event.key.keysym.sym - SDLK_1;
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
        header << "Gold " << player.get_money() << "G  |  Stamina " << player.get_stamina() << "/" << player.get_stamina_max()
               << "  |  Lv." << player.get_level() << "  |  Day " << player.get_day()
               << "  |  Inventory " << player.inventory().used_slots() << "/" << player.inventory().max_slots();
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
        summary << "Gold " << player.get_money() << "G  |  Stamina " << player.get_stamina() << "/" << player.get_stamina_max()
                << "  |  Lv." << player.get_level() << "  |  Day " << player.get_day();
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
                if (key == SDLK_ESCAPE || key == SDLK_0) return cancel_id;
                if ((key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE) && selected_index >= 0 && selected_index < static_cast<int>(entries.size())) {
                    if (!entries[static_cast<std::size_t>(selected_index)].disabled) return entries[static_cast<std::size_t>(selected_index)].id;
                    break;
                }
                if ((key == SDLK_s || key == SDLK_DOWN) && !entries.empty()) {
                    selected_index = std::min(static_cast<int>(entries.size()) - 1, selected_index + 1);
                    int item_top = selected_index * row_h;
                    if (item_top + row_h > scroll_y + content_h) scroll_y = item_top + row_h - content_h;
                    scroll_y = std::max(0, std::min(scroll_y, max_scroll));
                    break;
                }
                if ((key == SDLK_w || key == SDLK_UP) && !entries.empty()) {
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

    const int panel_w = 1040;
    const int panel_h = 680;
    const int panel_x = (WINDOW_WIDTH - panel_w) / 2;
    const int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
    const int margin = 26;
    const int left_w = 520;
    const int right_w = panel_w - margin * 2 - left_w - 24;
    const int right_x = panel_x + margin + left_w + 24;
    const int list_y = panel_y + 178;
    const int row_h = 68;
    int selected = 0;

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
        for (const auto& line : wrap_text(status_text, 52)) {
            draw_text(line, panel_x + margin, sy, SDL_Color{238, 230, 207, 255}, left_w);
            sy += 24;
            if (sy > panel_y + 154) break;
        }

        fill_rect(panel_x + margin, list_y - 14, left_w, panel_h - (list_y - panel_y) - margin, SDL_Color{31, 26, 20, 230});
        draw_rect_outline(panel_x + margin, list_y - 14, left_w, panel_h - (list_y - panel_y) - margin, SDL_Color{128, 93, 48, 255});
        draw_text("선택 목록 (W/S, Enter, 마우스)", panel_x + margin + 16, list_y - 6, SDL_Color{255, 221, 132, 255}, left_w - 32);

        for (std::size_t i = 0; i < actions.size(); ++i) {
            int y = list_y + 34 + static_cast<int>(i) * row_h;
            if (y + row_h > panel_y + panel_h - margin - 52) break;
            bool is_sel = static_cast<int>(i) == selected;
            SDL_Color base = actions[i].disabled ? SDL_Color{65, 58, 50, 220} : SDL_Color{73, 54, 35, 235};
            if (is_sel) base = actions[i].disabled ? SDL_Color{88, 78, 63, 235} : SDL_Color{118, 78, 38, 245};
            int x = panel_x + margin + 14;
            int w = left_w - 28;
            fill_rect(x, y, w, row_h - 8, base);
            draw_rect_outline(x, y, w, row_h - 8, is_sel ? SDL_Color{255, 230, 120, 255} : SDL_Color{232, 190, 95, 255});
            if (!actions[i].icon_key.empty()) { SDL_Rect icon{x + 12, y + 10, 42, 42}; draw_texture(actions[i].icon_key, icon); }
            int tx = x + (actions[i].icon_key.empty() ? 16 : 66);
            draw_text(actions[i].title, tx, y + 11, actions[i].disabled ? SDL_Color{180, 170, 150, 255} : SDL_Color{255, 245, 213, 255}, w - (tx - x) - 20);
            if (!actions[i].subtitle.empty()) draw_text(actions[i].subtitle, tx, y + 38, SDL_Color{214, 200, 170, 255}, w - (tx - x) - 20);
            if (actions[i].disabled) draw_locked_overlay(x, y, w, row_h - 8);
        }

        fill_rect(right_x, panel_y + 62, right_w, 270, SDL_Color{31, 26, 20, 230});
        draw_rect_outline(right_x, panel_y + 62, right_w, 270, SDL_Color{128, 93, 48, 255});
        draw_text(recipe_title, right_x + 16, panel_y + 74, SDL_Color{255, 221, 132, 255}, right_w - 32);
        int ry = panel_y + 112;
        for (const auto& r : recipes) {
            if (ry > panel_y + 300) break;
            if (!r.icon_key.empty()) { SDL_Rect icon{right_x + 16, ry, 38, 38}; draw_texture(r.icon_key, icon); }
            draw_text(r.title, right_x + 64, ry + 2, r.disabled ? SDL_Color{180, 170, 150, 255} : SDL_Color{255, 245, 213, 255}, right_w - 82);
            draw_text(r.subtitle, right_x + 64, ry + 25, SDL_Color{214, 200, 170, 255}, right_w - 82);
            if (r.disabled) draw_locked_overlay(right_x + 10, ry - 4, right_w - 20, 48);
            ry += 54;
        }

        fill_rect(right_x, panel_y + 354, right_w, 170, SDL_Color{31, 26, 20, 230});
        draw_rect_outline(right_x, panel_y + 354, right_w, 170, SDL_Color{128, 93, 48, 255});
        draw_text("안내", right_x + 16, panel_y + 366, SDL_Color{255, 221, 132, 255}, right_w - 32);
        draw_text("조건이 부족한 항목은 쇠사슬처럼 잠겨 선택할 수 없습니다. 조합식은 이 영역에서만 확인하세요.", right_x + 16, panel_y + 402, SDL_Color{238, 230, 207, 255}, right_w - 32);

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
                if (key == SDLK_ESCAPE || key == SDLK_0) return cancel_id;
                if ((key == SDLK_s || key == SDLK_DOWN) && !actions.empty()) {
                    selected = (selected + 1) % static_cast<int>(actions.size());
                    break;
                }
                if ((key == SDLK_w || key == SDLK_UP) && !actions.empty()) {
                    selected = (selected - 1 + static_cast<int>(actions.size())) % static_cast<int>(actions.size());
                    break;
                }
                if ((key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE) && selected >= 0 && selected < static_cast<int>(actions.size())) {
                    if (!actions[selected].disabled) return actions[selected].id;
                    break;
                }
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                SDL_Point pt{event.button.x, event.button.y};
                SDL_Rect close_rect{right_x + right_w - 150, panel_y + panel_h - margin - 46, 150, 46};
                if (SDL_PointInRect(&pt, &close_rect)) return cancel_id;
                for (std::size_t i = 0; i < actions.size(); ++i) {
                    int y = list_y + 34 + static_cast<int>(i) * row_h;
                    SDL_Rect r{panel_x + margin + 14, y, left_w - 28, row_h - 8};
                    if (SDL_PointInRect(&pt, &r)) {
                        selected = static_cast<int>(i);
                        if (!actions[i].disabled) return actions[i].id;
                        break;
                    }
                }
            }
            break;
        }
    }
}

int GuiRenderer::show_lotto_number_picker(const std::set<int>& chosen, int cancel_id) {
    if (!ready_ || !window_ || !renderer_) return cancel_id;
    const int panel_w = 640;
    const int panel_h = 500;
    const int panel_x = (WINDOW_WIDTH - panel_w) / 2;
    const int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
    const int btn_w = 86;
    const int btn_h = 58;
    const int start_x = panel_x + 68;
    const int start_y = panel_y + 190;
    auto draw_panel = [&]() {
        fill_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_Color{0, 0, 0, 120});
        fill_rect(panel_x, panel_y, panel_w, panel_h, SDL_Color{44, 34, 26, 250});
        draw_rect_outline(panel_x, panel_y, panel_w, panel_h, SDL_Color{224, 184, 97, 255});
        draw_text("로또 광산 번호 선택", panel_x + 28, panel_y + 22, SDL_Color{255, 221, 132, 255}, panel_w - 56);
        std::ostringstream oss;
        oss << "선택한 번호: ";
        for (int n : chosen) oss << n << " ";
        oss << "\n" << (6 - static_cast<int>(chosen.size())) << "개 더 선택하세요. 키보드 숫자와 입력 번호가 일치합니다. 나가기는 ESC만 사용합니다.";
        int y = panel_y + 72;
        for (const auto& line : wrap_text(oss.str(), 48)) {
            draw_text(line, panel_x + 28, y, SDL_Color{238, 230, 207, 255}, panel_w - 56);
            y += 25;
        }
        for (int n = 0; n <= 9; ++n) {
            int col = n % 5;
            int row = n / 5;
            int x = start_x + col * (btn_w + 20);
            int by = start_y + row * (btn_h + 22);
            bool used = chosen.count(n) > 0;
            fill_rect(x, by, btn_w, btn_h, used ? SDL_Color{65, 58, 50, 220} : SDL_Color{112, 84, 44, 255});
            draw_rect_outline(x, by, btn_w, btn_h, used ? SDL_Color{105, 95, 82, 255} : SDL_Color{232, 190, 95, 255});
            draw_text(std::to_string(n), x + 34, by + 17, used ? SDL_Color{160, 150, 135, 255} : SDL_Color{255, 245, 213, 255});
            if (used) {
                SDL_SetRenderDrawColor(renderer_, 150, 150, 150, 220);
                SDL_RenderDrawLine(renderer_, x + 8, by + 8, x + btn_w - 8, by + btn_h - 8);
                SDL_RenderDrawLine(renderer_, x + btn_w - 8, by + 8, x + 8, by + btn_h - 8);
            }
        }
        draw_text("ESC: 취소", panel_x + 28, panel_y + panel_h - 56, SDL_Color{214, 200, 170, 255}, panel_w - 56);
        SDL_RenderPresent(renderer_);
    };
    while (true) {
        draw_panel();
        SDL_Event event;
        while (SDL_WaitEvent(&event)) {
            if (event.type == SDL_QUIT) return cancel_id;
            if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_ESCAPE) return cancel_id;
                int n = -1;
                if (key >= SDLK_0 && key <= SDLK_9) n = key - SDLK_0;
                else if (key >= SDLK_KP_0 && key <= SDLK_KP_9) n = key - SDLK_KP_0;
                if (n >= 0 && n <= 9 && !chosen.count(n)) return n;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                SDL_Point pt{event.button.x, event.button.y};
                for (int n = 0; n <= 9; ++n) {
                    int col = n % 5;
                    int row = n / 5;
                    SDL_Rect r{start_x + col * (btn_w + 20), start_y + row * (btn_h + 22), btn_w, btn_h};
                    if (SDL_PointInRect(&pt, &r) && !chosen.count(n)) return n;
                }
            }
            break;
        }
    }
}

bool GuiRenderer::poll_input(char& input) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { input = 'q'; return true; }
        if (event.type == SDL_KEYDOWN && !event.key.repeat) {
            switch (event.key.keysym.sym) {
                case SDLK_j: case SDLK_RETURN: case SDLK_SPACE: input = 'j'; return true;
                case SDLK_e: input = 'e'; return true;
                case SDLK_i: input = 'i'; return true;
                case SDLK_u: input = 'u'; return true;
                case SDLK_5: input = '5'; return true;
                case SDLK_q: case SDLK_ESCAPE: input = 'q'; return true;
                default: break;
            }
        }
    }

    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    Uint32 now = SDL_GetTicks();
    const Uint32 move_interval_ms = 145;
    if (now - last_move_event_ticks_ < move_interval_ms) return false;

    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) { input = 'w'; last_move_event_ticks_ = now; return true; }
    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) { input = 's'; last_move_event_ticks_ = now; return true; }
    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) { input = 'a'; last_move_event_ticks_ = now; return true; }
    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) { input = 'd'; last_move_event_ticks_ = now; return true; }
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
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/nanum/NanumGothic.ttf",
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
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font_, safe_text.c_str(), color);
    if (!surface) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer_, surface);
    if (!tex) { SDL_FreeSurface(surface); return; }
    SDL_Rect dst{x, y, surface->w, surface->h};
    if (max_width > 0 && dst.w > max_width) {
        double scale = static_cast<double>(max_width) / std::max(1, dst.w);
        dst.w = max_width;
        dst.h = std::max(1, static_cast<int>(dst.h * scale));
    }
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer_, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
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
        case TileType::PATH: default: draw_texture("path", dst); break;
    }
}

void GuiRenderer::draw_tile_object(const Tile& tile, int x, int y) {
    SDL_Rect dst{x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};

    if (!tile.building_name.empty()) {
        if (tile.building_name == "주방") draw_texture("kitchen", dst);
        else if (tile.building_name == "작업장") draw_texture("workshop", dst);
        else if (tile.building_name == "저장소") draw_texture("storage", dst);
        else draw_texture("build_plot", dst);
        return;
    }

    switch (tile.type) {
        case TileType::BED: draw_texture("bed", dst); return;
        case TileType::MARKET: draw_texture("market", dst); return;
        case TileType::KITCHEN: draw_texture("kitchen", dst); return;
        case TileType::WORKSHOP: draw_texture("workshop", dst); return;
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

    draw_text("Farm Village", ox + 34, 28, SDL_Color{255, 221, 132, 255}, PANEL_WIDTH - 68);
    draw_text("Day / Season", ox + 34, 72, SDL_Color{214, 197, 166, 255});
    std::ostringstream day_season;
    day_season << player.get_day() << "  " << season_name(season_from_day(player.get_day()));
    draw_text(day_season.str(), ox + 150, 72, SDL_Color{255, 245, 213, 255});

    SDL_Rect coin{ox + 28, 108, 34, 34}; draw_texture("coin", coin);
    draw_text("Gold", ox + 72, 112, SDL_Color{214, 197, 166, 255});
    draw_text(std::to_string(player.get_money()) + " G", ox + 150, 112, SDL_Color{255, 245, 213, 255});

    SDL_Rect level{ox + 28, 154, 34, 34}; draw_texture("level", level);
    draw_text("Level", ox + 72, 158, SDL_Color{214, 197, 166, 255});
    draw_text("Lv." + std::to_string(player.get_level()), ox + 150, 158, SDL_Color{255, 245, 213, 255});

    SDL_Rect energy{ox + 28, 204, 34, 34}; draw_texture("energy", energy);
    std::ostringstream stamina_text;
    stamina_text << "Stamina  " << player.get_stamina() << "/" << player.get_stamina_max();
    draw_text(stamina_text.str(), ox + 72, 200, SDL_Color{255, 245, 213, 255}, 190);
    draw_bar(ox + 72, 226, 190, 20,
             static_cast<double>(player.get_stamina()) / std::max(1, player.get_stamina_max()),
             SDL_Color{244, 190, 61, 255});

    SDL_Rect xp_icon{ox + 28, 264, 34, 34}; draw_texture("xp", xp_icon);
    int exp_to_next = player.get_exp_to_next();
    std::ostringstream xp_text;
    xp_text << "XP  " << player.get_exp() << " / " << (player.get_exp() + exp_to_next);
    draw_text(xp_text.str(), ox + 72, 260, SDL_Color{255, 245, 213, 255}, 190);
    draw_bar(ox + 72, 286, 190, 20,
             exp_to_next <= 0 ? 1.0 : static_cast<double>(player.get_exp()) / std::max(1, player.get_exp() + exp_to_next),
             SDL_Color{91, 181, 255, 255});

    SDL_Rect bag{ox + 28, 324, 34, 34}; draw_texture("bag", bag);
    std::ostringstream inv_text;
    inv_text << "Inventory  " << player.inventory().used_slots() << "/" << player.inventory().max_slots();
    draw_text(inv_text.str(), ox + 72, 328, SDL_Color{255, 245, 213, 255}, 190);
    draw_bar(ox + 72, 354, 190, 18,
             static_cast<double>(player.inventory().used_slots()) / std::max(1, player.inventory().max_slots()),
             SDL_Color{117, 199, 121, 255});

    draw_text("Events", ox + 34, 400, SDL_Color{255, 221, 132, 255});
    bool any_event = false;
    if (player.is_sick_today()) {
        SDL_Rect cold{ox + 34, 436, 42, 42}; draw_texture("cold", cold);
        draw_text("감기", ox + 86, 446, SDL_Color{238, 230, 207, 255});
        any_event = true;
    }
    if (message.find("가뭄") != std::string::npos) {
        SDL_Rect drought{ox + 34, any_event ? 486 : 436, 42, 42}; draw_texture("drought", drought);
        draw_text("가뭄", ox + 86, any_event ? 496 : 446, SDL_Color{238, 230, 207, 255});
        any_event = true;
    }
    if (!any_event) draw_text("없음", ox + 34, 436, SDL_Color{188, 176, 150, 255});

    fill_rect(ox + 24, WINDOW_HEIGHT - 156, PANEL_WIDTH - 48, 126, SDL_Color{30, 24, 20, 225});
    draw_rect_outline(ox + 24, WINDOW_HEIGHT - 156, PANEL_WIDTH - 48, 126, SDL_Color{122, 94, 54, 255});
    draw_text("Message", ox + 36, WINDOW_HEIGHT - 144, SDL_Color{255, 221, 132, 255});
    int y = WINDOW_HEIGHT - 112;
    for (const auto& line : wrap_text(message.empty() ? "WASD 이동 / J 상호작용 / E 인벤토리" : message, 28)) {
        draw_text(line, ox + 36, y, SDL_Color{238, 230, 207, 255}, PANEL_WIDTH - 72);
        y += 22;
        if (y > WINDOW_HEIGHT - 44) break;
    }
}

void GuiRenderer::update_window_title(const Player& player, const std::string& message) {
    (void)player;
    (void)message;
    SDL_SetWindowTitle(window_, "Farm Village");
}

void GuiRenderer::draw(const Player& player, const Map& map, const std::string& message) {
    if (!ready_) return;
    SDL_SetRenderDrawColor(renderer_, 18, 24, 20, 255);
    SDL_RenderClear(renderer_);

    for (int y = 0; y < Map::HEIGHT; ++y) {
        for (int x = 0; x < Map::WIDTH; ++x) {
            const Tile& tile = map.get_tile(x, y);
            draw_tile_base(tile, x, y);
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
    update_window_title(player, message);
    SDL_RenderPresent(renderer_);
}

int GuiRenderer::fishing_hit_width_for_level(int level) const {
    if (level >= 20) return 3;
    if (level >= 10) return 2;
    return 1;
}

void GuiRenderer::draw_fishing_frame(int level, int pos, int hit_start, int hit_end, int bar_width) {
    SDL_SetRenderDrawColor(renderer_, 14, 25, 38, 255);
    SDL_RenderClear(renderer_);
    const int box_x = 120;
    const int box_y = 250;
    const int cell = 34;
    const int bar_h = 54;
    fill_rect(box_x - 22, box_y - 90, bar_width * cell + 44, 220, SDL_Color{38, 45, 58, 240});
    SDL_Rect fish{box_x + 60, box_y - 72, 96, 48}; draw_texture("fish_mackerel", fish);
    SDL_Rect tuna{box_x + 520, box_y - 72, 108, 54}; draw_texture("fish_tuna", tuna);
    for (int i = 0; i < bar_width; ++i) {
        SDL_Color c = (i >= hit_start && i <= hit_end) ? SDL_Color{80, 210, 95, 255} : SDL_Color{204, 198, 170, 255};
        fill_rect(box_x + i * cell, box_y, cell - 2, bar_h, c);
    }
    fill_rect(box_x + pos * cell + 6, box_y - 34, cell - 12, bar_h + 62, SDL_Color{255, 236, 68, 210});
    draw_bar(box_x, box_y + 78, bar_width * cell, 18, static_cast<double>(fishing_hit_width_for_level(level)) / 3.0,
             SDL_Color{91, 181, 255, 255});
    SDL_RenderPresent(renderer_);
}

bool GuiRenderer::play_fishing(Player& player, std::string& message) {
    if (!confirm("낚시", "스태미나 30을 사용해 낚시 미니게임을 시작할까요?\n스페이스/엔터/마우스 클릭으로 바늘을 멈춥니다.")) {
        message = "낚시를 취소했습니다.";
        return false;
    }
    if (!player.use_stamina(30)) {
        message = "스태미나가 부족합니다. (필요: 30)";
        show_notice("낚시 실패", message);
        return false;
    }

    const int BAR_WIDTH = 20;
    int pos = 0, dir = 1;
    const int hit_width = fishing_hit_width_for_level(player.get_level());
    const int hit_start = BAR_WIDTH / 2 - hit_width / 2;
    const int hit_end = hit_start + hit_width - 1;
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
        draw_fishing_frame(player.get_level(), pos, hit_start, hit_end, BAR_WIDTH);
        SDL_Delay(8);
    }

    if (pos >= hit_start && pos <= hit_end) {
        int dice = std::rand() % 100;
        if (dice < 70) { player.add_item(ItemType::FISH_MACKEREL, 1); message = "고등어를 낚았습니다! XP +60"; }
        else if (dice < 95) { player.add_item(ItemType::FISH_FLATFISH, 1); message = "광어를 낚았습니다! XP +60"; }
        else { player.add_item(ItemType::FISH_TUNA, 1); message = "참치를 낚았습니다! XP +60"; }
        player.add_exp(60);
        show_notice("낚시 성공", message);
        return true;
    }
    message = "앗! 물고기가 도망쳤습니다.";
    show_notice("낚시 실패", message);
    return false;
}

bool GuiRenderer::play_lotto_mining(Player& player, std::string& message) {
    if (!confirm("로또 광산", "스태미나 30을 사용해 로또 채굴을 시작할까요?\n0~9 중 서로 다른 번호 6개를 선택합니다. 키보드 숫자와 실제 번호가 그대로 일치합니다. 취소는 ESC입니다.")) {
        message = "광산 탐험을 취소했습니다.";
        return false;
    }
    if (!player.use_stamina(30)) {
        message = "스태미나가 부족합니다. (필요: 30)";
        show_notice("광산 실패", message);
        return false;
    }

    std::set<int> chosen;
    while (chosen.size() < 6) {
        std::ostringstream oss;
        oss << "선택한 번호: ";
        for (int n : chosen) oss << n << " ";
        oss << "\n" << (6 - chosen.size()) << "개 더 선택하세요.";
        int picked = show_lotto_number_picker(chosen, -1);
        if (picked < 0) { message = "번호 선택을 취소했습니다."; return false; }
        chosen.insert(picked);
    }

    std::set<int> win_set;
    while (win_set.size() < 6) win_set.insert(std::rand() % 10);
    int match = 0;
    for (int n : chosen) if (win_set.count(n)) ++match;

    std::ostringstream result;
    result << "내 번호: "; for (int n : chosen) result << n << " ";
    result << "\n당첨 번호: "; for (int n : win_set) result << n << " ";
    result << "\n일치 개수: " << match << "개\n";

    if (match == 3) { player.add_item(ItemType::COPPER, 1); player.add_exp(150); result << "구리 광석 1개 획득! XP +150"; }
    else if (match == 4) { player.add_item(ItemType::IRON, 1); player.add_exp(200); result << "철 광석 1개 획득! XP +200"; }
    else if (match == 5) { player.add_item(ItemType::GOLD, 1); player.add_exp(250); result << "금 광석 1개 획득! XP +250"; }
    else if (match == 6) { player.add_item(ItemType::COPPER, 1); player.add_item(ItemType::IRON, 1); player.add_item(ItemType::GOLD, 1); player.add_exp(300); result << "잭팟! 구리/철/금 광석 획득! XP +300"; }
    else result << "아쉽게도 광맥을 찾지 못했습니다.";

    message = result.str();
    show_notice("로또 광산 결과", message);
    return match >= 3;
}
