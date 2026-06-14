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
            show_market_menu();
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
        show_market_menu();
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

void Game::show_market_menu() {
    // 신용도 0 → 입장 거절 대사 + 선물하기만 가능
    if (player_.get_shop_trust() <= 0) {
        system("clear");
        std::cout << "\"더는 못 참아! 당장 내 상점에서 나가! 자네가 가져오는 건 이제 냄새만 맡아도 속이 메스꺼우니, 꼴도 보기 싫으니까 당장 꺼지게!\"\n\n";
        std::cout << "1. 선물하기\n0. 나가기\n>> " << std::flush;
        char sel = getch();
        if (sel == '1') market_.show_gift_shop(player_, kitchen_.get_level());
        return;
    }

    while (true) {
        system("clear");
        std::cout << "[상점] 신용도: " << player_.get_shop_trust() << "/100\n";
        std::cout << "1. 구매\n2. 판매\n3. 괴식 판매\n4. 선물하기\n0. 나가기\n>> " << std::flush;

        char sel = getch();
        if (sel == '0') return;
        if (sel == '1') market_.show_buy_shop(player_);
        else if (sel == '2') market_.show_sell_shop(player_);
        else if (sel == '3') market_.show_monster_food_shop(player_, kitchen_.get_level());
        else if (sel == '4') market_.show_gift_shop(player_, kitchen_.get_level());

        // 판매 도중 신용도 0이 됐으면 자동 퇴장
        if (player_.get_shop_trust() <= 0) return;
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
        system("clear");
        std::cout << "[" << Tutorial::npc_name() << "] " << tutorial_.current_title() << "\n\n";
        std::cout << tutorial_.current_dialogue() << "\n\n";
        std::cout << "(" << (tutorial_.current_step() + 1) << "/" << tutorial_.total_steps() << ") ";
        if (tutorial_.current_step() + 1 < tutorial_.total_steps()) {
            std::cout << "Enter를 누르면 다음으로..." << std::flush;
        } else {
            std::cout << "Enter를 누르면 게임을 시작합니다!" << std::flush;
        }
        getch();
        tutorial_.advance();
    }
    system("clear");
    renderer_.add_message("튜토리얼 완료! 이제 자유롭게 농장을 운영하세요.");
}

#ifdef FARM_GUI

