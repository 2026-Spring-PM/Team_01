#include "Renderer.h"

#include "Tile.h"
#include "Season.h"

#include <iostream>

void Renderer::draw(const Player& player, const Map& map) {
    clear_screen();
    draw_status_bar(player);
    draw_map(player, map);

    std::cout << "📍 위치: (" << player.get_x() << ", " << player.get_y() << ")"
              << "  |  현재: " << current_zone_name(player, map) << "\n";

    draw_message();
    draw_menu();
}

void Renderer::add_message(const std::string& msg) {
    last_message_ = msg;
}

void Renderer::clear_screen() {
    std::cout << "\033[2J\033[H";
}

void Renderer::draw_status_bar(const Player& player) {
    Season season = season_from_day(player.get_day());

    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << " " << player.get_name() << " 농부의 일지\n";
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "🌅 Day " << player.get_day() << " / 100   "
            << season_name(season) << "   "
            << "📈 Lv." << player.get_level();

    int to_next = player.get_exp_to_next();
    if (to_next > 0) {
        std::cout << " (다음까지 " << to_next << " XP)";
    } else {
        std::cout << " (만렙)";
    }
    std::cout << "\n";

    std::cout << "💰 " << player.get_money() << "원   "
              << "⚡ " << player.get_stamina() << "/" << player.get_stamina_max() << "   "
              << "🎒 " << player.inventory().used_slots()
              << "/" << player.inventory().max_slots() << "\n";
    std::cout << "═══════════════════════════════════════════════\n";
}

void Renderer::draw_map(const Player& player, const Map& map) {
    for (int y = 0; y < Map::HEIGHT; ++y) {
        for (int x = 0; x < Map::WIDTH; ++x) {
            if (x == player.get_x() && y == player.get_y()) {
                std::cout << "👤";
            } else {
                std::cout << tile_to_emoji(map.get_tile(x, y).type);
            }
        }
        std::cout << "\n";
    }
}

void Renderer::draw_message() {
    if (last_message_.empty()) {
        return;
    }
    std::cout << "───────────────────────────────────────────────\n";
    std::cout << "💬 " << last_message_ << "\n";
}

void Renderer::draw_menu() {
    std::cout << "───────────────────────────────────────────────\n";
    std::cout << "🎮 W/A/S/D: 이동  |  B: 잠자기  |  I: 가방  |  Q: 종료\n";
    std::cout << "───────────────────────────────────────────────\n";
    std::cout << "> ";
    std::cout.flush();
}

std::string Renderer::current_zone_name(const Player& player, const Map& map) const {
    TileType type = map.get_tile(player.get_x(), player.get_y()).type;
    switch (type) {
        case TileType::WATER:     return "🟦 물가";
        case TileType::SAND:      return "🟨 모래사장";
        case TileType::LIVESTOCK: return "🟫 사육 공간";
        case TileType::FARM:      return "🟩 농업 공간";
        case TileType::PATH:      return "⬜ 마당";
        case TileType::BUILDING:  return "🏡 집";
        case TileType::BED:       return "🛏️  침대 (잠자기 가능!)";
        case TileType::MARKET:    return "🏪 시장";
        case TileType::KITCHEN:   return "🍳 주방";
        case TileType::WORKSHOP:  return "🔨 제작소";
    }
    return "?";
}

std::string Renderer::tile_to_emoji(TileType type) const {
    switch (type) {
        case TileType::WATER:     return "🟦";
        case TileType::SAND:      return "🟨";
        case TileType::LIVESTOCK: return "🟫";
        case TileType::FARM:      return "🟩";
        case TileType::PATH:      return "⬜";
        case TileType::BUILDING:  return "🏡";
        case TileType::BED:       return "🛏️ ";
        case TileType::MARKET:    return "🏪";
        case TileType::KITCHEN:   return "🍳";
        case TileType::WORKSHOP:  return "🔨";
    }
    return "❓";
}

void Renderer::draw_inventory_screen(const Player& player) {
    clear_screen();
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "🎒 가방 (" << player.inventory().used_slots()
              << "/" << player.inventory().max_slots() << ")\n";
    std::cout << "═══════════════════════════════════════════════\n";

    auto items = player.inventory().get_all();
    if (items.empty()) {
        std::cout << "  (텅 비어있어요)\n";
    } else {
        for (const auto& [type, qty] : items) {
            std::cout << "  " << item_name(type) << "  x " << qty << "\n";
        }
    }

    std::cout << "───────────────────────────────────────────────\n";
    std::cout << "아무 키나 누르면 돌아갑니다...\n";
    std::cout << "> ";
    std::cout.flush();
}

void Renderer::draw_ending_screen(const Player& player) {
    clear_screen();
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "    🎉 " << player.get_name() << "이의 100일이 끝났네! 🎉\n";
    std::cout << "═══════════════════════════════════════════════\n\n";
    std::cout << "  허허, 한 해 농사 잘 지었구먼.\n\n";
    std::cout << "  💰 최종 자산:   " << player.get_money() << "원\n";
    std::cout << "  📈 달성 레벨:   Lv." << player.get_level() << "\n";
    std::cout << "  ⚡ 최대 스태미나: " << player.get_stamina_max() << "\n";
    std::cout << "  🎒 보유 아이템:  " << player.inventory().used_slots() << "종류\n\n";
    std::cout << "  마을 사람들이 " << player.get_name() << "이를 그리워할 게야. 🌱\n";
    std::cout << "═══════════════════════════════════════════════\n";
}

void Renderer::draw_title_screen() {
    clear_screen();
    std::cout << "\n";
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "        🌾 팜빌리지 🌾\n";
    std::cout << "═══════════════════════════════════════════════\n\n";
    std::cout << "  허허, 새로 온 양반이로구먼!\n\n";
    std::cout << "  자네, 이름이 뭔가?\n\n";
    std::cout << "  > ";
    std::cout.flush();
}