#include "Game.h"
#include "Input.h"
#include "CropSystem.h"
#include "LivestockSystem.h"
#include "MiniGames.h"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

Game::Game()
    : map_(),
      player_(),
      renderer_(),
      market_(),
      kitchen_(),
      workshop_(),
      event_system_(),
      is_running_(true) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

void Game::run() {
    renderer_.draw_title_screen();
    std::string player_name;
    std::getline(std::cin, player_name);
    if (player_name.empty()) player_name = "초보 농부";
    player_.set_name(player_name);

    player_.add_item(ItemType::SEED_POTATO, 3);
    player_.add_item(ItemType::BABY_CHICKEN, 1);
    player_.add_item(ItemType::COPPER, 2);
    player_.add_item(ItemType::IRON, 1);

    renderer_.add_message("허허, " + player_name + "이로구먼! 감자 씨앗 3개와 새끼 닭 1마리를 지급했네. 🌱");

#ifdef FARM_GUI
    gui_mode_ = gui_renderer_.initialize();
    if (gui_mode_) {
        renderer_.add_message("GUI 모드가 시작되었습니다. WASD/방향키 이동, J/Enter/Space 상호작용.");
        while (is_running_) {
            gui_renderer_.draw(player_, map_, renderer_.last_message());
            char input = 0;
            if (gui_renderer_.poll_input(input)) {
                handle_input(input);
                check_game_end();
            }
            SDL_Delay(16);
        }
        return;
    }
    renderer_.add_message("GUI 초기화에 실패하여 터미널 모드로 실행합니다. DISPLAY와 assets/sprites 경로를 확인하세요.");
#endif

    while (is_running_) {
        renderer_.draw(player_, map_);
        char input = getch();
        handle_input(input);
        check_game_end();
    }
}

void Game::handle_input(char input) {
    switch (input) {
        case 'w': case 'W':
            if (!player_.try_move(0, -1, map_)) renderer_.add_message("그쪽으로는 이동할 수 없습니다.");
            break;
        case 's': case 'S':
            if (!player_.try_move(0, 1, map_)) renderer_.add_message("그쪽으로는 이동할 수 없습니다.");
            break;
        case 'a': case 'A':
            if (!player_.try_move(-1, 0, map_)) renderer_.add_message("그쪽으로는 이동할 수 없습니다.");
            break;
        case 'd': case 'D':
            if (!player_.try_move(1, 0, map_)) renderer_.add_message("그쪽으로는 이동할 수 없습니다.");
            break;
        case 'j': case 'J':
            handle_action();
            break;
        case 'e': case 'E':
            show_inventory();
            break;
        case 'i': case 'I':
            renderer_.add_message("구매는 피에르 상점(🏪) 위치에서 J키로만 이용할 수 있습니다.");
            break;
        case 'u': case 'U':
            renderer_.add_message("판매는 피에르 상점(🏪) 위치에서 J키로만 이용할 수 있습니다.");
            break;
        case '5':
            renderer_.add_message("잠자기는 침대(🛏️) 위치에서 J키로만 할 수 있습니다.");
            break;
        case 'q': case 'Q':
            quit();
            break;
        case 'p': case 'P':
            show_skill_tree();
            break;
        default:
            renderer_.add_message("존재하지 않는 명령어입니다.");
            break;
    }

    if (!player_.get_levelup_message().empty()) {
        renderer_.add_message(player_.get_levelup_message());
        player_.clear_levelup_message();
    }
}

void Game::handle_action() {
    int px = player_.get_x();
    int py = player_.get_y();
    Tile& tile = map_.get_tile(px, py);

    if (tile.building_name == "광산") {
#ifdef FARM_GUI
        if (gui_mode_) {
            std::string msg;
            gui_renderer_.play_lotto_mining(player_, msg);
            renderer_.add_message(msg.empty() ? "광산 탐험을 마쳤습니다." : msg);
            return;
        }
#endif
        
        MiniGames::play_lotto_mining(player_);
        renderer_.add_message("광산 탐험을 마쳤습니다.");
        return;
    }

    switch (tile.type) {
        case TileType::SAND:
        case TileType::WATER:
#ifdef FARM_GUI
            if (gui_mode_) {
                std::string msg;
                gui_renderer_.play_fishing(player_, msg);
                renderer_.add_message(msg.empty() ? "낚시를 마쳤습니다." : msg);
                break;
            }
#endif
            
            MiniGames::play_fishing(player_);
            renderer_.add_message("낚시를 마쳤습니다.");
            break;
        case TileType::FARM:
            handle_farm(tile);
            break;
        case TileType::LIVESTOCK:
            handle_livestock(tile);
            break;
        case TileType::BED:
#ifdef FARM_GUI
            if (gui_mode_) {
                if (gui_renderer_.confirm("잠자기", "오늘 하루를 마치고 잠을 주무시겠습니까?")) advance_day();
                else renderer_.add_message("잠자기를 취소했습니다.");
                break;
            }
#endif
            {
                
                std::cout << "🛏️ 침대입니다. 잠을 주무시겠습니까? (Y/N): " << std::flush;
                char conf = getch();
                if (conf == 'y' || conf == 'Y') advance_day();
                else renderer_.add_message("잠자기를 취소했습니다.");
            }
            break;
        case TileType::MARKET:
#ifdef FARM_GUI
            if (gui_mode_) { handle_gui_market(); break; }
#endif
            
            std::cout << "🏪 피에르 상점입니다.\n1. 구매\n2. 판매\n0. 나가기\n>> " << std::flush;
            {
                char sel = getch();
                if (sel == '1') market_.show_buy_shop(player_);
                else if (sel == '2') market_.show_sell_shop(player_);
            }
            renderer_.add_message("상점 이용을 마쳤습니다.");
            break;
        case TileType::KITCHEN:
#ifdef FARM_GUI
            if (gui_mode_) { handle_gui_kitchen(); break; }
#endif
            kitchen_.interact(player_);
            renderer_.add_message("주방 이용을 마쳤습니다.");
            break;
        case TileType::WORKSHOP:
#ifdef FARM_GUI
            if (gui_mode_) { handle_gui_workshop(); break; }
#endif
            workshop_.interact(player_);
            renderer_.add_message("작업장 이용을 마쳤습니다.");
            break;
        case TileType::PATH:
            // 🌲 나무 벌목 로직
            if (tile.building_name == "나무") {
                
                std::cout << "🌲 야생의 나무입니다. 도끼질을 하여 베시겠습니까? (스태미나 1 소모) (Y/N): " << std::flush;
                char conf = getch();
                if (conf == 'y' || conf == 'Y') {
                    if (player_.use_stamina(1)) {
                        int wood_gain = 1 + std::rand() % 3; // 1~3개 랜덤 획득
                        player_.add_item(ItemType::WOOD, wood_gain);
                        tile.building_name = "";
                        tile.is_occupied = false;
                        renderer_.add_message("도끼질! 나무를 베어 목재 " + std::to_string(wood_gain) + "개를 얻었습니다! 🪵");
                    } else {
                        renderer_.add_message("스태미나가 부족하여 나무를 벨 수 없습니다. (필요: 1)");
                    }
                } else {
                    renderer_.add_message("나무 베기를 취소했습니다.");
                }
            } else {
                renderer_.add_message("여기서는 특별히 할 수 있는 일이 없습니다.");
            }
            break;
        default:
            break;
    }
}