bool Game::choose_gui_free_cook_ingredients(std::vector<ItemType>& ingredients) {
    // 괴식 미발견: 자유 요리 모드 (예상 결과 ???)
    // 괴식 발견 후: 괴식 제작 모드 (예상 결과 등급 표시)
    bool already_made = player_.has_made_monster_food();

    struct FreeIngOption {
        ItemType item;
        const char* name;
        const char* icon;
        int rarity;
    };
    static const FreeIngOption options[] = {
        {ItemType::POTATO,       "감자",   "item_potato",   1},
        {ItemType::CARROT,       "당근",   "item_carrot",   1},
        {ItemType::EGG,          "달걀",   "egg",           1},
        {ItemType::FISH_MACKEREL,"고등어", "fish_mackerel", 1},
        {ItemType::WHEAT,        "밀",     "item_wheat",    2},
        {ItemType::RICE,         "쌀",     "item_rice",     2},
        {ItemType::PORK,         "돼지고기","pork",          2},
        {ItemType::FISH_FLATFISH,"광어",   "fish_flatfish", 2},
        {ItemType::APPLE,        "사과",   "item_apple",    3},
        {ItemType::MILK,         "우유",   "milk",          3},
        {ItemType::FISH_TUNA,    "참치",   "fish_tuna",     3},
    };

    auto selected_count = [&](ItemType item) {
        int count = 0;
        for (ItemType s : ingredients) if (s == item) ++count;
        return count;
    };

    const std::string panel_title = already_made ? "괴식 제작" : "자유 요리";

    while (true) {
        int rarity_sum = 0;
        for (ItemType item : ingredients) {
            for (const auto& opt : options)
                if (opt.item == item) { rarity_sum += opt.rarity; break; }
        }

        std::ostringstream body;
        if (already_made) {
            body << "괴식은 정식 레시피가 아닌 재료 2~6개로 만듭니다.\n";
        } else {
            body << "재료를 자유롭게 선택하여 요리해봅시다!\n";
        }
        body << "선택한 재료 (" << ingredients.size() << "/6): ";
        if (ingredients.empty()) body << "없음";
        for (ItemType item : ingredients) body << item_name(item) << " ";

        // 예상 결과
        if (already_made) {
            if (ingredients.size() >= 2) {
                std::string grade = rarity_sum <= 5 ? "하급" : (rarity_sum <= 10 ? "중급" : "고급");
                body << "\n예상 결과: " << grade << " 괴식";
            } else {
                body << "\n예상 결과: 재료를 최소 2개 선택하세요.";
            }
        } else {
            body << "\n예상 결과: ???";
            if (ingredients.size() < 2) body << " (재료를 최소 2개 선택하세요)";
        }

        std::vector<GuiActionEntry> actions;
        int id = 1;
        if (ingredients.size() < 6) {
            for (const auto& opt : options) {
                int owned  = player_.count_item(opt.item);
                int chosen = selected_count(opt.item);
                if (owned <= chosen) continue;
                std::ostringstream label;
                if (already_made)
                    label << "희귀도 " << opt.rarity << " / 남은 수량 " << (owned - chosen) << "개";
                else
                    label << "남은 수량 " << (owned - chosen) << "개";
                actions.push_back({id, opt.name, label.str(), opt.icon, false});
                ++id;
            }
        }
        const std::string start_label = already_made ? "제작 시작" : "요리 시작";
        const std::string start_sub   = already_made ? "선택한 재료로 괴식을 만듭니다." : "선택한 재료로 요리합니다.";
        if (ingredients.size() >= 2) actions.push_back({100, start_label, start_sub, "kitchen", false});
        if (!ingredients.empty())    actions.push_back({99, "선택 초기화", "선택한 재료를 모두 비웁니다.", "kitchen", false});
        actions.push_back({0, "취소", "주방으로 돌아갑니다.", "kitchen", false});

        std::vector<GuiActionEntry> recipes = actions;
        int choice = gui_renderer_.show_action_panel(panel_title, body.str(), "선택 가능 재료", actions, recipes, 0);
        if (choice == 0) return false;
        if (choice == 100) return true;
        if (choice == 99) { ingredients.clear(); continue; }

        int cur_id = 1;
        for (const auto& opt : options) {
            int owned  = player_.count_item(opt.item);
            int chosen = selected_count(opt.item);
            if (owned <= chosen) continue;
            if (choice == cur_id) { ingredients.push_back(opt.item); break; }
            ++cur_id;
        }
    }
}

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

