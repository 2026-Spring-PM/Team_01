#include "Renderer.h"
#include "Season.h"
#include "ItemType.h"

#include <iostream>

void Renderer::draw(const Player& player, const Map& map) {
    clear_screen();
    draw_status_bar(player);
    draw_map(player, map);

    std::cout << "📍 위치: (" << player.get_x() << ", " << player.get_y() << ")"
              << "  |  현재 구역: " << current_zone_name(player, map) << "\n";

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
    std::cout << " 🧑‍🌾 " << player.get_name() << " 농부의 일지\n";
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "🌅 Day " << player.get_day() << " / 100   " << season_name(season)
              << "   📈 Lv." << player.get_level();

    int to_next = player.get_exp_to_next();
    if (to_next > 0) std::cout << " (다음까지 " << to_next << " XP)";
    else std::cout << " (만렙)";
    std::cout << "\n";

    std::cout << "💰 " << player.get_money() << "G   ⚡ " << player.get_stamina() << "/" << player.get_stamina_max()
              << "   🎒 " << player.inventory().used_slots() << "/" << player.inventory().max_slots()
              << "   🛠️ 도구등급: 낚싯대 " << player.get_fishing_rod_grade() << ", 곡괭이 " << player.get_pickaxe_grade() << " , 괭이 " << player.get_hoe_grade();
    if (player.is_sick_today()) std::cout << "   🤧 감기";
    std::cout << "\n";
    std::cout << "═══════════════════════════════════════════════\n";
}

void Renderer::draw_map(const Player& player, const Map& map) {
    for (int y = 0; y < Map::HEIGHT; ++y) {
        for (int x = 0; x < Map::WIDTH; ++x) {
            // 1. 플레이어 위치
            if (x == player.get_x() && y == player.get_y()) {
                std::cout << "🧑‍🌾";
                continue;
            }

            // 2. 광산 2x2 영역 체크
            if (x >= Map::MINE_X && x <= Map::MINE_X + 1 && y >= Map::MINE_Y && y <= Map::MINE_Y + 1) {
                std::cout << "⛏️ ";
                continue;
            }

            const Tile& tile = map.get_tile(x, y);

            // 3. 작물 렌더링
            if (tile.has_crop()) {
                if (tile.crop_progress >= tile.crop_max_progress && tile.crop_harvest_cooldown == 0) {
                    if (tile.crop_id == 1) std::cout << "🥔";
                    else if (tile.crop_id == 2) std::cout << "🥕";
                    else if (tile.crop_id == 3 || tile.crop_id == 4) std::cout << "🌾";
                    else if (tile.crop_id == 5) std::cout << "🍎";
                    else std::cout << "🌾";
                } else {
                    std::cout << "🌱";
                }
                continue;
            }

            // 4. 동물 렌더링
            if (tile.has_animal()) {
                bool adult = tile.animal_age >= tile.animal_adult_age;
                if (adult) {
                    if (tile.animal_type == 1) std::cout << "🐔";
                    else if (tile.animal_type == 2) std::cout << "🐖";
                    else if (tile.animal_type == 3) std::cout << "🐄";
                    else std::cout << "🐾";
                } else {
                    if (tile.animal_type == 1) std::cout << "🐣";
                    else if (tile.animal_type == 2) std::cout << "🐷";
                    else if (tile.animal_type == 3) std::cout << "🐂";
                    else std::cout << "🐾";
                }
                continue;
    
            }

            if (tile.has_wild_tree) {
                std::cout << "🌲";
                continue;
            }

            // 5. 건설된 건물 렌더링
            if (!tile.building_name.empty()) {
                if (tile.building_name == "주방") std::cout << "🍳";
                else if (tile.building_name == "작업장") std::cout << "🔨";
                else if (tile.building_name == "저장소") std::cout << "📦";
                else std::cout << "🏡";
                continue;
            }

            // 6. 기본 지형 (부지 등) 렌더링
            std::cout << tile_to_emoji(tile.type);
        }
        std::cout << "\n";
    }
}

void Renderer::draw_message() {
    if (last_message_.empty()) return;
    std::cout << "───────────────────────────────────────────────\n";
    std::cout << "💬 " << last_message_ << "\n";
}

