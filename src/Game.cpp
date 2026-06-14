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
      tutorial_(),
      current_area_(GameArea::FARM),
      farm_return_x_(Map::BED_X),
      farm_return_y_(Map::BED_Y),
      is_running_(true) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    map_.spawn_initial_trees();
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

    renderer_.add_message("안녕하세요, " + player_name + "님! 50G, 감자 씨앗 5개, 당근 씨앗 2개, 병아리 1마리로 시작합니다. 축사는 처음부터 설치되어 있습니다.");

#ifdef FARM_GUI
    gui_mode_ = gui_renderer_.initialize();
    if (gui_mode_) {
        renderer_.add_message("GUI 모드가 시작되었습니다. WASD 이동, J/Enter/Space 상호작용.");
        if (tutorial_.is_active()) {
            show_gui_tutorial();
        }
        bool needs_redraw = true;
        Uint32 last_gui_frame_ticks = 0;
        Uint32 last_barn_frame_ticks = 0;
        while (is_running_) {
            char input = 0;
            if (gui_renderer_.poll_input(input)) {
                handle_input(input);
                check_game_end();
                needs_redraw = true;
            }

            const Uint32 now = SDL_GetTicks();
            const bool animate_player = gui_renderer_.needs_animation_frame(player_);
            const bool animate_barn = current_area_ == GameArea::BARN && now - last_barn_frame_ticks >= 33;
            const bool frame_due = now - last_gui_frame_ticks >= 33;

            if (!needs_redraw && !animate_player && !animate_barn) {
                SDL_Delay(8);
                continue;
            }
            if (!needs_redraw && !frame_due) {
                SDL_Delay(8);
                continue;
            }

            sync_gui_event_status();
            if (current_area_ == GameArea::BARN) {
                barn_.update_random_movement(player_);
                gui_renderer_.draw_barn(player_, barn_, renderer_.last_message());
                last_barn_frame_ticks = now;
            } else {
                gui_renderer_.draw(player_, map_, renderer_.last_message());
            }
            last_gui_frame_ticks = now;
            needs_redraw = false;
            SDL_Delay(8);
        }
        return;
    }
    renderer_.add_message("GUI 초기화에 실패하여 터미널 모드로 실행합니다. DISPLAY와 assets 경로를 확인하세요.");
#endif

    while (is_running_) {
        if (tutorial_.is_active()) {
            show_tutorial();
        }
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
        case 'p': case 'P':
            show_skill_tree();
            break;
        case 'i': case 'I':
            renderer_.add_message("구매는 상점 앞에서 J로 이용할 수 있습니다.");
            break;
        case 'u': case 'U':
            renderer_.add_message("판매는 상점 앞에서 J로 이용할 수 있습니다.");
            break;
        case '5':
            renderer_.add_message("잠자기는 침대 위치에서 J로 이용할 수 있습니다.");
            break;
        case 'q': case 'Q':
            quit();
            break;
        default:
            renderer_.add_message("알 수 없는 명령입니다.");
            break;
    }
}

void Game::handle_action() {
    int px = player_.get_x();
    int py = player_.get_y();
    Tile& tile = map_.get_tile(px, py);

    // 2x2 건물은 건물 위가 아니라 문 타일에서만 상호작용한다.
    if (!tile.door_target_name.empty()) {
        handle_facility_by_name(tile.door_target_name);
        return;
    }

    if (px == Map::MINE_X && py == Map::MINE_Y) {
#ifdef FARM_GUI
        if (gui_mode_) {
            std::string msg;
            gui_renderer_.play_blackjack_mining(player_, msg);
            renderer_.add_message(msg.empty() ? "광산 탐험을 마쳤습니다." : msg);
            return;
        }
#endif
        system("clear");
        MiniGames::play_blackjack_mining(player_);
        renderer_.add_message("광산 탐험을 마쳤습니다.");
        return;
    }

    switch (tile.type) {
        case TileType::TREE:
            {
                constexpr int TREE_HARVEST_STAMINA = 1;
                if (!player_.use_stamina(TREE_HARVEST_STAMINA)) {
                    renderer_.add_message("스태미나가 부족해 나무를 벨 수 없습니다. 필요: 1");
                    break;
                }
                int wood = 1 + std::rand() % 3;
                if (player_.add_item(ItemType::WOOD, wood)) {
                    tile = Tile{};
                    tile.type = TileType::PATH;
                    renderer_.add_message("나무를 베어 목재 " + std::to_string(wood) + "개를 얻었습니다. 스태미나 -1, 남은 나무: " + std::to_string(map_.count_trees()) + "개");
                } else {
                    player_.restore_stamina(TREE_HARVEST_STAMINA);
                    renderer_.add_message("인벤토리 공간이 부족해 목재를 얻을 수 없습니다.");
                }
            }
            break;
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
            std::cout << "[상점]\n1. 구매\n2. 판매\n0. 나가기\n>> " << std::flush;
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
            renderer_.add_message("여기서는 할 수 있는 일이 없습니다.");
            break;
    }
}