// ── GUI 상점용 신용도 헬퍼 ──────────────────────────────────────
namespace {
int gui_trust_tier(int trust) {
    if (trust <= 0)  return 0;
    if (trust <= 30) return 1;
    if (trust <= 70) return 2;
    return 3;
}
const char* gui_entry_line(int tier) {
    switch (tier) {
        case 3: return "\"우리 마을 최고의 농부 아닌가! 어서오게나.\"";
        case 2: return "\"왔나. 필요한 물건 있으면 골라보게.\"";
        case 1: return "\"또 너냐? 이 사기꾼 놈, 내 상점에서 얼른 나가!\"";
        default: return "\"더는 못 참아! 당장 내 상점에서 나가!\n자네가 가져오는 건 이제 냄새만 맡아도 속이 메스꺼우니,\n꼴도 보기 싫으니까 당장 꺼지게!\"";
    }
}
const char* gui_success_line(int tier) {
    switch (tier) {
        case 3: return "\"오, 이게 말로만 듣던 그 귀한 고급 요리란 말이지?\n자네가 만든 거니 의심 없이 비싸게 사지! 자, 여기 있네!\"";
        case 2: return "\"음... 저번에 준 음식은 가족들이 먹고 배가 좀 아프다던데...\n이번 건 진짜 확실한 거 맞나? ...하긴, 눈빛을 보니\n거짓말을 하는 것 같진 않군. 이번 딱 한 번만 믿고 사 주겠네.\"";
        case 1: return "\"자네가 가져온 건 쳐다보기도 싫네만...\n윽, 대체 뭘 넣었길래 이렇게 군침 도는 냄새가 나는 거지?\n이번 한 번만 속아 넘어가 줄 테니 당장 돈 받고 나가게!\"";
        default: return "";
    }
}
const char* gui_fail_line(int tier) {
    switch (tier) {
        case 3: return "\"음? 자네 실력답지 않게 오늘은 요리 모양새가 왜 이리 고약한가?\n아무리 자네라지만 이 상태인 걸 돈 주고 사 가긴 곤란하겠어.\n다음엔 제대로 만들어 오게나.\"";
        case 2: return "\"잠깐, 이 냄새는 도저히 사람이 먹을 만한 게 아니잖아!\n자네 요즘 나한테 자꾸 이상한 걸 음식이라고 속여서\n밀어 넘기려는 것 같은데... 내 눈이 호락호락해 보이나?\"";
        case 1: return "\"어디서 또 이 쓰레기를 음식이라고 속여 팔려고 해?!\n경찰에 신고하기 전에 당장 내 눈앞에서 치워!\n이제부터 자네가 가져오는 건 다 쓰레기로 볼 줄 알게!\"";
        default: return "";
    }
}
const char* gui_gift_line(int trust_restore, bool trust_was_reduced) {
    if (trust_restore == 5) {
        return trust_was_reduced
            ? "\"음... 나한테 준 상처가 있으니 아직 완전히 풀린 건 아니네만,\n그래도 이렇게 챙겨주니 자네를 다시 보게 되는구만.\n잘 먹겠네. (신용도 소폭 상승)\""
            : "\"어, 흠... 나를 주려고 가져온 건가? 고, 고맙네.\n갑자기 이런 걸 주다니... 혹시 나한테 무슨 할 말이라도 있는 건가?\n왠지 기분이 묘하게 좋아지는군!\"";
    } else if (trust_restore == 10) {
        return trust_was_reduced
            ? "\"안 그래도 자네를 계속 의심하느라 마음이 복잡했는데,\n이 정성을 보니 자네가 확실히 믿음직스러워졌습니다.\n고맙게 받겠네. (신용도 크게 상승)\""
            : "\"아니, 냄새부터 장난이 아니구만!\n자네, 혹시 매일 내 얼굴 보러 오려고 이렇게 정성을 들이는 건가?\n하하, 이 아저씨 인기가 아직 안 죽었군! 기분이 정말 좋아졌다네!\"";
    } else {
        return trust_was_reduced
            ? "\"이런 엄청난 요리를 나한테 그냥 주다니...\n내가 그동안 자네를 너무 매정하게 몰아세웠나 보군.\n예전처럼 다시 잘 지내보세나. (신용도 대폭 상승)\""
            : "\"맙소사, 이건 국왕 부럽지 않은 최고의 만찬이군!\n...잠깐, 설마 자네 나를... 좋아하는 건가?!\n어흠, 아무리 그래도 우린 상인과 농부 사이일 뿐이네. 어쨌든 고맙네!\"";
    }
}
} // namespace