void Game::handle_farm(Tile& tile) {
#ifdef FARM_GUI
    if (gui_mode_) { handle_gui_farm(tile); return; }
#endif
    

    if (!tile.has_crop()) {
        std::cout << "🌱 [농경지] 심을 씨앗을 고르세요.\n";
        std::cout << "1. 감자 씨앗 (보유: " << player_.count_item(ItemType::SEED_POTATO) << ")\n";
        std::cout << "2. 당근 씨앗 (보유: " << player_.count_item(ItemType::SEED_CARROT) << ")\n";
        std::cout << "3. 밀 씨앗   (보유: " << player_.count_item(ItemType::SEED_WHEAT) << ")\n";
        std::cout << "4. 쌀 씨앗   (보유: " << player_.count_item(ItemType::SEED_RICE) << ")\n";
        std::cout << "5. 사과 묘목 (보유: " << player_.count_item(ItemType::SEED_APPLE) << ")\n";
        std::cout << "0. 취소\n>> " << std::flush;

        char sel = getch();
        int crop_id = 0;
        if (sel == '1') crop_id = 1;
        else if (sel == '2') crop_id = 2;
        else if (sel == '3') crop_id = 3;
        else if (sel == '4') crop_id = 4;
        else if (sel == '5') crop_id = 5;

        if (crop_id == 0) { renderer_.add_message("작업을 취소했습니다."); return; }
        std::string message;
        CropSystem::plant_crop(player_, tile, crop_id, message);
        renderer_.add_message(message);
        return;
    }

    std::cout << "🌾 [농경지 관리]\n";
    std::cout << "🌱 성장 현황: " << tile.crop_progress << "/" << tile.crop_max_progress << "일\n";
    std::cout << "1. 수확하기\n";
    if (event_system_.is_drought_active()) std::cout << "2. 물 주기 (가뭄 대처, 스태미나 20)\n";
    std::cout << "9. 작물 제거\n";
    std::cout << "0. 취소\n>> " << std::flush;
    char sel = getch();

    std::string message;
    if (sel == '1') CropSystem::harvest_crop(player_, tile, message);
    else if (sel == '2' && event_system_.is_drought_active()) CropSystem::water_crop(player_, tile, message);
    else if (sel == '2') message = "가뭄이 아닐 때는 물 주기 행동이 필요하지 않습니다.";
    else if (sel == '9') remove_crop_from_tile(tile, message);
    else message = "작업을 취소했습니다.";
    renderer_.add_message(message);
}

void Game::handle_livestock(Tile& tile) {
#ifdef FARM_GUI
    if (gui_mode_) { handle_gui_livestock(tile); return; }
#endif
    

    if (!tile.has_animal()) {
        std::cout << "🐾 [사육장] 작업을 선택하세요.\n";
        std::cout << "1. 새끼 닭 배치 (보유: " << player_.count_item(ItemType::BABY_CHICKEN) << ")\n";
        std::cout << "2. 새끼 돼지 배치 (보유: " << player_.count_item(ItemType::BABY_PIG) << ")\n";
        std::cout << "3. 새끼 소 배치 (보유: " << player_.count_item(ItemType::BABY_COW) << ")\n";
        std::cout << "4. 달걀 부화 시도 (달걀 1개, 성공률 35%)\n";
        std::cout << "0. 취소\n>> " << std::flush;

        char sel = getch();
        std::string message;
        if (sel == '1') LivestockSystem::place_animal(player_, tile, 1, message);
        else if (sel == '2') LivestockSystem::place_animal(player_, tile, 2, message);
        else if (sel == '3') LivestockSystem::place_animal(player_, tile, 3, message);
        else if (sel == '4') LivestockSystem::hatch_egg(player_, message);
        else message = "작업을 취소했습니다.";
        renderer_.add_message(message);
        return;
    }

    std::cout << "🐄 [가축 관리]\n1. 생산물 수집\n2. 가축 제거\n0. 취소\n>> " << std::flush;
    char sel = getch();
    std::string message;
    if (sel == '1') {
        if (player_.is_wildlife_today() && !player_.has_str_node(2)) {
            message = "🐺 야생동물 출몰! 가축이 극심한 스트레스를 받아 오늘은 생산물을 수확할 수 없습니다.";
        } else {
            LivestockSystem::collect_product(player_, tile, message);
        }
    }
    else if (sel == '2') {
        std::cout << "\n정말 제거하시겠습니까? (Y/N): " << std::flush;
        char conf = getch();
        if (conf == 'y' || conf == 'Y') LivestockSystem::remove_animal(tile, message);
        else message = "가축 제거를 취소했습니다.";
    } else message = "작업을 취소했습니다.";
    renderer_.add_message(message);
}