void Renderer::draw_menu() {
    std::cout << "───────────────────────────────────────────────\n";
    std::cout << "🎮 [W/A/S/D] 이동 | [J] 위치 상호작용 | [E] 가방 | [Q] 종료\n";
    std::cout << "   ※ 잠자기=침대에서 J, 구매/판매=상점에서 J, 작업장=건축 부지에서 건설 후 이용\n";
    std::cout << "───────────────────────────────────────────────\n";
    std::cout << "> " << std::flush;
}

std::string Renderer::current_zone_name(const Player& player, const Map& map) const {
    if (player.get_x() >= Map::MINE_X && player.get_x() <= Map::MINE_X + 1 && 
        player.get_y() >= Map::MINE_Y && player.get_y() <= Map::MINE_Y + 1) return "⛏️ 보물 광산지대";
    const Tile& current_tile = map.get_tile(player.get_x(), player.get_y());
    if (current_tile.building_name == "주방") return "🍳 요리 주방";
    if (current_tile.building_name == "작업장") return "🔨 작업장";
    if (current_tile.building_name == "저장소") return "📦 저장소";

    TileType type = current_tile.type;
    switch (type) {
        case TileType::WATER:     return "🟦 물가";
        case TileType::SAND:      return "🟨 강변 낚시터";
        case TileType::LIVESTOCK: return "🟫 가축 사육장";
        case TileType::FARM:      return "🟩 비옥한 농경지";
        case TileType::PATH:      return "⬜ 자갈 마당";
        
        case TileType::BED:       return "🛏️ 침대";
        case TileType::MARKET:    return "🏪 피에르 잡화점";

        case TileType::KITCHEN:   return "🍳 주방 건축 부지";
        case TileType::WORKSHOP:  return "🔨 작업장 건축 부지";
        case TileType::STORAGE:   return "🏡 저장소 건축 부지";
    }
    return "?";
}

std::string Renderer::tile_to_emoji(TileType type) const {
    switch (type) {
        case TileType::WATER:     return "🟦";
        case TileType::LIVESTOCK: return "🟫";
        case TileType::FARM:      return "🟩";
        case TileType::PATH:      return "⬜";
        case TileType::SAND:      return "🟨";

        case TileType::BED:       return "🛏️ ";
        case TileType::MARKET:    return "🏪";
        case TileType::KITCHEN:   return "🍳";
        case TileType::WORKSHOP:  return "🔨";

        case TileType::STORAGE:   return "🏡";
    }
    return "❓";
}

void Renderer::draw_inventory_screen(const Player& player) {
    clear_screen();
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "🎒 가방 현황 - 총 아이템 수 (" << player.inventory().used_slots() << "/" << player.inventory().max_slots() << ")\n";
    std::cout << "═══════════════════════════════════════════════\n";

    auto items = player.inventory().get_all();
    if (items.empty()) {
        std::cout << "  (가방이 비어 있습니다.)\n";
    } else {
        for (const auto& pair : items) {
            std::cout << "  " << item_name(pair.first) << " x " << pair.second << "개\n";
        }
    }
    std::cout << "───────────────────────────────────────────────\n";
    std::cout << "[아무 키나 누르면 돌아갑니다]\n> " << std::flush;
}

void Renderer::draw_ending_screen(const Player& player) {
    clear_screen();
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "    🎉 " << player.get_name() << " 농부의 100일 귀농 일지 완료! 🎉\n";
    std::cout << "═══════════════════════════════════════════════\n\n";
    std::cout << "  💰 최종 골드: " << player.get_money() << "G\n";
    std::cout << "  📈 최종 레벨: Lv." << player.get_level() << "\n";
    std::cout << "  ⚡ 최대 스태미나: " << player.get_true_stamina_max() << "\n";
    std::cout << "  🎒 최종 보유 아이템 수: " << player.inventory().used_slots() << "개\n\n";
    std::cout << "═══════════════════════════════════════════════\n";
}

void Renderer::draw_title_screen() {
    clear_screen();
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "        🌾 통합 팜 빌리지 시뮬레이터 🌾\n";
    std::cout << "═══════════════════════════════════════════════\n\n";
    std::cout << "  새로 귀농한 농부님의 이름을 입력해 주세요.\n\n";
    std::cout << "  > " << std::flush;
}