void Game::handle_gui_market() {
    // ── 신용도 0: 추방 → 선물만 ─────────────────────────────────
    if (player_.get_shop_trust() <= 0) {
        int sel0 = gui_renderer_.show_npc_dialogue("npc_buyer",
            std::string(gui_entry_line(0)) + "\n\n선물로 신용을 회복해야 다시 거래할 수 있습니다.",
            {{1, "선물하기"}, {0, "나가기"}}, 0);
        if (sel0 == 1) handle_gui_gift_shop();
        renderer_.add_message("상점 이용을 마쳤습니다.");
        return;
    }

    // ── 입장: NPC 사진 + 대사 + 메뉴 버튼을 한 화면에 ──────────
    while (true) {
        if (player_.get_shop_trust() <= 0) {
            gui_renderer_.show_npc_dialogue("npc_buyer", gui_entry_line(0),
                {{0, "나가기"}}, 0);
            renderer_.add_message("신용도가 0이 되어 상점에서 쫓겨났습니다.");
            return;
        }

        int tier = gui_trust_tier(player_.get_shop_trust());

        // 기본 2×2: 구매/판매/선물하기/나가기
        // 괴식 판매 생기면 선물하기 앞에 추가 → 2+3 배치
        std::vector<std::pair<int,std::string>> menu_btns;
        menu_btns.push_back({1, "구매"});
        menu_btns.push_back({2, "판매"});
        if (player_.has_made_monster_food())
            menu_btns.push_back({3, "괴식 판매"});
        menu_btns.push_back({4, "선물하기"});
        menu_btns.push_back({0, "나가기"});

        int mode = gui_renderer_.show_npc_dialogue("npc_buyer",
            gui_entry_line(tier), menu_btns, 0);

        if (mode == 0) { renderer_.add_message("상점을 닫았습니다."); return; }

        // ── 1. 구매 ──────────────────────────────────────────
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
                int choice = gui_renderer_.show_shop_selector("구매 상점", "인벤토리", "구매 가능 품목", entries, player_, "", 0);
                if (choice <= 0) break;

                const auto& item = items[static_cast<std::size_t>(choice - 1)];
                int price = player_.discounted_market_buy_price(item.buy_price);
                int max_qty = price <= 0 ? 999 : player_.get_money() / price;
                if (max_qty <= 0) {
                    gui_renderer_.show_notice("구매 실패", "골드가 부족합니다.");
                    continue;
                }
                std::ostringstream qmsg;
                qmsg << item.name << "을(를) 몇 개 구매할까요?\n"
                     << "가격: " << price << "G | 보유: " << player_.count_item(item.item) << "\n최대: " << max_qty;
                int qty = gui_renderer_.prompt_quantity("구매 수량", qmsg.str(), max_qty, 1);
                if (qty <= 0) { renderer_.add_message("구매를 취소했습니다."); continue; }
                std::string msg;
                market_.buy_item(player_, static_cast<std::size_t>(choice - 1), qty, msg);
                renderer_.add_message(msg);
                gui_renderer_.show_notice("구매 결과", msg);
            }
        }

        // ── 2. 일반 판매 (괴식 제외) ──────────────────────────
        if (mode == 2) {
            while (true) {
                std::vector<GuiShopEntry> entries;
                const auto& items = market_.sell_items();
                for (std::size_t i = 0; i < items.size(); ++i) {
                    const auto& item = items[i];
                    bool monster = item.item == ItemType::MONSTER_FOOD_LOW ||
                                   item.item == ItemType::MONSTER_FOOD_MID ||
                                   item.item == ItemType::MONSTER_FOOD_HIGH;
                    if (monster) continue;
                    std::ostringstream sub;
                    sub << item.sell_price << "G";
                    if (player_.count_item(item.item) <= 0) sub << " | 보유 없음";
                    entries.push_back({static_cast<int>(i + 1), item.item, item.name, sub.str(), player_.count_item(item.item) <= 0});
                }
                int choice = gui_renderer_.show_shop_selector("판매 상점", "인벤토리", "판매 가능 품목", entries, player_, "", 0);
                if (choice <= 0) break;

                const auto& item = market_.sell_items()[static_cast<std::size_t>(choice - 1)];
                int max_qty = player_.count_item(item.item);
                if (max_qty <= 0) { gui_renderer_.show_notice("판매 실패", "이 아이템을 보유하고 있지 않습니다."); continue; }
                std::ostringstream qmsg;
                qmsg << item.name << "을(를) 몇 개 판매할까요?\n가격: " << item.sell_price << "G | 보유: " << max_qty;
                int qty = gui_renderer_.prompt_quantity("판매 수량", qmsg.str(), max_qty, max_qty);
                if (qty <= 0) { renderer_.add_message("판매를 취소했습니다."); continue; }
                std::string msg;
                market_.sell_item(player_, static_cast<std::size_t>(choice - 1), qty, msg);
                renderer_.add_message(msg);
                gui_renderer_.show_notice("판매 결과", msg);
            }
        }

        // ── 3. 괴식 판매 ──────────────────────────────────────
        if (mode == 3) {
            handle_gui_monster_food_shop();
            if (player_.get_shop_trust() <= 0) return;
        }

        // ── 4. 선물하기 ───────────────────────────────────────
        if (mode == 4) {
            handle_gui_gift_shop();
        }
    }
}