void Game::handle_building(Tile& tile) {
#ifdef FARM_GUI
    if (gui_mode_) { handle_gui_building(tile); return; }
#endif
    

    if (!tile.building_name.empty()) {
        if (tile.building_name == "주방") kitchen_.interact(player_);
        else if (tile.building_name == "작업장") workshop_.interact(player_);
        renderer_.add_message("건물 이용을 마쳤습니다.");
        return;
    }

    std::cout << "🏡 [건축 부지]\n1. 주방 건설\n2. 작업장 건설\n0. 취소\n>> " << std::flush;
    char sel = getch();

    if (sel == '1') {
        if (has_built_facility("주방")) renderer_.add_message("주방은 이미 건설되어 있습니다. 하나만 지을 수 있습니다.");
        else { tile.building_name = "주방"; tile.is_occupied = true; renderer_.add_message("주방을 건설했습니다. 🍳"); }
    }
    else if (sel == '2') {
        if (has_built_facility("작업장")) renderer_.add_message("작업장은 이미 건설되어 있습니다. 하나만 지을 수 있습니다.");
        else { tile.building_name = "작업장"; tile.is_occupied = true; renderer_.add_message("작업장을 건설했습니다. 🔨"); }
    }
    else renderer_.add_message("건설을 취소했습니다.");
}

void Game::advance_day() {
    player_.sleep_anywhere();   // day++, recover_daily_status (tick 없음)
    market_.start_new_day();

    std::string event_message;
    event_system_.process_new_day(player_, map_, event_message);

    // 별 내리는 밤: process_new_day 이후 is_starry_night 체크
    // → 오늘 새로 발동된 경우도 포함, map.advance_day 1번 추가
    if (player_.is_starry_night()) {
        map_.advance_day();
    }
    map_.advance_day();

    // process_new_day 이후 tick: 남은 일수 차감
    player_.tick_starry_night();

    std::string message = "침대에서 잠을 잤습니다. 하루가 지나 작물과 가축이 성장했습니다. 🌅";
    if (!event_message.empty()) message += " " + event_message;
    renderer_.add_message(message);
#ifdef FARM_GUI
    if (gui_mode_) gui_renderer_.show_notice("새로운 하루", message);
#endif
}

void Game::show_inventory() {
#ifdef FARM_GUI
    if (gui_mode_) { show_gui_inventory(); return; }
#endif
    renderer_.draw_inventory_screen(player_);
    getch();
}

void Game::quit() {
#ifdef FARM_GUI
    if (gui_mode_) {
        if (gui_renderer_.confirm("게임 종료", "정말 게임을 종료하시겠습니까?\n저장 기능은 아직 없습니다.")) {
            is_running_ = false;
        } else {
            renderer_.add_message("게임 종료를 취소했습니다.");
        }
        return;
    }
#endif
    
    std::cout << "정말 게임을 종료하시겠습니까? 저장 기능은 아직 없습니다. (Y/N): " << std::flush;
    char ans = getch();
    if (ans == 'y' || ans == 'Y') {
        is_running_ = false;
        std::cout << "\n게임을 종료합니다. 다음에 또 만나요! 👋\n";
    } else renderer_.add_message("게임 종료를 취소했습니다.");
}

void Game::check_game_end() {
    if (player_.get_day() > MAX_DAY) {
        is_running_ = false;
        renderer_.draw_ending_screen(player_);
#ifdef FARM_GUI
        if (gui_mode_) gui_renderer_.show_notice("게임 종료", "100일간의 시뮬레이션이 종료되었습니다!");
#endif
        std::cout << "\n💯 100일간의 시뮬레이션이 종료되었습니다!\n";
    }
}


bool Game::has_built_facility(const std::string& name) const {
    for (int y = 0; y < Map::HEIGHT; ++y) {
        for (int x = 0; x < Map::WIDTH; ++x) {
            const Tile& t = map_.get_tile(x, y);
            if (t.building_name == name) return true;
            if (name == "주방" && t.type == TileType::KITCHEN) return true;
            if (name == "작업장" && t.type == TileType::WORKSHOP) return true;
        }
    }
    return false;
}


namespace {
struct StorageUpgradeRule {
    int target_level;
    int required_player_level;
    int gold;
    int wood;
    int copper;
    int iron;
    int gold_ore;
    int added_slots;
};

StorageUpgradeRule storage_rule_for_target(int target_level) {
    if (target_level == 1) return {1, 1, 100, 5, 1, 0, 0, 30};
    if (target_level == 2) return {2, 8, 400, 10, 3, 2, 0, 40};
    return {3, 16, 1000, 20, 0, 5, 2, 50};
}

std::string storage_rule_text(const StorageUpgradeRule& r) {
    std::ostringstream oss;
    oss << "Lv." << r.required_player_level << " / " << r.gold << "G / 나무" << r.wood;
    if (r.copper > 0) oss << " / 구리" << r.copper;
    if (r.iron > 0) oss << " / 철" << r.iron;
    if (r.gold_ore > 0) oss << " / 금" << r.gold_ore;
    oss << " / 용량 +" << r.added_slots;
    return oss.str();
}
}

bool Game::build_storage(Tile& tile, std::string& message) {
    if (has_built_facility("저장소") || player_.get_storage_level() > 0) {
        message = "저장소는 이미 건설되어 있습니다. 하나만 지을 수 있습니다.";
        return false;
    }
    StorageUpgradeRule r = storage_rule_for_target(1);
    if (player_.get_level() < r.required_player_level) {
        message = "저장소 Lv.1은 플레이어 Lv." + std::to_string(r.required_player_level) + "부터 건설할 수 있습니다.";
        return false;
    }
    if (player_.get_money() < r.gold || player_.count_item(ItemType::WOOD) < r.wood ||
        player_.count_item(ItemType::COPPER) < r.copper) {
        message = "저장소 건설 재료가 부족합니다. 필요: " + storage_rule_text(r);
        return false;
    }
    player_.spend_money(r.gold);
    player_.remove_item(ItemType::WOOD, r.wood);
    if (r.copper > 0) player_.remove_item(ItemType::COPPER, r.copper);
    player_.set_storage_level(1);
    player_.expand_inventory_slots(r.added_slots);
    tile.building_name = "저장소";
    tile.is_occupied = true;
    message = "저장소 Lv.1을 건설했습니다. 인벤토리 최대 용량 +" + std::to_string(r.added_slots) + "!";
    return true;
}

