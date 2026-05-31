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
      barn_(),
      current_area_(GameArea::FARM),
      farm_return_x_(Map::BED_X),
      farm_return_y_(Map::BED_Y),
      is_running_(true) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

void Game::run() {
    renderer_.draw_title_screen();
    std::string player_name;
    std::getline(std::cin, player_name);
    if (player_name.empty()) player_name = "초보 농부";
    player_.set_name(player_name);

    player_.add_item(ItemType::SEED_POTATO, 5);
    player_.add_item(ItemType::SEED_CARROT, 2);
    player_.add_item(ItemType::BABY_CHICKEN, 1);

    renderer_.add_message("허허, " + player_name + "이로구먼! 50G, 감자 씨앗 5개, 당근 씨앗 2개, 새끼 닭 1마리로 시작하네. 축사는 시작부터 설치되어 있네. 🌱");

#ifdef FARM_GUI
    gui_mode_ = gui_renderer_.initialize();
    if (gui_mode_) {
        renderer_.add_message("GUI 모드가 시작되었습니다. WASD 이동, J/Enter/Space 상호작용.");
        while (is_running_) {
            sync_gui_event_status();
            if (current_area_ == GameArea::BARN) {
                barn_.update_random_movement(player_);
                gui_renderer_.draw_barn(player_, barn_, renderer_.last_message());
            } else {
                gui_renderer_.draw(player_, map_, renderer_.last_message());
            }
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
    if (current_area_ == GameArea::BARN) {
        handle_barn_input(input);
    } else {
        handle_farm_input(input);
    }

    if (!player_.get_levelup_message().empty()) {
        renderer_.add_message(player_.get_levelup_message());
        player_.clear_levelup_message();
    }
}

void Game::handle_farm_input(char input) {
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
            renderer_.add_message("구매는 피에르 상점 위치에서 J키로만 이용할 수 있습니다.");
            break;
        case 'u': case 'U':
            renderer_.add_message("판매는 피에르 상점 위치에서 J키로만 이용할 수 있습니다.");
            break;
        case '5':
            renderer_.add_message("잠자기는 침대 위치에서 J키로만 할 수 있습니다.");
            break;
        case 'q': case 'Q':
            quit();
            break;
        default:
            renderer_.add_message("존재하지 않는 명령어입니다.");
            break;
    }
}

void Game::handle_barn_input(char input) {
    int dx = 0, dy = 0;
    switch (input) {
        case 'w': case 'W': dy = -1; player_.set_facing('b'); break;
        case 's': case 'S': dy = 1; player_.set_facing('f'); break;
        case 'a': case 'A': dx = -1; player_.set_facing('l'); break;
        case 'd': case 'D': dx = 1; player_.set_facing('r'); break;
        case 'j': case 'J': handle_barn_action(); return;
        case 't': case 'T': handle_barn_quick_interaction(); return;
        case 'e': case 'E': show_inventory(); return;
        case 'q': case 'Q': quit(); return;
        default: renderer_.add_message("축사 안에서는 WASD 이동, T 가축 상호작용, J 축사 관리, E 인벤토리를 사용합니다."); return;
    }

    if (dx != 0 || dy != 0) {
        int nx = player_.get_x() + dx;
        int ny = player_.get_y() + dy;
        if (barn_.is_walkable(nx, ny, player_)) {
            player_.set_position(nx, ny);
        } else {
            renderer_.add_message("그쪽으로는 이동할 수 없습니다.");
        }
    }
}

void Game::handle_action() {
    int px = player_.get_x();
    int py = player_.get_y();
    Tile& tile = map_.get_tile(px, py);

    // 2x2 건물은 건물 위가 아니라 문 앞 타일에서만 상호작용한다.
    if (!tile.door_target_name.empty()) {
        handle_facility_by_name(tile.door_target_name);
        return;
    }

    if (px == Map::MINE_X && py == Map::MINE_Y) {
#ifdef FARM_GUI
        if (gui_mode_) {
            std::string msg;
            gui_renderer_.play_lotto_mining(player_, msg);
            renderer_.add_message(msg.empty() ? "광산 탐험을 마쳤습니다." : msg);
            return;
        }
#endif
        system("clear");
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
            system("clear");
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
            handle_bed_interaction();
            break;
        case TileType::MARKET:
#ifdef FARM_GUI
            if (gui_mode_) { handle_gui_market(); break; }
#endif
            system("clear");
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
        case TileType::BUILDING:
            handle_building(tile);
            break;
        case TileType::PATH:
            renderer_.add_message("여기서는 특별히 할 수 있는 일이 없습니다.");
            break;
    }
}

void Game::handle_farm(Tile& tile) {
#ifdef FARM_GUI
    if (gui_mode_) { handle_gui_farm(tile); return; }
#endif
    system("clear");

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
    system("clear");

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
    if (sel == '1') LivestockSystem::collect_product(player_, tile, message);
    else if (sel == '2') {
        std::cout << "\n정말 제거하시겠습니까? (Y/N): " << std::flush;
        char conf = getch();
        if (conf == 'y' || conf == 'Y') LivestockSystem::remove_animal(tile, message);
        else message = "가축 제거를 취소했습니다.";
    } else message = "작업을 취소했습니다.";
    renderer_.add_message(message);
}

void Game::handle_facility_by_name(const std::string& name) {
    if (name == "상점") {
#ifdef FARM_GUI
        if (gui_mode_) { handle_gui_market(); return; }
#endif
        system("clear");
        std::cout << "🏪 피에르 상점입니다.\n1. 구매\n2. 판매\n0. 나가기\n>> " << std::flush;
        char sel = getch();
        if (sel == '1') market_.show_buy_shop(player_);
        else if (sel == '2') market_.show_sell_shop(player_);
        renderer_.add_message("상점 이용을 마쳤습니다.");
        return;
    }
    if (name == "주방") {
#ifdef FARM_GUI
        if (gui_mode_) { handle_gui_kitchen(); return; }
#endif
        kitchen_.interact(player_);
        renderer_.add_message("주방 이용을 마쳤습니다.");
        return;
    }
    if (name == "작업장") {
#ifdef FARM_GUI
        if (gui_mode_) { handle_gui_workshop(); return; }
#endif
        workshop_.interact(player_);
        renderer_.add_message("작업장 이용을 마쳤습니다.");
        return;
    }
    if (name == "축사") {
        enter_barn();
        return;
    }
    if (name == "저장소") {
#ifdef FARM_GUI
        if (gui_mode_) { handle_gui_storage(); return; }
#endif
        renderer_.add_message("저장소는 GUI 모드에서 업그레이드할 수 있습니다.");
        return;
    }
    renderer_.add_message("문 앞에 섰지만 이용할 수 있는 건물이 없습니다.");
}

bool Game::place_facility_on_current_plot(const std::string& name, std::string& message) {
    int ax = -1, ay = -1;
    if (!map_.find_build_plot_anchor(player_.get_x(), player_.get_y(), ax, ay)) {
        message = "건축 부지 위에서만 건설할 수 있습니다.";
        return false;
    }
    if (has_built_facility(name)) {
        message = name + "은(는) 이미 건설되어 있습니다. 하나만 지을 수 있습니다.";
        return false;
    }
    if (!map_.place_facility_2x2(ax, ay, name)) {
        message = "이 건축 부지에는 이미 건물이 있거나 건설할 수 없습니다.";
        return false;
    }
    message = name + "을(를) 2x2 크기로 건설했습니다. 건물 아래쪽 문 앞에서 J로 이용하세요.";
    return true;
}

void Game::sync_facility_level(const std::string& name, int level) {
    for (int y = 0; y < Map::HEIGHT; ++y) {
        for (int x = 0; x < Map::WIDTH; ++x) {
            Tile& t = map_.get_tile(x, y);
            if (t.building_name == name) t.building_level = level;
        }
    }
}

void Game::handle_building(Tile& tile) {
#ifdef FARM_GUI
    if (gui_mode_) { handle_gui_building(tile); return; }
#endif
    system("clear");

    if (!tile.building_name.empty()) {
        renderer_.add_message("건물 위로는 이동할 수 없습니다. 문 앞에서 J키로 이용하세요.");
        return;
    }

    std::cout << "🏡 [2x2 건축 부지]\n1. 주방 건설\n2. 작업장 건설\n3. 저장소 건설\n4. 축사 건설\n0. 취소\n>> " << std::flush;
    char sel = getch();
    std::string message;

    if (sel == '1') place_facility_on_current_plot("주방", message);
    else if (sel == '2') place_facility_on_current_plot("작업장", message);
    else if (sel == '3') build_storage(tile, message);
    else message = "건설을 취소했습니다.";
    renderer_.add_message(message);
}

void Game::handle_bed_interaction() {
#ifdef FARM_GUI
    if (gui_mode_) {
        if (gui_renderer_.confirm("잠자기", "오늘 하루를 마치고 잠들까요?")) advance_day();
        else renderer_.add_message("잠자기를 취소했습니다.");
        return;
    }
#endif
    system("clear");
    std::cout << "🛏️ 오늘 하루를 마치고 잠들까요? (Y/N): " << std::flush;
    char ans = getch();
    if (ans == 'y' || ans == 'Y') advance_day();
    else renderer_.add_message("잠자기를 취소했습니다.");
}


void Game::enter_barn() {
    farm_return_x_ = player_.get_x();
    farm_return_y_ = player_.get_y();
    current_area_ = GameArea::BARN;
    player_.set_position(BarnSystem::EXIT_X, BarnSystem::EXIT_Y - 1);
    renderer_.add_message("축사 내부로 들어왔습니다. 가축은 축사 안에서 랜덤하게 움직입니다.");
}

void Game::leave_barn() {
    current_area_ = GameArea::FARM;
    player_.set_position(farm_return_x_, farm_return_y_);
    renderer_.add_message("메인 농장으로 돌아왔습니다.");
}

void Game::handle_barn_action() {
    if (player_.get_x() == BarnSystem::EXIT_X && player_.get_y() == BarnSystem::EXIT_Y) {
        leave_barn();
        return;
    }

#ifdef FARM_GUI
    if (gui_mode_) {
        int sel = gui_renderer_.show_choice("축사 관리", "J키는 가축 배치와 부화 같은 축사 관리에 사용합니다. 바라보는 방향의 가축과 직접 상호작용하려면 T키를 누르세요.", {{1, "새끼 닭 배치"}, {2, "새끼 돼지 배치"}, {3, "새끼 소 배치"}, {4, "달걀 부화"}, {0, "취소"}}, 0);
        std::string msg;
        if (sel == 1) barn_.place_animal(player_, 1, msg);
        else if (sel == 2) barn_.place_animal(player_, 2, msg);
        else if (sel == 3) barn_.place_animal(player_, 3, msg);
        else if (sel == 4) barn_.hatch_egg(player_, msg);
        else msg = "축사 관리를 취소했습니다.";
        renderer_.add_message(msg);
        if (sel != 0) gui_renderer_.show_notice("축사 관리", msg);
        return;
    }
#endif

    std::string msg;
    if (barn_.collect_nearby_product(player_, msg)) renderer_.add_message(msg);
    else renderer_.add_message("축사에서는 GUI 모드로 가축을 배치/관리하는 것을 권장합니다. " + msg);
}


void Game::handle_barn_quick_interaction() {
    BarnAnimal* animal = barn_.front_animal(player_);
    if (!animal) {
        renderer_.add_message("바라보는 방향 1칸 앞에 상호작용할 가축이 없습니다. 가축을 바라본 뒤 T키를 누르세요.");
#ifdef FARM_GUI
        if (gui_mode_) gui_renderer_.show_notice("가축 상호작용", "바라보는 방향 1칸 앞에 상호작용할 가축이 없습니다.\n가축을 바라본 뒤 T키를 누르세요.");
#endif
        return;
    }

    const LivestockInfo* info = LivestockSystem::get_info(animal->tile.animal_type);
    std::string animal_name = info ? info->name : "가축";
#ifdef FARM_GUI
    if (gui_mode_) {
        std::string intro = animal_name + "이(가) 바로 앞에 있습니다.\n원하는 행동을 선택하세요.";
        int sel = gui_renderer_.show_choice("T: " + animal_name + " 상호작용", intro,
                                           {{1, "생산물 수집"}, {2, "상태 보기"}, {3, "가축 제거"}, {0, "취소"}}, 0);
        std::string msg;
        if (sel == 1) {
            barn_.collect_front_product(player_, msg);
        } else if (sel == 2) {
            BarnAnimal* current = barn_.front_animal(player_);
            msg = current ? barn_.describe_animal(*current) : "가축이 이동했습니다.";
        } else if (sel == 3) {
            if (gui_renderer_.confirm("가축 제거", animal_name + "을(를) 정말 제거할까요?\n이 작업은 되돌릴 수 없습니다.")) {
                barn_.remove_front_animal(player_, msg);
            } else {
                msg = "가축 제거를 취소했습니다.";
            }
        } else {
            msg = "가축 상호작용을 취소했습니다.";
        }
        renderer_.add_message(msg);
        if (sel != 0) gui_renderer_.show_notice("가축 상호작용", msg);
        return;
    }
#endif

    system("clear");
    std::cout << "🐾 " << animal_name << "과(와) 상호작용\n"
              << "1. 생산물 수집\n"
              << "2. 상태 보기\n"
              << "3. 가축 제거\n"
              << "0. 취소\n>> " << std::flush;
    char sel = getch();
    std::string msg;
    if (sel == '1') {
        barn_.collect_front_product(player_, msg);
    } else if (sel == '2') {
        BarnAnimal* current = barn_.front_animal(player_);
        msg = current ? barn_.describe_animal(*current) : "가축이 이동했습니다.";
    } else if (sel == '3') {
        std::cout << "\n정말 이 가축을 제거할까요? (Y/N): " << std::flush;
        char ans = getch();
        if (ans == 'y' || ans == 'Y') barn_.remove_front_animal(player_, msg);
        else msg = "가축 제거를 취소했습니다.";
    } else {
        msg = "가축 상호작용을 취소했습니다.";
    }
    renderer_.add_message(msg);
}

void Game::advance_day() {
    player_.sleep_anywhere();
    market_.start_new_day();

    EventDayReport event_report = event_system_.process_new_day(player_, map_);
    map_.advance_day();
    barn_.advance_day();

    std::string message = "침대에서 잠을 잤습니다. 하루가 지나 작물과 가축이 성장했습니다. 🌅";
    if (!event_report.message.empty()) message += " " + event_report.message;
    renderer_.add_message(message);
#ifdef FARM_GUI
    if (gui_mode_) {
        sync_gui_event_status();
        gui_renderer_.show_notice("새로운 하루", message);
        for (std::size_t i = 0; i < event_report.popup_titles.size(); ++i) {
            gui_renderer_.show_event_popup(event_report.popup_titles[i], event_report.popup_messages[i]);
        }
    }
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
    system("clear");
    std::cout << "정말 게임을 종료하시겠습니까? 저장 기능은 아직 없습니다. (Y/N): " << std::flush;
    char ans = getch();
    if (ans == 'y' || ans == 'Y') {
        is_running_ = false;
        std::cout << "\n게임을 종료합니다. 다음에 또 만나요! 👋\n";
    } else renderer_.add_message("게임 종료를 취소했습니다.");
}


#ifdef FARM_GUI
std::vector<std::string> Game::current_event_status_lines() const {
    std::vector<std::string> lines;
    if (event_system_.is_drought_active()) {
        lines.push_back("가뭄: " + std::to_string(event_system_.drought_days_left()) + "일 남음");
        lines.push_back("농지에서 물 주기 필요");
    }
    return lines;
}

void Game::sync_gui_event_status() {
    if (!gui_mode_) return;
    gui_renderer_.set_event_status(current_event_status_lines());
}
#endif

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

bool Game::build_barn(std::string& message) {
    if (has_built_facility("축사")) {
        message = "축사는 이미 건설되어 있습니다. 하나만 지을 수 있습니다.";
        return false;
    }
    int ax = -1, ay = -1;
    if (!map_.find_build_plot_anchor(player_.get_x(), player_.get_y(), ax, ay)) {
        message = "건축 부지 위에서만 축사를 건설할 수 있습니다.";
        return false;
    }
    if (!map_.place_facility_2x2(ax, ay, "축사")) {
        message = "이 건축 부지에는 축사를 건설할 수 없습니다.";
        return false;
    }
    message = "축사를 2x2 크기로 건설했습니다. 문 앞에서 J키를 누르면 축사 내부로 들어갑니다.";
    return true;
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
    int ax = -1, ay = -1;
    if (!map_.find_build_plot_anchor(player_.get_x(), player_.get_y(), ax, ay)) {
        message = "건축 부지 위에서만 저장소를 건설할 수 있습니다.";
        return false;
    }
    if (!map_.place_facility_2x2(ax, ay, "저장소")) {
        message = "이 건축 부지에는 저장소를 건설할 수 없습니다.";
        return false;
    }
    player_.spend_money(r.gold);
    player_.remove_item(ItemType::WOOD, r.wood);
    if (r.copper > 0) player_.remove_item(ItemType::COPPER, r.copper);
    player_.set_storage_level(1);
    player_.expand_inventory_slots(r.added_slots);
    message = "저장소 Lv.1을 2x2 크기로 건설했습니다. 인벤토리 최대 용량 +" + std::to_string(r.added_slots) + "! 문 앞에서 이용하세요.";
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
        gui_renderer_.show_notice("건물 이용", "건물 위로는 이동할 수 없습니다. 건물 아래쪽 문 앞에서 J/Enter로 이용하세요.");
        return;
    }
    int sel = gui_renderer_.show_choice("2x2 건축 부지", "건설할 시설을 선택하세요. 축사는 게임 시작 시 기본 설치되어 있습니다. 건설 후에는 건물 아래쪽 문 앞에서 이용합니다.", {{1, "주방"}, {2, "작업장"}, {3, "저장소"}, {0, "취소"}}, 0);
    std::string msg;
    if (sel == 1) {
        place_facility_on_current_plot("주방", msg);
        renderer_.add_message(msg);
        if (msg.find("건설했습니다") == std::string::npos) gui_renderer_.show_notice("건설 결과", msg);
    }
    else if (sel == 2) {
        place_facility_on_current_plot("작업장", msg);
        renderer_.add_message(msg);
        if (msg.find("건설했습니다") == std::string::npos) gui_renderer_.show_notice("건설 결과", msg);
    }
    else if (sel == 3) {
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
    sync_facility_level("주방", kitchen_.get_level());
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
    sync_facility_level("작업장", workshop_.get_level());
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
    sync_facility_level("저장소", player_.get_storage_level());
    renderer_.add_message(msg);
    gui_renderer_.show_notice("저장소 결과", msg);
}

void Game::show_gui_inventory() {
    gui_renderer_.show_inventory(player_);
    renderer_.add_message("인벤토리를 확인했습니다.");
}
#endif