void Game::handle_gui_monster_food_shop() {
    const ItemType MFOODS[3] = {
        ItemType::MONSTER_FOOD_LOW,
        ItemType::MONSTER_FOOD_MID,
        ItemType::MONSTER_FOOD_HIGH,
    };
    const char* MNAMES[3] = {"하급 몬스터 푸드", "중급 몬스터 푸드", "고급 몬스터 푸드"};
    const int   MPRICES[3] = {200, 500, 2000};

    while (true) {
        if (player_.get_shop_trust() <= 0) {
            gui_renderer_.show_notice("상점 주인", gui_entry_line(0));
            renderer_.add_message("신용도가 0이 되어 상점에서 쫓겨났습니다.");
            return;
        }

        std::vector<GuiShopEntry> entries;
        for (int i = 0; i < 3; ++i) {
            int have  = player_.count_item(MFOODS[i]);
            int price = player_.monster_food_sell_price(MPRICES[i]);
            std::ostringstream sub;
            sub << price << "G";
            if (have <= 0) sub << " | 보유 없음";
            entries.push_back({i + 1, MFOODS[i], MNAMES[i], sub.str(), have <= 0});
        }

        int choice = gui_renderer_.show_shop_selector(
            "괴식 판매", "인벤토리", "판매 가능 괴식", entries, player_, "", 0);
        if (choice <= 0) return;

        int idx     = choice - 1;
        int max_qty = player_.count_item(MFOODS[idx]);
        if (max_qty <= 0) { gui_renderer_.show_notice("판매 실패", "보유한 아이템이 없습니다."); continue; }

        int price = player_.monster_food_sell_price(MPRICES[idx]);
        std::ostringstream qmsg;
        qmsg << MNAMES[idx] << "을(를) 몇 개 판매할까요?\n판매가: " << price << "G | 보유: " << max_qty;
        int qty = gui_renderer_.prompt_quantity("판매 수량", qmsg.str(), max_qty, 1);
        if (qty <= 0) { renderer_.add_message("판매를 취소했습니다."); continue; }

        int chance = player_.monster_food_success_chance();
        int success = 0, fail = 0, protected_fail = 0, total_gold = 0;
        bool first_drop = !player_.trust_ever_dropped();

        for (int i = 0; i < qty; ++i) {
            player_.remove_item(MFOODS[idx], 1);
            player_.set_sold_monster_food(true);

            int cur_tier = gui_trust_tier(player_.get_shop_trust());

            if (std::rand() % 100 < chance) {
                ++success;
                total_gold += price;
                int drop = 3 + std::rand() % 5;
                player_.lower_shop_trust(drop);

                std::ostringstream result;
                result << "✓ 판매 성공! (+" << price << "G)\n\n"
                       << gui_success_line(cur_tier);
                gui_renderer_.show_notice("판매 성공", result.str());
            } else {
                ++fail;
                if (player_.protects_monster_food_failure()) {
                    ++protected_fail;
                    std::ostringstream result;
                    result << "✗ 판매 실패\n능수능란한 언변으로 위기 모면!\n\n"
                           << gui_fail_line(cur_tier);
                    gui_renderer_.show_notice("판매 실패", result.str());
                } else {
                    int drop = 13 + std::rand() % 5;
                    player_.lower_shop_trust(drop);
                    std::ostringstream result;
                    result << "✗ 판매 실패\n\n" << gui_fail_line(cur_tier);
                    gui_renderer_.show_notice("판매 실패", result.str());
                }
            }

            // 처음 신용도가 하락한 경우 → 안내 팝업 (NPC 없는 일반 공지)
            if (first_drop && player_.trust_ever_dropped()) {
                first_drop = false;
                gui_renderer_.show_notice("알림",
                    "괴식 판매를 시도할 경우, 상점 주인의 신용도가 떨어집니다.\n\n"
                    "성공할 때보다 실패할 때 신용도가 더 크게 하락하며,\n"
                    "신용도가 일정 수준 이하로 떨어지면 상점 이용이 제한됩니다.\n\n"
                    "'선물하기' 기능을 통해 틈틈이 상점 주인의 신용도를 잘 관리해주세요!");
            }

            if (player_.get_shop_trust() <= 0) {
                if (total_gold > 0) {
                    player_.add_money(total_gold);
                    player_.record_sale_income(total_gold);
                    player_.add_exp(success * 2);
                }
                gui_renderer_.show_notice("상점 주인", gui_entry_line(0));
                renderer_.add_message("신용도가 0이 되어 상점에서 쫓겨났습니다.");
                return;
            }
        }

        if (total_gold > 0) {
            player_.add_money(total_gold);
            player_.record_sale_income(total_gold);
            player_.add_exp(success * 2);
        }

        std::ostringstream summary;
        summary << "거래 결과: 성공 " << success << "회 / 실패 " << fail << "회\n"
                << "획득 골드: +" << total_gold << "G";
        if (protected_fail > 0)
            summary << "\n능수능란한 언변으로 신용도 하락 " << protected_fail << "회 방어!";
        gui_renderer_.show_notice("괴식 거래 결과", summary.str());
        renderer_.add_message(summary.str());
    }
}