bool Game::upgrade_storage(std::string& message) {
    int current = player_.get_storage_level();
    if (current <= 0) {
        message = "먼저 저장소를 건설해야 합니다.";
        return false;
    }
    if (current >= 3) {
        message = "저장소는 이미 최고 레벨입니다.";
        return false;
    }
    StorageUpgradeRule r = storage_rule_for_target(current + 1);
    if (player_.get_level() < r.required_player_level) {
        message = "저장소 Lv." + std::to_string(r.target_level) + " 업그레이드는 플레이어 Lv." + std::to_string(r.required_player_level) + "부터 가능합니다.";
        return false;
    }
    if (player_.get_money() < r.gold || player_.count_item(ItemType::WOOD) < r.wood ||
        player_.count_item(ItemType::COPPER) < r.copper || player_.count_item(ItemType::IRON) < r.iron ||
        player_.count_item(ItemType::GOLD) < r.gold_ore) {
        message = "저장소 업그레이드 재료가 부족합니다. 필요: " + storage_rule_text(r);
        return false;
    }
    player_.spend_money(r.gold);
    player_.remove_item(ItemType::WOOD, r.wood);
    if (r.copper > 0) player_.remove_item(ItemType::COPPER, r.copper);
    if (r.iron > 0) player_.remove_item(ItemType::IRON, r.iron);
    if (r.gold_ore > 0) player_.remove_item(ItemType::GOLD, r.gold_ore);
    player_.set_storage_level(r.target_level);
    player_.expand_inventory_slots(r.added_slots);
    message = "저장소 Lv." + std::to_string(r.target_level) + "로 업그레이드했습니다. 인벤토리 최대 용량 +" + std::to_string(r.added_slots) + "!";
    return true;
}

void Game::remove_crop_from_tile(Tile& tile, std::string& message) {
    if (!tile.has_crop()) {
        message = "제거할 작물이 없습니다.";
        return;
    }
    const CropInfo* info = CropSystem::get_crop_info(tile.crop_id);
    std::string name = info ? info->name : "작물";
    tile.crop_id = 0;
    tile.crop_progress = 0;
    tile.crop_max_progress = 0;
    tile.crop_perennial = false;
    tile.crop_harvest_cooldown = 0;
    tile.watered_today = false;
    tile.is_occupied = tile.has_animal() || !tile.building_name.empty();
    message = name + "을(를) 제거했습니다.";
}


#ifdef FARM_GUI

static std::string gui_inventory_summary(const Player& player, int max_items = 8) {
    std::ostringstream oss;
    oss << "현재 보유: ";
    auto items = player.inventory().get_all();
    if (items.empty()) return oss.str() + "없음";
    int shown = 0;
    for (const auto& it : items) {
        if (shown > 0) oss << ", ";
        oss << item_name(it.first) << " x" << it.second;
        ++shown;
        if (shown >= max_items) {
            if (static_cast<int>(items.size()) > shown) oss << ", ...";
            break;
        }
    }
    return oss.str();
}

void Game::handle_gui_market() {
    int mode = gui_renderer_.show_choice("상점", "피에르/루이스 상점입니다. 어떤 거래를 할까요?", {{1, "구매"}, {2, "판매"}, {0, "나가기"}}, 0);
    if (mode == 0) { renderer_.add_message("상점 이용을 취소했습니다."); return; }

    if (mode == 1) {
        std::vector<GuiShopEntry> entries;
        const auto& items = market_.buy_items();
        for (std::size_t i = 0; i < items.size(); ++i) {
            const auto& item = items[i];
            std::ostringstream sub;
            sub << item.buy_price << "G";
            bool locked = player_.get_level() < item.required_level;
            if (locked) sub << " | Lv." << item.required_level << " 해금";
            entries.push_back({static_cast<int>(i + 1), item.item, item.name, sub.str(), locked});
        }
        int choice = gui_renderer_.show_shop_selector("구매 상점", "내 인벤토리", "구매 가능 상품", entries, player_, "W/S 또는 방향키로 이동, Enter로 선택. 마우스 클릭/휠/스크롤바 드래그도 가능합니다.", 0);
        if (choice <= 0) { renderer_.add_message("구매를 취소했습니다."); return; }
        const auto& item = items[choice - 1];
        if (player_.get_level() < item.required_level) {
            std::string msg = item.name + "은(는) Lv." + std::to_string(item.required_level) + "부터 구매할 수 있습니다.";
            renderer_.add_message(msg);
            gui_renderer_.show_notice("구매 불가", msg);
            return;
        }
        int max_qty = item.buy_price <= 0 ? 999 : player_.get_money() / item.buy_price;
        if (max_qty <= 0) {
            std::string msg = "골드가 부족합니다.";
            renderer_.add_message(msg);
            gui_renderer_.show_notice("구매 불가", msg);
            return;
        }
        std::ostringstream qmsg;
        qmsg << item.name << "을(를) 몇 개 구매할까요?\n"
             << "단가: " << item.buy_price << "G | 현재 보유: " << player_.count_item(item.item) << "개\n"
             << "최대 구매 가능: " << max_qty << "개";
        int qty = gui_renderer_.prompt_quantity("구매 수량 입력", qmsg.str(), max_qty, 1);
        if (qty <= 0) { renderer_.add_message("구매를 취소했습니다."); return; }
        std::string msg;
        market_.buy_item(player_, static_cast<std::size_t>(choice - 1), qty, msg);
        renderer_.add_message(msg);
        gui_renderer_.show_notice("구매 결과", msg);
        return;
    }

    if (mode == 2) {
        std::vector<GuiShopEntry> entries;
        const auto& items = market_.sell_items();
        for (std::size_t i = 0; i < items.size(); ++i) {
            const auto& item = items[i];
            std::ostringstream sub;
            sub << item.sell_price << "G";
            if (player_.count_item(item.item) <= 0) sub << " | 보유 없음";
            entries.push_back({static_cast<int>(i + 1), item.item, item.name, sub.str(), player_.count_item(item.item) <= 0});
        }
        int choice = gui_renderer_.show_shop_selector("판매 상점", "내 인벤토리", "판매 가능 품목", entries, player_, "W/S 또는 방향키로 이동, Enter로 선택. 마우스 클릭/휠/스크롤바 드래그도 가능합니다.", 0);
        if (choice <= 0) { renderer_.add_message("판매를 취소했습니다."); return; }
        const auto& item = items[choice - 1];
        int max_qty = player_.count_item(item.item);
        if (max_qty <= 0) {
            std::string msg = "보유 수량이 없어 판매할 수 없습니다.";
            renderer_.add_message(msg);
            gui_renderer_.show_notice("판매 불가", msg);
            return;
        }
        std::ostringstream qmsg;
        qmsg << item.name << "을(를) 몇 개 판매할까요?\n"
             << "단가: " << item.sell_price << "G | 현재 보유: " << max_qty << "개";
        int qty = gui_renderer_.prompt_quantity("판매 수량 입력", qmsg.str(), max_qty, max_qty);
        if (qty <= 0) { renderer_.add_message("판매를 취소했습니다."); return; }
        std::string msg;
        market_.sell_item(player_, static_cast<std::size_t>(choice - 1), qty, msg);
        renderer_.add_message(msg);
        gui_renderer_.show_notice("판매 결과", msg);
    }
}