void Game::handle_farm(Tile& tile) {
#ifdef FARM_GUI
    if (gui_mode_) { handle_gui_farm(tile); return; }
#endif
    system("clear");

    if (!tile.has_crop()) {
        std::cout << "[농경지] 심을 씨앗을 고르세요.\n";
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

    std::cout << "[농경지 관리]\n";
    std::cout << "성장 현황: " << tile.crop_progress << "/" << tile.crop_max_progress << "일\n";
    std::cout << "1. ?섑솗?섍린\n";
    if (event_system_.is_drought_active()) std::cout << "2. 물 주기 (가뭄 대처, 스태미나 2)\n";
    std::cout << "9. ?묐Ъ ?쒓굅\n";
    std::cout << "0. 취소\n>> " << std::flush;
    char sel = getch();

    std::string message;
    if (sel == '1') CropSystem::harvest_crop(player_, tile, message);
    else if (sel == '2' && event_system_.is_drought_active()) CropSystem::water_crop(player_, tile, message);
    else if (sel == '2') message = "가뭄이 아닐 때는 물 주기가 필요하지 않습니다.";
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
        std::cout << "[사육장] 작업을 선택하세요.\n";
        std::cout << "1. 병아리 배치 (보유: " << player_.count_item(ItemType::BABY_CHICKEN) << ")\n";
        std::cout << "2. 새끼 돼지 배치 (보유: " << player_.count_item(ItemType::BABY_PIG) << ")\n";
        std::cout << "3. 송아지 배치 (보유: " << player_.count_item(ItemType::BABY_COW) << ")\n";
        std::cout << "4. 알 부화 시도 (알 1개, 성공률 35%)\n";
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

    std::cout << "[가축 관리]\n1. 생산물 수집\n2. 가축 제거\n0. 취소\n>> " << std::flush;
    char sel = getch();
    std::string message;
    if (sel == '1') LivestockSystem::collect_product(player_, tile, message);
    else if (sel == '2') {
        std::cout << "\n정말 제거할까요? (Y/N): " << std::flush;
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
        std::cout << "[상점]\n1. 구매\n2. 판매\n0. 나가기\n>> " << std::flush;
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
    if (name == "작업장" || name == "Workshop") {
#ifdef FARM_GUI
        if (gui_mode_) { handle_gui_workshop(); return; }
#endif
        workshop_.interact(player_);
        renderer_.add_message("작업장 이용을 마쳤습니다.");
        return;
    }
    if (name == "저장소") {
#ifdef FARM_GUI
        if (gui_mode_) { handle_gui_storage(); return; }
#endif
        renderer_.add_message("저장소는 GUI 모드에서 업그레이드할 수 있습니다.");
        return;
    }
    if (name == "축사") {
        enter_barn();
        return;
    }
    renderer_.add_message("문 앞에 연결된 건물이 없습니다.");
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
        case 'p': case 'P': show_skill_tree(); return;
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

bool Game::place_facility_on_current_plot(const std::string& name, std::string& message) {
    int ax = -1, ay = -1;
    if (!map_.find_build_plot_anchor(player_.get_x(), player_.get_y(), ax, ay)) {
        message = "건축 부지 위에서만 건설할 수 있습니다.";
        return false;
    }
    if (has_built_facility(name)) {
        message = name + "은 이미 건설되어 있습니다. 하나만 지을 수 있습니다.";
        return false;
    }
    if (!map_.place_facility_2x2(ax, ay, name)) {
        message = "이 건축 부지에는 이미 건물이 있거나 건설할 수 없습니다.";
        return false;
    }
    message = name + "을 2x2 크기로 건설했습니다. 건물 아래쪽 문 앞에서 J로 이용하세요.";
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
        renderer_.add_message("건물 위로는 이동할 수 없습니다. 문 앞에서 J로 이용하세요.");
        return;
    }

    std::cout << "[2x2 건축 부지]\n1. 주방 건설\n2. 작업장 건설\n3. 저장소 건설\n0. 취소\n>> " << std::flush;
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
    std::cout << "오늘 하루를 마치고 잠들까요? (Y/N): " << std::flush;
    char ans = getch();
    if (ans == 'y' || ans == 'Y') advance_day();
    else renderer_.add_message("잠자기를 취소했습니다.");
}

void Game::enter_barn() {
    farm_return_x_ = player_.get_x();
    farm_return_y_ = player_.get_y();
    current_area_ = GameArea::BARN;
    player_.set_position(BarnSystem::EXIT_X, BarnSystem::EXIT_Y - 1);
    renderer_.add_message("축사 안으로 들어왔습니다. 가축은 축사 안에서 자유롭게 움직입니다.");
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
        int sel = gui_renderer_.show_choice("축사", "가축을 배치하거나 달걀을 부화시킵니다. 가축을 바라보고 T를 누르면 바로 상호작용합니다.",
                                            {{1, "닭 배치"}, {2, "돼지 배치"}, {3, "소 배치"}, {4, "달걀 부화"}, {0, "취소"}}, 0);
        std::string msg;
        if (sel == 1) barn_.place_animal(player_, 1, msg);
        else if (sel == 2) barn_.place_animal(player_, 2, msg);
        else if (sel == 3) barn_.place_animal(player_, 3, msg);
        else if (sel == 4) barn_.hatch_egg(player_, msg);
        else msg = "축사 관리를 취소했습니다.";
        renderer_.add_message(msg);
        if (sel != 0) gui_renderer_.show_notice("축사", msg);
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
        renderer_.add_message("바라보는 방향 1칸 앞에 상호작용할 가축이 없습니다. 가축을 바라보고 T를 누르세요.");
#ifdef FARM_GUI
        if (gui_mode_) gui_renderer_.show_notice("가축 상호작용", "바라보는 방향 1칸 앞에 상호작용할 가축이 없습니다.\n가축을 바라보고 T를 누르세요.");
#endif
        return;
    }

    const LivestockInfo* info = LivestockSystem::get_info(animal->tile.animal_type);
    std::string animal_name = info ? info->name : "Animal";
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
    std::cout << animal_name << "와(과) 상호작용\n"
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
    player_.tick_starry_night();
    market_.start_new_day();
    std::string economy_message = player_.start_new_day_economy();

    EventDayReport event_report = event_system_.process_new_day(player_, map_);
    map_.advance_day();
    barn_.advance_day();
    std::string message = "침대에서 잠을 잤습니다. 하루가 지나 작물과 가축이 성장했습니다.";
    if (!economy_message.empty()) message += " " + economy_message;
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

void Game::show_skill_tree() {
#ifdef FARM_GUI
    if (gui_mode_) { show_gui_skill_tree(); return; }
#endif
    while (true) {
        system("clear");
        std::cout << "[스킬 트리]\n";
        std::cout << "보유 스킬 포인트: " << player_.get_skill_points() << "\n\n";
        std::cout << "1. STR 체력/채굴: 1 -> 2/3 -> 4 -> 5/6\n";
        std::cout << "2. INT 생산/거래: 경제 1->2->3->4, 괴식 5->6->7->8\n";
        std::cout << "3. LUK 수확/행운: 1 -> 2/3 -> 4 -> 5/6\n";
        std::cout << "0. 나가기\n>> " << std::flush;
        char sel = getch();
        if (sel == '0') break;
        if (sel == '1') show_str_tree();
        else if (sel == '2') show_int_tree();
        else if (sel == '3') show_luk_tree();
    }
    renderer_.add_message("스킬 트리를 닫았습니다.");
}

void Game::show_str_tree() {
    while (true) {
        system("clear");
        auto st = [&](int id) { return player_.has_str_node(id) ? "[O]" : "[ ]"; };
        std::cout << "[STR] 보유 포인트: " << player_.get_skill_points() << "\n";
        std::cout << st(1) << " 1. 건강한 몸: 최대 스태미나 +5\n";
        std::cout << "   ├ " << st(2) << " 2. 감기 저항: 감기 최대 스태미나 감소 50% -> 20%, 야생동물 출몰 차단\n";
        std::cout << "   └ " << st(3) << " 3. 채굴 집중: 광산 붕괴 시 30% 확률로 이전 보상 보존\n";
        std::cout << "      └ " << st(4) << " 4. 강인한 체력: 최대 스태미나 +10\n";
        std::cout << "         ├ " << st(5) << " 5. 농사 숙련: 수확 시 20% 확률로 스태미나 소모 없음\n";
        std::cout << "         └ " << st(6) << " 6. 광산 숙련: 붕괴 직전 마지막 채굴 무효화, 탈출 보상 2배\n";
        std::cout << "0. 뒤로\n>> " << std::flush;
        char sel = getch();
        if (sel == '0') break;
        int node = sel - '0';
        if (node >= 1 && node <= 6) {
            std::string msg;
            player_.invest_str_node(node, msg);
            std::cout << "\n" << msg << "\n";
            getch();
        }
    }
}

void Game::show_int_tree() {
    while (true) {
        system("clear");
        auto st = [&](int id) { return player_.has_int_node(id) ? "[O]" : "[ ]"; };
        std::cout << "[INT] 보유 포인트: " << player_.get_skill_points() << "\n";
        std::cout << st(1) << " 1. 공정 최적화: 주방/작업장/도구 제작 비용 20% 감소\n";
        std::cout << "   └ " << st(2) << " 2. 재무 설계: 매일 보유 골드의 1% 이자, 최대 100G\n";
        std::cout << "      └ " << st(3) << " 3. 할인 감각: 상점 구매 가격 10% 감소\n";
        std::cout << "         └ " << st(4) << " 4. 협동 조합: 하루 판매 수익의 10%를 다음날 배당금으로 예약\n";
        std::cout << st(5) << " 5. 몬스터 조리: 몬스터 푸드 판매가 10% 증가\n";
        std::cout << "   └ " << st(6) << " 6. 제조 전문가: 몬스터 푸드 판매 성공률 +20%\n";
        std::cout << "      └ " << st(7) << " 7. 손실 방지: 실패 시 50% 확률로 신뢰도 하락 방지\n";
        std::cout << "         └ " << st(8) << " 8. 관리 루틴: 몬스터 푸드 판매 성공률 추가 +20%\n";
        std::cout << "0. 뒤로\n>> " << std::flush;
        char sel = getch();
        if (sel == '0') break;
        int node = sel - '0';
        if (node >= 1 && node <= 8) {
            std::string msg;
            player_.invest_int_node(node, msg);
            std::cout << "\n" << msg << "\n";
            getch();
        }
    }
}

void Game::show_luk_tree() {
    while (true) {
        system("clear");
        auto st = [&](int id) { return player_.has_luk_node(id) ? "[O]" : "[ ]"; };
        std::cout << "[LUK] 보유 포인트: " << player_.get_skill_points() << "\n";
        std::cout << st(1) << " 1. 씨앗 회수: 수확 시 10% 확률로 심은 씨앗 1개 회수\n";
        std::cout << "   ├ " << st(2) << " 2. 축산 행운: 가축 생산물 수확 시 20% 확률로 생산물 +1\n";
        std::cout << "   └ " << st(3) << " 3. 낚시 감각: 각 물고기 획득 확률 +10%\n";
        std::cout << "      └ " << st(4) << " 4. 요리 재능: 일반 요리 시 재료 1개를 무작위로 아낌\n";
        std::cout << "         ├ " << st(5) << " 5. 별자리 기운: 별 내리는 밤 확률 1% -> 10%\n";
        std::cout << "         └ " << st(6) << " 6. 황금 어장: 물고기 확률 추가 +10%, Perfect 낚시 시 10%로 3종 추가 획득\n";
        std::cout << "0. 뒤로\n>> " << std::flush;
        char sel = getch();
        if (sel == '0') break;
        int node = sel - '0';
        if (node >= 1 && node <= 6) {
            std::string msg;
            player_.invest_luk_node(node, msg);
            std::cout << "\n" << msg << "\n";
            getch();
        }
    }
}

void Game::quit() {
#ifdef FARM_GUI
    if (gui_mode_) {
        if (gui_renderer_.confirm("게임 종료", "정말 게임을 종료할까요?\n저장 기능은 아직 없습니다.")) {
            is_running_ = false;
        } else {
            renderer_.add_message("게임 종료를 취소했습니다.");
        }
        return;
    }
#endif
    system("clear");
    std::cout << "정말 게임을 종료할까요? 저장 기능은 아직 없습니다. (Y/N): " << std::flush;
    char ans = getch();
    if (ans == 'y' || ans == 'Y') {
        is_running_ = false;
        std::cout << "\n게임을 종료합니다. 다음에 또 만나요.\n";
    } else renderer_.add_message("게임 종료를 취소했습니다.");
}


#ifdef FARM_GUI
std::vector<std::string> Game::current_event_status_lines() const {
    std::vector<std::string> lines;
    if (event_system_.is_drought_active()) {
        lines.push_back("가뭄 " + std::to_string(event_system_.drought_days_left()) + "일 남음");
        lines.push_back("농지에서 물 주기 필요");
    }
    if (player_.is_starry_night()) {
        lines.push_back("별 내리는 밤 활성화");
    }
    if (player_.is_wildlife_today()) {
        lines.push_back("야생동물 출몰 중");
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
        if (gui_mode_) gui_renderer_.show_notice("게임 종료", "100일간의 플레이가 종료되었습니다.");
#endif
        std::cout << "\n100일간의 플레이가 종료되었습니다.\n";
    }
}


bool Game::has_built_facility(const std::string& name) const {
    for (int y = 0; y < Map::HEIGHT; ++y) {
        for (int x = 0; x < Map::WIDTH; ++x) {
            const Tile& t = map_.get_tile(x, y);
            if (t.building_name == name) return true;
            if (name == "주방" && t.type == TileType::KITCHEN) return true;
            if ((name == "작업장" || name == "Workshop") && t.type == TileType::WORKSHOP) return true;
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
    oss << "플레이어 레벨 " << r.required_player_level << " / " << r.gold << "G / 나무 " << r.wood;
    if (r.copper > 0) oss << " / 구리 " << r.copper;
    if (r.iron > 0) oss << " / 철 " << r.iron;
    if (r.gold_ore > 0) oss << " / 금 " << r.gold_ore;
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
    message = "축사를 2x2 크기로 건설했습니다. 문 앞에서 J를 누르면 축사 안으로 들어갑니다.";
    return true;
}

bool Game::build_storage(Tile& tile, std::string& message) {
    (void)tile;
    if (has_built_facility("저장소") || player_.get_storage_level() > 0) {
        message = "저장소는 이미 건설되어 있습니다. 하나만 지을 수 있습니다.";
        return false;
    }
    StorageUpgradeRule r = storage_rule_for_target(1);
    if (player_.get_level() < r.required_player_level) {
        message = "저장소 레벨 1은 플레이어 레벨 " + std::to_string(r.required_player_level) + "부터 건설할 수 있습니다.";
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
    message = "저장소 레벨 1을 2x2 크기로 건설했습니다. 인벤토리 최대 용량 +" + std::to_string(r.added_slots) + "! 문 앞에서 이용하세요.";
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
        message = "저장소 레벨 " + std::to_string(r.target_level) + " 업그레이드는 플레이어 레벨 " + std::to_string(r.required_player_level) + "부터 가능합니다.";
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
    message = "저장소 레벨 " + std::to_string(r.target_level) + "로 업그레이드했습니다. 인벤토리 최대 용량 +" + std::to_string(r.added_slots) + "!";
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

void Game::show_tutorial() {
    while (tutorial_.is_active()) {
        std::cout << "\033[2J\033[H";
        std::cout << "[" << Tutorial::npc_name() << "] " << tutorial_.current_title() << "\n\n";
        std::cout << tutorial_.current_dialogue() << "\n\n";
        std::cout << "(" << (tutorial_.current_step() + 1) << "/" << tutorial_.total_steps() << ") ";
        if (tutorial_.current_step() + 1 < tutorial_.total_steps()) {
            std::cout << "아무 키나 누르면 다음 설명으로 넘어갑니다.";
        } else {
            std::cout << "아무 키나 누르면 튜토리얼을 마칩니다.";
        }
        std::cout << std::flush;
        wait_key();
        tutorial_.advance();
    }
    renderer_.add_message("튜토리얼 완료! 이제 자유롭게 농장을 운영하세요.");
}


#ifdef FARM_GUI

bool Game::choose_gui_monster_food_ingredients(std::vector<ItemType>& ingredients) {
    struct MonsterIngredientOption {
        ItemType item;
        const char* name;
        const char* icon;
        int rarity;
    };

    static const MonsterIngredientOption options[] = {
        {ItemType::POTATO, "감자", "item_potato", 1},
        {ItemType::CARROT, "당근", "item_carrot", 1},
        {ItemType::EGG, "달걀", "egg", 1},
        {ItemType::FISH_MACKEREL, "고등어", "fish_mackerel", 1},
        {ItemType::WHEAT, "밀", "item_wheat", 2},
        {ItemType::RICE, "쌀", "item_rice", 2},
        {ItemType::PORK, "돼지고기", "pork", 2},
        {ItemType::FISH_FLATFISH, "광어", "fish_flatfish", 2},
        {ItemType::APPLE, "사과", "item_apple", 3},
        {ItemType::MILK, "우유", "milk", 3},
        {ItemType::FISH_TUNA, "참치", "fish_tuna", 3},
    };

    auto selected_count = [&](ItemType item) {
        int count = 0;
        for (ItemType selected : ingredients) {
            if (selected == item) ++count;
        }
        return count;
    };

    while (true) {
        int rarity_sum = 0;
        std::ostringstream body;
        body << "괴식은 정식 레시피가 아닌 재료 2~6개로 만듭니다.\n";
        body << "선택한 재료 (" << ingredients.size() << "/6): ";
        if (ingredients.empty()) body << "없음";
        for (ItemType item : ingredients) {
            body << item_name(item) << " ";
            for (const auto& option : options) {
                if (option.item == item) {
                    rarity_sum += option.rarity;
                    break;
                }
            }
        }

        std::string grade = "하급";
        if (rarity_sum > 10) grade = "고급";
        else if (rarity_sum > 5) grade = "중급";
        body << "\n희귀도 합: " << rarity_sum;
        if (ingredients.size() >= 2) body << " / 예상 결과: " << grade << " 몬스터 푸드";
        else body << " / 재료를 최소 2개 선택하세요.";

        std::vector<GuiActionEntry> actions;
        int id = 1;
        if (ingredients.size() < 6) {
            for (const auto& option : options) {
                int owned = player_.count_item(option.item);
                int chosen = selected_count(option.item);
                if (owned <= chosen) continue;
                std::ostringstream label;
                label << "희귀도 " << option.rarity << " / 남은 수량 " << (owned - chosen) << "개";
                actions.push_back({id, option.name, label.str(), option.icon, false});
                ++id;
            }
        }
        if (ingredients.size() >= 2) actions.push_back({100, "제작 시작", "선택한 재료로 괴식을 만듭니다.", "kitchen", false});
        if (!ingredients.empty()) actions.push_back({99, "선택 초기화", "선택한 재료를 모두 비웁니다.", "kitchen", false});
        actions.push_back({0, "취소", "주방으로 돌아갑니다.", "kitchen", false});

        std::vector<GuiActionEntry> recipes = actions;
        int choice = gui_renderer_.show_action_panel("괴식 조합", body.str(), "선택 가능 재료", actions, recipes, 0);
        if (choice == 0) return false;
        if (choice == 100) return true;
        if (choice == 99) {
            ingredients.clear();
            continue;
        }

        int current_id = 1;
        for (const auto& option : options) {
            int owned = player_.count_item(option.item);
            int chosen = selected_count(option.item);
            if (owned <= chosen) continue;
            if (choice == current_id) {
                ingredients.push_back(option.item);
                break;
            }
            ++current_id;
        }
    }
}

void Game::handle_gui_market() {
    while (true) {
        int mode = gui_renderer_.show_choice("상점", "거래 방식을 선택하세요.", {{1, "구매"}, {2, "판매"}, {0, "뒤로"}}, 0);
        if (mode == 0) { renderer_.add_message("상점을 닫았습니다."); return; }

        if (mode == 1) {
            while (true) {
                std::vector<GuiShopEntry> entries;
                const auto& items = market_.buy_items();
                for (std::size_t i = 0; i < items.size(); ++i) {
                    const auto& item = items[i];
                    std::ostringstream sub;
                    int price = player_.discounted_market_buy_price(item.buy_price);
                    bool trust_locked = player_.is_shop_restricted() && item.required_level > 1;
                    bool locked = player_.get_level() < item.required_level || trust_locked;
                    sub << price << "G";
                    if (price < item.buy_price) sub << " | INT 할인";
            if (player_.get_level() < item.required_level) sub << " | 레벨 " << item.required_level;
                    if (trust_locked) sub << " | 신뢰도 제한";
                    entries.push_back({static_cast<int>(i + 1), item.item, item.name, sub.str(), locked});
                }
                int choice = gui_renderer_.show_shop_selector("구매 상점", "인벤토리", "구매 가능 품목", entries, player_, "신뢰도 " + std::to_string(player_.get_shop_trust()) + "/100. 신뢰도가 낮으면 고급 품목이 잠깁니다.", 0);
                if (choice <= 0) { renderer_.add_message("상점을 닫았습니다."); return; }

                const auto& item = items[choice - 1];
                int price = player_.discounted_market_buy_price(item.buy_price);
                int max_qty = price <= 0 ? 999 : player_.get_money() / price;
                if (max_qty <= 0) {
                    std::string msg = "골드가 부족합니다.";
                    renderer_.add_message(msg);
                    gui_renderer_.show_notice("구매 실패", msg);
                    continue;
                }

                std::ostringstream qmsg;
                qmsg << item.name << "을(를) 몇 개 구매할까요?\n"
                     << "가격: " << price << "G | 보유: " << player_.count_item(item.item) << "\n"
                     << "최대 수량: " << max_qty;
                int qty = gui_renderer_.prompt_quantity("구매 수량", qmsg.str(), max_qty, 1);
                if (qty <= 0) { renderer_.add_message("구매를 취소했습니다."); continue; }

                std::string msg;
                market_.buy_item(player_, static_cast<std::size_t>(choice - 1), qty, msg);
                renderer_.add_message(msg);
                gui_renderer_.show_notice("구매 결과", msg);
            }
        }

        if (mode == 2) {
            while (true) {
                std::vector<GuiShopEntry> entries;
                const auto& items = market_.sell_items();
                for (std::size_t i = 0; i < items.size(); ++i) {
                    const auto& item = items[i];
                    bool monster = item.item == ItemType::MONSTER_FOOD_LOW ||
                                   item.item == ItemType::MONSTER_FOOD_MID ||
                                   item.item == ItemType::MONSTER_FOOD_HIGH;
                    int price = monster ? player_.monster_food_sell_price(item.sell_price) : item.sell_price;
                    std::ostringstream sub;
                    sub << price << "G";
                    if (monster) sub << " | 성공률 " << player_.monster_food_success_chance() << "%";
                    if (player_.count_item(item.item) <= 0) sub << " | 보유 없음";
                    entries.push_back({static_cast<int>(i + 1), item.item, item.name, sub.str(), player_.count_item(item.item) <= 0});
                }
                int choice = gui_renderer_.show_shop_selector("판매 상점", "인벤토리", "판매 가능 품목", entries, player_, "신뢰도 " + std::to_string(player_.get_shop_trust()) + "/100. 몬스터 푸드는 실패 시 신뢰도가 내려갈 수 있습니다.", 0);
                if (choice <= 0) { renderer_.add_message("상점을 닫았습니다."); return; }

                const auto& item = items[choice - 1];
                int max_qty = player_.count_item(item.item);
                if (max_qty <= 0) {
                    std::string msg = "이 아이템을 보유하고 있지 않습니다.";
                    renderer_.add_message(msg);
                    gui_renderer_.show_notice("판매 실패", msg);
                    continue;
                }

                std::ostringstream qmsg;
                qmsg << item.name << "을(를) 몇 개 판매할까요?\n"
                     << "기본 가격: " << item.sell_price << "G | 보유: " << max_qty;
                int qty = gui_renderer_.prompt_quantity("판매 수량", qmsg.str(), max_qty, max_qty);
                if (qty <= 0) { renderer_.add_message("판매를 취소했습니다."); continue; }

                std::string msg;
                market_.sell_item(player_, static_cast<std::size_t>(choice - 1), qty, msg);
                renderer_.add_message(msg);
                gui_renderer_.show_notice("판매 결과", msg);
            }
        }
    }
}


void Game::handle_gui_farm(Tile& tile) {
    std::string msg;
    if (!tile.has_crop()) {
        std::ostringstream body;
        body << "심을 씨앗을 선택하세요.\n"
             << "감자 씨앗: " << player_.count_item(ItemType::SEED_POTATO) << "\n"
             << "당근 씨앗: " << player_.count_item(ItemType::SEED_CARROT) << "\n"
             << "밀 씨앗: " << player_.count_item(ItemType::SEED_WHEAT) << "\n"
             << "쌀 씨앗: " << player_.count_item(ItemType::SEED_RICE) << "\n"
             << "사과 묘목: " << player_.count_item(ItemType::SEED_APPLE);
        int crop_id = gui_renderer_.show_choice("농경지", body.str(), {{1, "감자"}, {2, "당근"}, {3, "밀"}, {4, "쌀"}, {5, "사과"}, {0, "취소"}}, 0);
        if (crop_id == 0) {
            renderer_.add_message("씨앗 심기를 취소했습니다.");
            return;
        }
        bool ok = CropSystem::plant_crop(player_, tile, crop_id, msg);
        renderer_.add_message(msg);
        if (!ok) gui_renderer_.show_notice("농경지", msg);
        return;
    }

    std::ostringstream body;
    body << "성장: " << tile.crop_progress << "/" << tile.crop_max_progress << "일\n";
    if (tile.crop_progress < tile.crop_max_progress) body << "작물이 아직 자라는 중입니다.";
    std::vector<std::pair<int, std::string>> buttons{{1, "수확"}};
    if (event_system_.is_drought_active()) buttons.push_back({2, "물 주기"});
    buttons.push_back({9, "작물 제거"});
    buttons.push_back({0, "취소"});
    int sel = gui_renderer_.show_choice("농경지", body.str(), buttons, 0);
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
        body << "가축을 배치하거나 달걀을 부화시킵니다.\n"
             << "병아리: " << player_.count_item(ItemType::BABY_CHICKEN) << "\n"
             << "새끼 돼지: " << player_.count_item(ItemType::BABY_PIG) << "\n"
             << "송아지: " << player_.count_item(ItemType::BABY_COW) << "\n"
             << "달걀: " << player_.count_item(ItemType::EGG);
        int sel = gui_renderer_.show_choice("가축", body.str(), {{1, "닭 배치"}, {2, "돼지 배치"}, {3, "소 배치"}, {4, "달걀 부화"}, {0, "취소"}}, 0);
        if (sel == 1) LivestockSystem::place_animal(player_, tile, 1, msg);
        else if (sel == 2) LivestockSystem::place_animal(player_, tile, 2, msg);
        else if (sel == 3) LivestockSystem::place_animal(player_, tile, 3, msg);
        else if (sel == 4) LivestockSystem::hatch_egg(player_, msg);
        else msg = "작업을 취소했습니다.";
        renderer_.add_message(msg);
        gui_renderer_.show_notice("가축", msg);
        return;
    }

    int sel = gui_renderer_.show_choice("가축", "이 가축을 관리합니다.", {{1, "생산물 얻기"}, {2, "가축 제거"}, {0, "취소"}}, 0);
    if (sel == 1) LivestockSystem::collect_product(player_, tile, msg);
    else if (sel == 2) {
        if (gui_renderer_.confirm("가축 제거", "정말 이 가축을 제거할까요?")) LivestockSystem::remove_animal(tile, msg);
        else msg = "가축 제거를 취소했습니다.";
    } else msg = "작업을 취소했습니다.";
    renderer_.add_message(msg);
    gui_renderer_.show_notice("가축", msg);
}

void Game::handle_gui_building(Tile& tile) {
    if (!tile.building_name.empty()) {
        gui_renderer_.show_notice("건물 이용", "건물 위로는 이동할 수 없습니다. 건물 아래쪽 문 앞에서 J/Enter로 이용하세요.");
        return;
    }
    int sel = gui_renderer_.show_choice("건설 부지", "건설할 시설을 선택하세요. 축사는 시작부터 설치되어 있습니다.", {{1, "주방"}, {2, "작업장"}, {3, "저장소"}, {0, "취소"}}, 0);
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
    while (true) {
        std::ostringstream status;
        status << "주방 레벨 " << kitchen_.get_level()
               << " | 골드 " << player_.get_money() << "G"
                 << " | 스태미나 " << player_.get_stamina() << "/" << player_.get_stamina_max()
               << "\n보유: 감자 " << player_.count_item(ItemType::POTATO)
               << ", 당근 " << player_.count_item(ItemType::CARROT)
               << ", 달걀 " << player_.count_item(ItemType::EGG)
               << ", 생선 " << (player_.count_item(ItemType::FISH_MACKEREL) + player_.count_item(ItemType::FISH_FLATFISH) + player_.count_item(ItemType::FISH_TUNA));

        const bool lv2_locked = kitchen_.get_level() < 2;
        const bool lv3_locked = kitchen_.get_level() < 3;
        const bool lv4_locked = kitchen_.get_level() < 4;
        const bool upgrade_locked = kitchen_.get_level() >= 4;
        std::vector<GuiActionEntry> actions{
            {1, "감자 수프", "감자 2", "food_potato_soup", false},
            {2, "당근 달걀 롤", "당근 2 + 달걀", "food_carrot_egg_roll", false},
            {3, "고등어 당근 조림", "고등어 + 당근 2", "food_mackerel_carrot", false},
            {4, "돼지고기 감자 짜글이", "레벨 2 / 돼지고기 + 감자 2", "food_pork_potato", lv2_locked},
            {5, "감자 밀 부침개", "레벨 2 / 밀 2 + 감자 + 달걀", "food_potato_wheat_pancake", lv2_locked},
            {6, "당근 광어 스테이크", "레벨 2 / 광어 + 당근 2", "food_carrot_flatfish", lv2_locked},
            {7, "3색 당근 쌀밥", "레벨 3 / 쌀 2 + 당근 + 달걀", "food_3color_carrot_rice", lv3_locked},
            {8, "감자 매쉬드 비프", "레벨 3 / 우유 + 돼지고기 + 감자 2", "food_mashed_beef", lv3_locked},
            {9, "당근 사과 주스 만찬", "레벨 3 / 사과 2 + 당근 + 우유", "food_carrot_apple_juice", lv3_locked},
            {10, "참치 감자 스테이크", "레벨 4 / 참치 + 감자 2 + 당근", "food_tuna_potato", lv4_locked},
            {11, "특선 하이라이스", "레벨 4 / 쌀 2 + 당근 2 + 돼지고기 + 우유", "food_hailight_rice", lv4_locked},
            {12, "사과 감자 무스 케이크", "레벨 4 / 사과 2 + 밀 2 + 감자 2 + 우유", "food_apple_potato_cake", lv4_locked},
            {13, upgrade_locked ? "주방 최고 레벨" : "주방 업그레이드", upgrade_locked ? "더 이상 업그레이드할 수 없습니다." : "골드와 플레이어 레벨 필요", "kitchen", upgrade_locked},
            {14, "괴식 조합", "정식 레시피가 아닌 재료 2~6개 선택", "monster_food_mid", false}
        };
        std::vector<GuiActionEntry> recipes = actions;

        int sel = gui_renderer_.show_action_panel("주방", status.str(), "레시피 / 조건", actions, recipes, 0);
        if (sel == 0) { renderer_.add_message("주방 이용을 취소했습니다."); return; }
        std::string msg;
        if (sel == 14) {
            std::vector<ItemType> ingredients;
            if (!choose_gui_monster_food_ingredients(ingredients)) {
                renderer_.add_message("괴식 조합을 취소했습니다.");
                continue;
            }

            std::string recipe_name = kitchen_.get_recipe_name(ingredients);
            if (!recipe_name.empty()) {
                msg = "선택한 조합은 정식 레시피(" + recipe_name + ")입니다. 괴식은 레시피와 다른 조합만 가능합니다.";
            } else {
                kitchen_.cook(player_, ingredients, msg);
            }
        } else {
            kitchen_.perform_action(player_, sel, msg);
        }
        sync_facility_level("주방", kitchen_.get_level());
        renderer_.add_message(msg);
        gui_renderer_.show_notice("주방 결과", msg);
    }
}
void Game::handle_gui_workshop() {
    while (true) {
        std::ostringstream status;
        status << "작업장 레벨 " << workshop_.get_level()
               << " | 괭이 " << player_.get_hoe_grade()
               << " | 낚싯대 " << player_.get_rod_grade()
               << " | 곡괭이 " << player_.get_pick_grade()
               << " | 플레이어 레벨 " << player_.get_level()
               << "\n보유: 나무 " << player_.count_item(ItemType::WOOD)
               << ", 구리 " << player_.count_item(ItemType::COPPER)
               << ", 철 " << player_.count_item(ItemType::IRON)
               << ", 금 " << player_.count_item(ItemType::GOLD)
               << ", 골드 " << player_.get_money() << "G";

        int need_level = 0;
        std::string expand_req;
        if (workshop_.get_level() == 0) { need_level = 6; expand_req = "Lv6 / 200G / 스태미나 8"; }
        else if (workshop_.get_level() == 1) { need_level = 10; expand_req = "Lv10 / 600G / 스태미나 15"; }
        else if (workshop_.get_level() == 2) { need_level = 15; expand_req = "Lv15 / 1200G / 스태미나 20"; }
        else { need_level = 999; expand_req = "최고 레벨"; }

        const bool expand_locked = workshop_.get_level() >= 3 || player_.get_level() < need_level;
        std::vector<GuiActionEntry> actions{
            {1, workshop_.get_level() >= 3 ? "작업장 최고 레벨" : "작업장 증축", expand_req, "workshop", expand_locked},
            {2, "구리 괭이", "작업장 레벨 1 / 100G / 나무8 / 구리4", "copper", workshop_.get_level() < 1 || player_.get_hoe_grade() >= 1},
            {3, "철 괭이", "작업장 레벨 2 / 300G / 나무15 / 철5", "iron", workshop_.get_level() < 2 || player_.get_hoe_grade() >= 2},
            {4, "금 괭이", "작업장 레벨 3 / 1000G / 나무30 / 금5", "gold", workshop_.get_level() < 3 || player_.get_hoe_grade() >= 3},
            {5, "구리 낚싯대", "작업장 레벨 1 / 100G / 나무12 / 구리2", "copper", workshop_.get_level() < 1 || player_.get_rod_grade() >= 1},
            {6, "철 낚싯대", "작업장 레벨 2 / 300G / 나무25 / 철3", "iron", workshop_.get_level() < 2 || player_.get_rod_grade() >= 2},
            {7, "금 낚싯대", "작업장 레벨 3 / 1000G / 나무50 / 금2", "gold", workshop_.get_level() < 3 || player_.get_rod_grade() >= 3},
            {8, "구리 곡괭이", "작업장 레벨 1 / 100G / 나무10 / 구리3", "copper", workshop_.get_level() < 1 || player_.get_pick_grade() >= 1},
            {9, "철 곡괭이", "작업장 레벨 2 / 300G / 나무30 / 철4", "iron", workshop_.get_level() < 2 || player_.get_pick_grade() >= 2},
            {10, "금 곡괭이", "작업장 레벨 3 / 1000G / 나무40 / 금3", "gold", workshop_.get_level() < 3 || player_.get_pick_grade() >= 3}
        };
        std::vector<GuiActionEntry> recipes = actions;

        int sel = gui_renderer_.show_action_panel("작업장", status.str(), "제작식 / 조건", actions, recipes, 0);
        if (sel == 0) { renderer_.add_message("작업장 이용을 취소했습니다."); return; }
        std::string msg;
        workshop_.perform_action(player_, sel, msg);
        sync_facility_level("작업장", workshop_.get_level());
        renderer_.add_message(msg);
        gui_renderer_.show_notice("작업장 결과", msg);
    }
}
void Game::handle_gui_storage() {
    int current = player_.get_storage_level();
    std::ostringstream status;
    status << "저장소 레벨 " << current
           << " | 인벤토리 " << player_.inventory().used_slots() << "/" << player_.inventory().max_slots()
           << " | 플레이어 레벨 " << player_.get_level()
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
        {1, "레벨 1 저장소", storage_rule_text(storage_rule_for_target(1)), "storage", current >= 1},
        {2, "레벨 2 저장소", storage_rule_text(storage_rule_for_target(2)), "storage", current >= 2 || player_.get_level() < 8},
        {3, "레벨 3 저장소", storage_rule_text(storage_rule_for_target(3)), "storage", current >= 3 || player_.get_level() < 16}
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

void Game::show_gui_skill_tree() {
    std::ostringstream intro;
    intro << "보유 스킬 포인트: " << player_.get_skill_points()
          << "\n원하는 계열을 선택하면 세부 노드와 선행 조건을 볼 수 있습니다."
          << "\nSTR: 체력과 광산 안정성"
          << "\nINT: 제작/거래와 몬스터 푸드"
          << "\nLUK: 수확, 축산, 낚시, 요리 행운";
    int tree = gui_renderer_.show_choice("스킬 트리", intro.str(),
        {{1, "STR 체력/채굴"}, {2, "INT 제작/거래"}, {3, "LUK 수확/행운"}}, 0);
    if (tree == 0) {
        renderer_.add_message("스킬 트리를 닫았습니다.");
        return;
    }

    std::vector<GuiSkillNode> nodes;
    std::string title;
    std::string status;

    if (tree == 1) {
        title = "STR 스킬 트리";
        status = "체력과 광산 안정성을 강화합니다. 노드를 클릭하면 오른쪽에 효과가 표시되고, 투자하기로 습득합니다.";
        auto learned = [&](int id) { return player_.has_str_node(id); };
        auto can = [&](int id) {
            if (id == 1) return true;
            if (id == 2 || id == 3) return learned(1);
            if (id == 4) return learned(2) || learned(3);
            return learned(4);
        };
        nodes = {
            {1, 260, 70, "1 건강한 몸", "최대 스태미나가 5 증가합니다. STR 계열의 시작 노드입니다.", "energy", learned(1), !learned(1) && !can(1), {}},
            {2, 90, 185, "2 감기 저항", "감기 페널티를 완화해 오늘 최대 스태미나가 80%까지 유지되고, 야생동물 출몰을 막습니다. 필요: 건강한 몸.", "cold", learned(2), !learned(2) && !can(2), {1}},
            {3, 430, 185, "3 채굴 집중", "광산 붕괴 시 30% 확률로 직전 채굴을 포기하고 이전 보상을 보존합니다. 필요: 건강한 몸.", "mine", learned(3), !learned(3) && !can(3), {1}},
            {4, 260, 300, "4 강인한 체력", "최대 스태미나가 10 추가로 증가합니다. 필요: 감기 저항 또는 채굴 집중.", "energy", learned(4), !learned(4) && !can(4), {2, 3}},
            {5, 90, 415, "5 농사 숙련", "수확할 때 20% 확률로 스태미나를 소모하지 않습니다. 필요: 강인한 체력.", "hand", learned(5), !learned(5) && !can(5), {4}},
            {6, 430, 415, "6 광산 숙련", "광산 붕괴 직전 마지막 채굴을 무효화하고 탈출 보상을 2배로 만듭니다. 필요: 강인한 체력.", "mine", learned(6), !learned(6) && !can(6), {4}}
        };
    } else if (tree == 2) {
        title = "INT 스킬 트리";
        status = "제작/거래 경제 가지와 몬스터 푸드 가지로 나뉩니다. 노드를 클릭해 효과와 선행 조건을 확인하세요.";
        auto learned = [&](int id) { return player_.has_int_node(id); };
        auto can = [&](int id) {
            if (id == 1 || id == 5) return true;
            if (id >= 2 && id <= 4) return learned(id - 1);
            if (id >= 6 && id <= 8) return learned(id - 1);
            return false;
        };
        nodes = {
            {1, 105, 48, "1 공정 최적화", "주방, 작업장, 도구 제작 비용이 20% 감소합니다. 경제 가지의 시작 노드입니다.", "workshop", learned(1), !learned(1) && !can(1), {}},
            {2, 105, 158, "2 재무 설계", "하루 시작 시 보유 골드의 1%를 이자로 받습니다. 하루 최대 100G입니다. 필요: 공정 최적화.", "coin", learned(2), !learned(2) && !can(2), {1}},
            {3, 105, 268, "3 할인 감각", "상점 구매 가격이 10% 감소합니다. 필요: 재무 설계.", "shop_icon", learned(3), !learned(3) && !can(3), {2}},
            {4, 105, 378, "4 협동 조합", "하루 판매 수익의 10%를 다음 날 배당금으로 예약합니다. 필요: 할인 감각.", "coin", learned(4), !learned(4) && !can(4), {3}},
            {5, 425, 48, "5 몬스터 조리", "몬스터 푸드 판매 가격이 10% 증가합니다. 괴식 가지의 시작 노드입니다.", "kitchen", learned(5), !learned(5) && !can(5), {}},
            {6, 425, 158, "6 제조 전문가", "몬스터 푸드 판매 성공률이 20% 증가합니다. 필요: 몬스터 조리.", "kitchen", learned(6), !learned(6) && !can(6), {5}},
            {7, 425, 268, "7 손실 방지", "몬스터 푸드 판매 실패 시 50% 확률로 상점 신뢰도 하락을 막습니다. 필요: 제조 전문가.", "hand", learned(7), !learned(7) && !can(7), {6}},
            {8, 425, 378, "8 관리 루틴", "몬스터 푸드 판매 성공률이 추가로 20% 증가합니다. 필요: 손실 방지.", "level", learned(8), !learned(8) && !can(8), {7}}
        };
    } else {
        title = "LUK 스킬 트리";
        status = "수확, 축산, 낚시, 요리에 보너스를 주는 행운 계열입니다. 원하는 노드를 클릭하면 자세한 효과가 표시됩니다.";
        auto learned = [&](int id) { return player_.has_luk_node(id); };
        auto can = [&](int id) {
            if (id == 1) return true;
            if (id == 2 || id == 3) return learned(1);
            if (id == 4) return learned(2) || learned(3);
            return learned(4);
        };
        nodes = {
            {1, 260, 70, "1 씨앗 회수", "수확 시 10% 확률로 방금 심었던 씨앗 1개를 회수합니다. LUK 계열의 시작 노드입니다.", "seed_potato", learned(1), !learned(1) && !can(1), {}},
            {2, 90, 185, "2 축산 행운", "가축 생산물을 수확할 때 20% 확률로 생산물을 1개 더 얻습니다. 필요: 씨앗 회수.", "chicken", learned(2), !learned(2) && !can(2), {1}},
            {3, 430, 185, "3 낚시 감각", "각 물고기 획득 확률이 10%씩 증가합니다. 필요: 씨앗 회수.", "fish_mackerel", learned(3), !learned(3) && !can(3), {1}},
            {4, 260, 300, "4 요리 재능", "일반 요리 시 재료 1개를 무작위로 아낍니다. 필요: 축산 행운 또는 낚시 감각.", "kitchen", learned(4), !learned(4) && !can(4), {2, 3}},
            {5, 90, 415, "5 별자리 기운", "별 내리는 밤 발생 확률이 1%에서 10%로 증가합니다. 필요: 요리 재능.", "xp", learned(5), !learned(5) && !can(5), {4}},
            {6, 430, 415, "6 황금 어장", "물고기 획득 확률이 추가로 10% 증가하고, Perfect 낚시 시 10% 확률로 생선 3종을 추가 획득합니다. 필요: 요리 재능.", "fish_tuna", learned(6), !learned(6) && !can(6), {4}}
        };
    }

    int node = gui_renderer_.show_skill_tree_panel(title, status, nodes, 0);
    if (node == 0) {
        renderer_.add_message("스킬 투자를 취소했습니다.");
        return;
    }

    std::string msg;
    if (tree == 1) player_.invest_str_node(node, msg);
    else if (tree == 2) player_.invest_int_node(node, msg);
    else player_.invest_luk_node(node, msg);
    renderer_.add_message(msg);
    gui_renderer_.show_notice("스킬 결과", msg);
}

void Game::show_gui_tutorial() {
    gui_renderer_.run_tutorial(player_, map_, tutorial_);
    renderer_.add_message("튜토리얼 완료! 이제 자유롭게 농장을 운영하세요.");
}
#endif