void Game::handle_gui_gift_shop() {
    bool trust_was_reduced = player_.get_shop_trust() < 100;
    const auto& gifts = MarketSystem::GIFT_ITEMS;
    int kl = kitchen_.get_level();

    // 입장 대사: 신용도가 한 번이라도 하락한 적 있으면 '회복' 메시지
    std::string intro_msg = trust_was_reduced
        ? "선물로 상점 주인의 신용도를 회복합시다!"
        : "선물로 상점 주인을 기쁘게 해줍시다!";

    int intro_sel = gui_renderer_.show_npc_dialogue("npc_buyer", intro_msg, {{1, "확인"}, {0, "뒤로"}}, 0);
    if (intro_sel != 1) return;

    while (true) {
        std::vector<GuiShopEntry> entries;
        for (int i = 0; i < 3; ++i) {
            if (kl < gifts[i].kitchen_level_required) continue;
            int have = player_.count_item(gifts[i].item);
            std::ostringstream sub;
            // 신용도 수치 대신 체감 표현
            if (gifts[i].trust_restore <= 5)       sub << "신뢰도 소폭 상승";
            else if (gifts[i].trust_restore <= 10)  sub << "신뢰도 크게 상승";
            else                                    sub << "신뢰도 대폭 상승";
            if (have <= 0) sub << " | 보유 없음";
            entries.push_back({i + 1, gifts[i].item, gifts[i].name, sub.str(), have <= 0});
        }

        if (entries.empty()) {
            gui_renderer_.show_notice("선물하기", "(주방을 업그레이드하면 선물할 수 있는 음식이 생깁니다.)");
            return;
        }

        int choice = gui_renderer_.show_shop_selector("선물하기", "인벤토리", "선물 가능 음식", entries, player_, "", 0);
        if (choice <= 0) return;

        int gift_idx = choice - 1;
        const GiftItem& g = gifts[gift_idx];

        if (player_.count_item(g.item) <= 0) {
            gui_renderer_.show_notice("선물 실패", "보유한 " + g.name + "이(가) 없습니다.");
            continue;
        }

        player_.remove_item(g.item, 1);
        player_.restore_shop_trust(g.trust_restore);

        std::ostringstream result;
        result << gui_gift_line(g.trust_restore, trust_was_reduced);
        gui_renderer_.show_npc_dialogue("npc_buyer", result.str(), {{0, "확인"}}, 0);
        renderer_.add_message(g.name + " 선물! 신뢰도 상승");
        trust_was_reduced = player_.get_shop_trust() < 100;
    }
}