void Game::handle_gui_farm(Tile& tile) {
    std::string msg;
    if (!tile.has_crop()) {
        std::ostringstream body;
        body << "심을 씨앗을 선택하세요.\n"
             << "감자 씨앗: " << player_.count_item(ItemType::SEED_POTATO) << "개\n"
             << "당근 씨앗: " << player_.count_item(ItemType::SEED_CARROT) << "개\n"
             << "밀 씨앗: " << player_.count_item(ItemType::SEED_WHEAT) << "개\n"
             << "쌀 씨앗: " << player_.count_item(ItemType::SEED_RICE) << "개\n"
             << "사과 묘목: " << player_.count_item(ItemType::SEED_APPLE) << "개";
        int crop_id = gui_renderer_.show_choice("농경지", body.str(), {{1, "감자"}, {2, "당근"}, {3, "밀"}, {4, "쌀"}, {5, "사과"}, {0, "취소"}}, 0);
        if (crop_id == 0) {
            renderer_.add_message("씨앗 심기를 취소했습니다.");
            return;
        }
        bool ok = CropSystem::plant_crop(player_, tile, crop_id, msg);
        renderer_.add_message(msg);
        // 씨앗이 정상적으로 심어진 경우에는 확인 팝업 없이 바로 게임 화면으로 복귀한다.
        // 씨앗 부족/레벨 제한/스태미나 부족 등 실패 상황은 기존처럼 안내 팝업을 유지한다.
        if (!ok) gui_renderer_.show_notice("농경지", msg);
        return;
    }

    std::ostringstream body;
    body << "성장 현황: " << tile.crop_progress << "/" << tile.crop_max_progress << "일\n";
    if (tile.crop_progress < tile.crop_max_progress) body << "아직 씨앗/새싹 상태입니다. 필요하면 작물을 제거할 수 있습니다.";
    std::vector<std::pair<int, std::string>> buttons{{1, "수확"}};
    if (event_system_.is_drought_active()) buttons.push_back({2, "물 주기"});
    buttons.push_back({9, "작물 제거"});
    buttons.push_back({0, "취소"});
    int sel = gui_renderer_.show_choice("농경지 관리", body.str(), buttons, 0);
    if (sel == 1) {
        CropSystem::harvest_crop(player_, tile, msg);
        renderer_.add_message(msg);
        return;
    }
    else if (sel == 2 && event_system_.is_drought_active()) CropSystem::water_crop(player_, tile, msg);
    else if (sel == 9) remove_crop_from_tile(tile, msg);
    else msg = "작업을 취소했습니다.";
    renderer_.add_message(msg);
    if (sel != 0) gui_renderer_.show_notice("농경지 결과", msg);
}


void Game::handle_gui_livestock(Tile& tile) {
    std::string msg;
    if (!tile.has_animal()) {
        std::ostringstream body;
        body << "새끼 가축을 배치하거나 달걀을 부화할 수 있습니다.\n"
             << "새끼 닭: " << player_.count_item(ItemType::BABY_CHICKEN) << "개\n"
             << "새끼 돼지: " << player_.count_item(ItemType::BABY_PIG) << "개\n"
             << "새끼 소: " << player_.count_item(ItemType::BABY_COW) << "개\n"
             << "달걀: " << player_.count_item(ItemType::EGG) << "개";
        int sel = gui_renderer_.show_choice("사육장", body.str(), {{1, "닭 배치"}, {2, "돼지 배치"}, {3, "소 배치"}, {4, "부화"}, {0, "취소"}}, 0);
        if (sel == 1) LivestockSystem::place_animal(player_, tile, 1, msg);
        else if (sel == 2) LivestockSystem::place_animal(player_, tile, 2, msg);
        else if (sel == 3) LivestockSystem::place_animal(player_, tile, 3, msg);
        else if (sel == 4) LivestockSystem::hatch_egg(player_, msg);
        else msg = "작업을 취소했습니다.";
        renderer_.add_message(msg);
        gui_renderer_.show_notice("사육장", msg);
        return;
    }

    int sel = gui_renderer_.show_choice("가축 관리", "성체가 된 가축은 생산물을 얻을 수 있습니다.", {{1, "생산물 수집"}, {2, "가축 제거"}, {0, "취소"}}, 0);
    if (sel == 1) LivestockSystem::collect_product(player_, tile, msg);
    else if (sel == 2) {
        if (gui_renderer_.confirm("가축 제거", "정말 이 가축을 제거할까요?")) LivestockSystem::remove_animal(tile, msg);
        else msg = "가축 제거를 취소했습니다.";
    } else msg = "작업을 취소했습니다.";
    renderer_.add_message(msg);
    gui_renderer_.show_notice("가축 관리", msg);
}

void Game::handle_gui_building(Tile& tile) {
    if (!tile.building_name.empty()) {
        if (tile.building_name == "주방") handle_gui_kitchen();
        else if (tile.building_name == "작업장") handle_gui_workshop();
        else if (tile.building_name == "저장소") handle_gui_storage();
        return;
    }
    int sel = gui_renderer_.show_choice("건축 부지", "건설할 시설을 선택하세요. 주방, 작업장, 저장소는 각각 하나만 지을 수 있습니다.", {{1, "주방"}, {2, "작업장"}, {3, "저장소"}, {0, "취소"}}, 0);
    if (sel == 1) {
        if (has_built_facility("주방")) {
            std::string msg = "주방은 이미 건설되어 있습니다. 하나만 지을 수 있습니다.";
            renderer_.add_message(msg);
            gui_renderer_.show_notice("건설 불가", msg);
        } else { tile.building_name = "주방"; tile.is_occupied = true; renderer_.add_message("주방을 건설했습니다. 🍳"); }
    }
    else if (sel == 2) {
        if (has_built_facility("작업장")) {
            std::string msg = "작업장은 이미 건설되어 있습니다. 하나만 지을 수 있습니다.";
            renderer_.add_message(msg);
            gui_renderer_.show_notice("건설 불가", msg);
        } else { tile.building_name = "작업장"; tile.is_occupied = true; renderer_.add_message("작업장을 건설했습니다. 🔨"); }
    }
    else if (sel == 3) {
        std::string msg;
        build_storage(tile, msg);
        renderer_.add_message(msg);
        if (msg.find("건설했습니다") == std::string::npos) gui_renderer_.show_notice("건설 결과", msg);
    }
    else renderer_.add_message("건설을 취소했습니다.");
}

void Game::handle_gui_kitchen() {
    std::ostringstream status;
    status << "주방 Lv." << kitchen_.get_level()
           << " | 골드 " << player_.get_money() << "G"
           << " | 스태미나 " << player_.get_stamina() << "/" << player_.get_stamina_max()
           << "\n보유: 밀 " << player_.count_item(ItemType::WHEAT)
           << ", 고등어 " << player_.count_item(ItemType::FISH_MACKEREL)
           << ", 사과 " << player_.count_item(ItemType::APPLE);

    const bool apple_locked = kitchen_.get_level() < 2;
    const bool upgrade_locked = kitchen_.get_level() >= 3;
    std::vector<GuiActionEntry> actions{
        {1, "빵 만들기", "밀2 / Sta10 / XP20", "food_bread", false},
        {2, "생선구이 만들기", "고등어1 / Sta15 / XP30", "food_grilled_fish", false},
        {3, "사과파이 만들기", "주방 Lv.2 필요 / 밀2+사과3 / Sta20 / XP50", "food_apple_pie", apple_locked},
        {4, upgrade_locked ? "주방 최고 레벨" : "주방 업그레이드", upgrade_locked ? "더 이상 업그레이드할 수 없습니다." : "500G / 최대 Lv.3", "kitchen", upgrade_locked}
    };
    std::vector<GuiActionEntry> recipes{
        {1, "빵", "밀 2개 + 스태미나 10", "item_wheat", false},
        {2, "생선구이", "고등어 1개 + 스태미나 15", "fish_mackerel", false},
        {3, "사과파이", "주방 Lv.2 / 밀 2개 + 사과 3개", "food_apple_pie", apple_locked},
        {4, "업그레이드", "500G로 주방 레벨 +1", "coin", upgrade_locked}
    };

    int sel = gui_renderer_.show_action_panel("주방", status.str(), "조합식 / 조건", actions, recipes, 0);
    if (sel == 0) { renderer_.add_message("주방 이용을 취소했습니다."); return; }
    std::string msg;
    kitchen_.perform_action(player_, sel, msg);
    renderer_.add_message(msg);
    gui_renderer_.show_notice("주방 결과", msg);
}

void Game::handle_gui_workshop() {
    std::ostringstream status;
    status << "작업장 Lv." << workshop_.get_level()
           << " | 농기구 등급 " << player_.get_tool_grade()
           << " | 플레이어 Lv." << player_.get_level()
           << "\n보유: 나무 " << player_.count_item(ItemType::WOOD)
           << ", 구리 " << player_.count_item(ItemType::COPPER)
           << ", 철 " << player_.count_item(ItemType::IRON)
           << ", 금 " << player_.count_item(ItemType::GOLD)
           << ", 골드 " << player_.get_money() << "G";

    int need_level = 0;
    std::string expand_req;
    if (workshop_.get_level() == 0) { need_level = 6; expand_req = "Lv6 / 200G / Sta80"; }
    else if (workshop_.get_level() == 1) { need_level = 10; expand_req = "Lv10 / 600G / Sta150"; }
    else if (workshop_.get_level() == 2) { need_level = 15; expand_req = "Lv15 / 1200G / Sta200"; }
    else { need_level = 999; expand_req = "최고 레벨"; }

    const bool expand_locked = workshop_.get_level() >= 3 || player_.get_level() < need_level;
    const bool copper_locked = workshop_.get_level() < 1 || player_.get_tool_grade() >= 1;
    const bool iron_locked = workshop_.get_level() < 2 || player_.get_tool_grade() >= 2;
    const bool gold_locked = workshop_.get_level() < 3 || player_.get_tool_grade() >= 3;

    std::vector<GuiActionEntry> actions{
        {1, workshop_.get_level() >= 3 ? "작업장 최고 레벨" : "작업장 증축", expand_req, "workshop", expand_locked},
        {2, "구리 농기구", "작업장Lv1 / 100G / 나무5 / 구리3 / Sta30", "copper", copper_locked},
        {3, "철 농기구", "작업장Lv2 / 300G / 나무10 / 철4 / Sta30", "iron", iron_locked},
        {4, "금 농기구", "작업장Lv3 / 800G / 나무20 / 금2 / Sta30", "gold", gold_locked}
    };
    std::vector<GuiActionEntry> recipes{
        {1, "작업장 증축", expand_req, "workshop", expand_locked},
        {2, "구리 농기구", "구리 3 + 나무 5 + 100G", "copper", copper_locked},
        {3, "철 농기구", "철 4 + 나무 10 + 300G", "iron", iron_locked},
        {4, "금 농기구", "금 2 + 나무 20 + 800G", "gold", gold_locked}
    };

    int sel = gui_renderer_.show_action_panel("작업장", status.str(), "제작식 / 조건", actions, recipes, 0);
    if (sel == 0) { renderer_.add_message("작업장 이용을 취소했습니다."); return; }
    std::string msg;
    workshop_.perform_action(player_, sel, msg);
    renderer_.add_message(msg);
    gui_renderer_.show_notice("작업장 결과", msg);
}