void Game::handle_gui_farm(Tile& tile) {
    std::string msg;
    if (!tile.has_crop()) {
        std::ostringstream body;
        body << "심을 씨앗을 선택하세요.\n"
             << "감자 씨앗: " << player_.count_item(ItemType::SEED_POTATO) << "\n"
             << "당근 씨앗: " << player_.count_item(ItemType::SEED_CARROT) << "\n"
             << "밀 씨앗: "   << player_.count_item(ItemType::SEED_WHEAT) << "\n"
             << "쌀 씨앗: "   << player_.count_item(ItemType::SEED_RICE) << "\n"
             << "사과 묘목: " << player_.count_item(ItemType::SEED_APPLE);
        int crop_id = gui_renderer_.show_choice("씨앗 심기", body.str(), {{1, "감자"}, {2, "당근"}, {3, "밀"}, {4, "쌀"}, {5, "사과"}, {0, "취소"}}, 0);
        if (crop_id == 0) {
            renderer_.add_message("씨앗 심기를 취소했습니다.");
            return;
        }
        bool ok = CropSystem::plant_crop(player_, tile, crop_id, msg);
        renderer_.add_message(msg);
        if (!ok) gui_renderer_.show_notice("씨앗 심기", msg);
        return;
    }

    // 작물 이름 결정
    static const char* CROP_NAMES[] = {"", "감자", "당근", "밀", "쌀", "사과나무"};
    std::string crop_title = (tile.crop_id >= 1 && tile.crop_id <= 5)
        ? CROP_NAMES[tile.crop_id] : "작물";

    std::ostringstream body;
    body << "성장: " << tile.crop_progress << "/" << tile.crop_max_progress << "일\n";
    if (tile.crop_progress < tile.crop_max_progress) body << "아직 자라는 중입니다.";
    else body << "수확할 수 있습니다!";
    std::vector<std::pair<int, std::string>> buttons{{1, "수확"}};
    if (event_system_.is_drought_active()) buttons.push_back({2, "물 주기"});
    buttons.push_back({9, "작물 제거"});
    buttons.push_back({0, "취소"});
    int sel = gui_renderer_.show_choice(crop_title, body.str(), buttons, 0);
    if (sel == 1) {
        CropSystem::harvest_crop(player_, tile, msg);
        renderer_.add_message(msg);
        return;
    }
    else if (sel == 2 && event_system_.is_drought_active()) CropSystem::water_crop(player_, tile, msg);
    else if (sel == 9) remove_crop_from_tile(tile, msg);
    else msg = "작업을 취소했습니다.";
    renderer_.add_message(msg);
    if (sel != 0) gui_renderer_.show_notice(crop_title + " 결과", msg);
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

        // 괴식을 만든 적 있으면 '괴식 제작', 아니면 '자유 요리'
        bool already_made = player_.has_made_monster_food();
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
            // 자유 요리 / 괴식 제작
            already_made
                ? GuiActionEntry{14, "괴식 제작", "정식 레시피가 아닌 재료 2~6개 선택", "monster_food_mid", false}
                : GuiActionEntry{14, "자유 요리", "자유롭게 요리해봅시다!", "bag", false}
        };
        std::vector<GuiActionEntry> recipes = actions;

        int sel = gui_renderer_.show_action_panel("주방", status.str(), "레시피 / 조건", actions, recipes, 0);
        if (sel == 0) { renderer_.add_message("주방 이용을 취소했습니다."); return; }
        std::string msg;
        if (sel == 14) {
            std::vector<ItemType> ingredients;
            const std::string mode_name = already_made ? "괴식 제작" : "자유 요리";
            if (!choose_gui_free_cook_ingredients(ingredients)) {
                renderer_.add_message(mode_name + "을 취소했습니다.");
                continue;
            }

            std::string recipe_name = kitchen_.get_recipe_name(ingredients);
            if (!recipe_name.empty()) {
                msg = "선택한 조합은 정식 레시피(" + recipe_name + ")입니다.\n레시피에 있는 요리는 자유로운 요리가 아닙니다!";
                gui_renderer_.show_notice(mode_name + " 결과", msg);
            } else {
                kitchen_.cook(player_, ingredients, msg);
                // 희귀도 합 문구 제거
                std::string display_msg = msg;
                auto pos = display_msg.find(" 희귀도 합 ");
                if (pos != std::string::npos) display_msg = display_msg.substr(0, pos) + " 완성!";

                gui_renderer_.show_notice(mode_name + " 결과", display_msg);

                // 괴식 최초 생성 튜토리얼
                if (!player_.has_made_monster_food()) {
                    // 괴식 아이템이 인벤에 있으면 괴식을 만든 것
                    bool made = player_.count_item(ItemType::MONSTER_FOOD_LOW) > 0 ||
                                player_.count_item(ItemType::MONSTER_FOOD_MID) > 0 ||
                                player_.count_item(ItemType::MONSTER_FOOD_HIGH) > 0;
                    if (made) {
                        player_.set_made_monster_food(true);
                        // 2페이지 튜토리얼 (NPC 사진 없음)
                        gui_renderer_.show_notice("괴식을 만들었습니다!",
                            "레시피에 없는 조합으로 요리하면 '괴식'을 만들 수 있습니다.\n"
                            "괴식은 넣은 재료의 희귀도 합에 따라 하급, 중급, 상급 괴식으로 구분됩니다.\n\n"
                            "상점에서 괴식을 팔 경우, 확률에 따라 판매를 성공할 수도, 실패할 수도 있습니다.\n"
                            "성공할 경우 일반 음식에 비해 막대한 돈을 얻을 수 있지만,\n"
                            "실패할 경우 돈을 얻지 못한 채 괴식도 잃게 됩니다.");
                        gui_renderer_.show_notice("INT 스킬 트리의 히든 가지가 해금되었습니다!",
                            "INT 스킬 트리의 히든 가지인 '몬스터 푸드 가지'를 통해\n"
                            "괴식 판매 성공률, 괴식 판매 가격 등을 높일 수 있습니다.\n\n"
                            "화려한 말솜씨로 상점 주인의 눈을 속여,\n"
                            "주방의 쓰레기를 황금으로 바꾸는 영악한 사기꾼이 되어보세요.");
                    }
                }
            }
        } else {
            kitchen_.perform_action(player_, sel, msg);
            gui_renderer_.show_notice("주방 결과", msg);
        }
        sync_facility_level("주방", kitchen_.get_level());
        renderer_.add_message(msg);
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
    bool monster_unlocked = player_.has_made_monster_food();
    std::ostringstream intro;
    intro << "보유 스킬 포인트: " << player_.get_skill_points()
          << "\n원하는 계열을 선택하면 세부 노드와 선행 조건을 볼 수 있습니다."
          << "\nSTR: 체력과 광산 안정성"
          << "\nINT: 제작/거래" << (monster_unlocked ? "와 괴식 판매" : "")
          << "\nLUK: 수확, 축산, 낚시, 요리 행운";
    int tree = gui_renderer_.show_choice("스킬 트리", intro.str(),
        {{1, "STR 체력/채굴"}, {2, monster_unlocked ? "INT 제작/거래/괴식" : "INT 제작/거래"}, {3, "LUK 수확/행운"}}, 0);
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
        bool monster_unlocked = player_.has_made_monster_food();
        status = monster_unlocked
            ? "제작/거래 경제 가지와 괴식 판매 가지로 나뉩니다. 노드를 클릭해 효과와 선행 조건을 확인하세요."
            : "제작/거래 경제 가지입니다. 노드를 클릭해 효과와 선행 조건을 확인하세요.";
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
        };
        // 괴식을 만든 뒤에만 괴식 판매 가지 표시
        if (monster_unlocked) {
            nodes.push_back({5, 425, 48, "5 몬스터 조리", "괴식 판매 가격이 10% 증가합니다. 괴식 판매 가지의 시작 노드입니다.", "kitchen", learned(5), !learned(5) && !can(5), {}});
            nodes.push_back({6, 425, 158, "6 제조 전문가", "괴식 판매 성공률이 20% 증가합니다. 필요: 몬스터 조리.", "kitchen", learned(6), !learned(6) && !can(6), {5}});
            nodes.push_back({7, 425, 268, "7 손실 방지", "괴식 판매 실패 시 50% 확률로 상점 신뢰도 하락을 막습니다. 필요: 제조 전문가.", "hand", learned(7), !learned(7) && !can(7), {6}});
            nodes.push_back({8, 425, 378, "8 관리 루틴", "괴식 판매 성공률이 추가로 30% 증가합니다. 필요: 손실 방지.", "level", learned(8), !learned(8) && !can(8), {7}});
        }
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