void Game::handle_gui_storage() {
    int current = player_.get_storage_level();
    std::ostringstream status;
    status << "저장소 Lv." << current
           << " | 인벤토리 용량 " << player_.inventory().used_slots() << "/" << player_.inventory().max_slots()
           << " | 플레이어 Lv." << player_.get_level()
           << "\n보유: 나무 " << player_.count_item(ItemType::WOOD)
           << ", 구리 " << player_.count_item(ItemType::COPPER)
           << ", 철 " << player_.count_item(ItemType::IRON)
           << ", 금 " << player_.count_item(ItemType::GOLD)
           << ", 골드 " << player_.get_money() << "G";

    bool maxed = current >= 3;
    StorageUpgradeRule next = storage_rule_for_target(maxed ? 3 : current + 1);
    bool locked = maxed || player_.get_level() < next.required_player_level;
    std::vector<GuiActionEntry> actions{
        {1, maxed ? "저장소 최고 레벨" : "저장소 업그레이드", maxed ? "더 이상 확장할 수 없습니다." : storage_rule_text(next), "storage", locked}
    };
    std::vector<GuiActionEntry> recipes{
        {1, "Lv.1 저장소", storage_rule_text(storage_rule_for_target(1)), "storage", current >= 1},
        {2, "Lv.2 저장소", storage_rule_text(storage_rule_for_target(2)), "storage", current >= 2 || player_.get_level() < 8},
        {3, "Lv.3 저장소", storage_rule_text(storage_rule_for_target(3)), "storage", current >= 3 || player_.get_level() < 16}
    };
    int sel = gui_renderer_.show_action_panel("저장소", status.str(), "확장 조건", actions, recipes, 0);
    if (sel == 0) { renderer_.add_message("저장소 이용을 취소했습니다."); return; }
    std::string msg;
    upgrade_storage(msg);
    renderer_.add_message(msg);
    gui_renderer_.show_notice("저장소 결과", msg);
}

void Game::show_gui_inventory() {
    gui_renderer_.show_inventory(player_);
    renderer_.add_message("인벤토리를 확인했습니다.");
}
#endif

void Game::show_skill_tree() {
    while (true) {
        
        std::cout << "🌟 [스킬 트리]\n";
        std::cout << "보유 스킬 포인트: " << player_.get_skill_points() << "\n\n";
        std::cout << "어떤 스킬 트리를 확인하시겠습니까?\n";
        std::cout << "[S] STR (근력) 트리\n";
        std::cout << "[I] INT (지능) 트리\n";
        std::cout << "[L] LUK (행운) 트리\n";
        std::cout << "[0] 창 닫기\n>> " << std::flush;

        char sel = getch();
        if (sel == '0') break;
        else if (sel == 's' || sel == 'S') show_str_tree();
        else if (sel == 'i' || sel == 'I') show_int_tree();
        else if (sel == 'l' || sel == 'L') show_luk_tree();
    }
    renderer_.add_message("스킬 트리 창을 닫았습니다.");
}

void Game::show_str_tree() {
    while (true) {
        
        std::cout << "💪 [STR 스킬 트리] (보유 포인트: " << player_.get_skill_points() << ")\n\n";
        
        // 람다 함수로 배운 스킬은 [O], 안 배운 건 [ ] 표시
        auto st = [&](int id) { return player_.has_str_node(id) ? "[O]" : "[ ]"; };

        // ── 기존 스킬 출력 부분을 아래 코드로 교체 ──
        std::cout << "1단계:\n";
        std::cout << "  " << st(1) << " 1. 거대 위장\n";
        std::cout << "     └─ 🫐 최대 스태미나 통 +5 영구 확장\n\n";

        std::cout << "2단계:\n";
        std::cout << "  ├── " << st(2) << " 2. 압도적인 기백\n";
        std::cout << "  │    └─ 🍃 감기 이벤트 스태미나 감소 완화 (-50% → -20%), 야생동물 출몰 무효화\n";
        std::cout << "  └── " << st(3) << " 3. 채광왕\n";
        std::cout << "       └─ ⛏️ 광산 붕괴 압력 초과 시 30% 확률로 완력 버팀 (즉시 탈출 시 보상 보존)\n\n";

        std::cout << "3단계:\n";
        std::cout << "  " << st(4) << " 4. 강철 폐활량\n";
        std::cout << "     └─ 🫁 최대 스태미나 통 +10 추가 영구 확장\n\n";

        std::cout << "4단계:\n";
        std::cout << "  ├── " << st(5) << " 5. 대지의 아들\n";
        std::cout << "  │    └─ 🌾 수확 시 20% 확률로 스태미나 소모 0 (면제)\n";
        std::cout << "  └── " << st(6) << " 6. 헤라클레스\n";
        std::cout << "       └─ 🔥 광산 압력 초과 시 100% 확률로 버팀, 누적 압력 수치 공개, 탈출 시 광물 보상 2배\n\n";

        std::cout << "투자할 스킬 번호를 입력하세요 (0: 뒤로 가기) >> " << std::flush;
        char sel = getch();
        if (sel == '0') break;

        int node = sel - '0';
        if (node >= 1 && node <= 6) {
            std::string msg;
            if (player_.invest_str_node(node, msg)) {
                std::cout << "\n✨ " << msg << "\n";
            } else {
                std::cout << "\n❌ " << msg << "\n";
            }
            std::cout << "(아무 키나 누르면 돌아갑니다)";
            getch();
        }
    }
}

void Game::show_int_tree() {
    while (true) {
        
        std::cout << "🧠 [INT 스킬 트리] (보유 포인트: " << player_.get_skill_points() << ")\n\n";
        auto st = [&](int id) { return player_.has_int_node(id) ? "[O]" : "[ ]"; };

        // ── 1. 정상 루트 출력 ──
        std::cout << "💡 <정상 루트>\n";
        std::cout << "1단계: " << st(1) << " 1. 공정 최적화\n";
        std::cout << "     └─ ⚙️ 주방, 대장간 업그레이드 비용 및 농기구 제작 비용(G) 20% 감소\n\n";
        
        std::cout << "2단계: " << st(2) << " 2. 재무 설계\n";
        std::cout << "     └─ 📈 하루를 시작할 때 소지 골드의 1%를 이자로 획득 (최대 100G)\n\n";
        
        std::cout << "3단계: " << st(3) << " 3. 단골 손님\n";
        std::cout << "     └─ 🛒 상점 물품 구매 가격 -10%\n\n";
        
        std::cout << "4단계: " << st(4) << " 4. 농업 협동조합장\n";
        std::cout << "     └─ 🌾 상점에 판매한 모든 품목 총수익의 10%를 다음 날 아침 추가 배당금으로 획득\n\n";

        // ── 2. 히든 루트 출력 및 보안 마스킹 사양 처리 ──
        std::cout << "🔮 <히든 빌드> ";
        
        // 플레이어가 괴식을 한 번이라도 판매했는지 체크
        bool is_hidden_unlocked = player_.has_sold_monster_food(); 

        if (!is_hidden_unlocked) {
            // [최초 판매 전] 철저하게 은폐 처리
            std::cout << "🔒 (특정 조건 만족 후 잠금 해제)\n";
            std::cout << "  ※ 아직 알 수 없는 기묘한 기운이 잠재되어 있습니다...\n\n";
        } else {
            // [최초 판매 후] 전체 스킬명과 이모지 요약 설명 오픈
            std::cout << "🔓 (잠금 해제 완료)\n";
            std::cout << "1단계: " << st(5) << " 5. 야바위꾼\n";
            std::cout << "     └─ 🃏 괴식 판매 시 기본 단가 +10% 추가 보너스 획득\n\n";
            
            // 데이터 연동 흐름 가시성 확보를 위해 2~4단계 연달아 노출
            std::cout << "2단계: " << st(6) << " 6. 위조 전문가\n";
            std::cout << "     └─ 📜 괴식 사기 판매 성공률 +20%\n\n";
            
            std::cout << "3단계: " << st(7) << " 7. 능수능란한 언변\n";
            std::cout << "     └─ 💬 괴식 사기 판매 실패 시 50% 확률로 신뢰도 하락 무효\n\n";
            
            std::cout << "4단계: " << st(8) << " 8. 괴도 루팡\n";
            std::cout << "     └─ 🎩 괴식 사기 판매 성공률 +20% 추가 증가\n\n";
        }

        std::cout << "투자할 스킬 번호를 입력하세요 (1~8, 0: 뒤로 가기) >> " << std::flush;
        char sel = getch();
        if (sel == '0') break;

        int node = sel - '0';
        if (node >= 1 && node <= 8) {
            // 아직 잠겨있는데 히든 루트(5~8번)를 치트나 실수로 누른 경우 방어문 추가
            if (node >= 5 && node <= 8 && !is_hidden_unlocked) {
                std::cout << "\n❌ 아직 해금되지 않은 비밀 스킬입니다!\n";
                std::cout << "(아무 키나 누르면 돌아갑니다)";
                getch();
                continue;
            }

            std::string msg;
            if (player_.invest_int_node(node, msg)) {
                std::cout << "\n✨ " << msg << "\n";
            } else {
                std::cout << "\n❌ " << msg << "\n";
            }
            std::cout << "(아무 키나 누르면 돌아갑니다)";
            getch();
        }
    }
}

void Game::show_luk_tree() {
    while (true) {
        
        std::cout << "🍀 [LUK 스킬 트리] (보유 포인트: " << player_.get_skill_points() << ")\n\n";
        auto st = [&](int id) { return player_.has_luk_node(id) ? "[O]" : "[ ]"; };

        // ── 행운 스킬 트리 출력 ──
        std::cout << "1단계:\n";
        std::cout << "  " << st(1) << " 1. 씨앗 환급\n";
        std::cout << "     └─ 🌱 농사 작물 수확 시 10% 확률로 심었던 씨앗 1개 회수\n\n";

        std::cout << "2단계:\n";
        std::cout << "  ├── " << st(2) << " 2. 쌍둥이 가축\n";
        std::cout << "  │    └─ 🐣 가축 생산물 수확 시 20% 확률로 생산물 +1\n";
        std::cout << "  └── " << st(3) << " 3. 강태공의 가호\n";
        std::cout << "       └─ 🎣 고등어, 광어, 참치 주사위에서 각 생선이 지급될 확률 +10%\n\n";

        std::cout << "3단계:\n";
        std::cout << "  " << st(4) << " 4. 알뜰한 손재주\n";
        std::cout << "     └─ 🍳 주방 요리 시 요구 재료 중 무작위 1개 소모 면제\n\n";

        std::cout << "4단계:\n";
        std::cout << "  ├── " << st(5) << " 5. 우주적 기연\n";
        std::cout << "  │    └─ ✨ '별 내리는 밤' 이벤트 발생 확률 증가 (1% → 10%)\n";
        std::cout << "  └── " << st(6) << " 6. 황금 어장의 지배자\n";
        std::cout << "       └─ 👑 생선 주사위 확률 +10% 추가 증가 & 낚시 성공 시 10% 확률로 생선 3종 세트 추가 획득\n\n";

        std::cout << "투자할 스킬 번호를 입력하세요 (1~6, 0: 뒤로 가기) >> " << std::flush;
        char sel = getch();
        if (sel == '0') break;

        int node = sel - '0';
        if (node >= 1 && node <= 6) {
            std::string msg;
            if (player_.invest_luk_node(node, msg)) {
                std::cout << "\n✨ " << msg << "\n";
            } else {
                std::cout << "\n❌ " << msg << "\n";
            }
            std::cout << "(아무 키나 누르면 돌아갑니다)";
            getch();
        }
    }
}
