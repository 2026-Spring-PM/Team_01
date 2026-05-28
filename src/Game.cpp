#include "Game.h"
#include "Input.h"
#include "CropSystem.h"
#include "LivestockSystem.h"
#include "MiniGames.h"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

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

    // 기존 코드의 시작 자원
    player_.add_item(ItemType::SEED_POTATO, 3);
    player_.add_item(ItemType::BABY_CHICKEN, 1);
    player_.add_item(ItemType::COPPER, 2);
    player_.add_item(ItemType::IRON, 1);

    renderer_.add_message("허허, " + player_name + "이로구먼! 감자 씨앗 3개와 새끼 닭 1마리를 지급했네. 🌱");

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
        default:
            renderer_.add_message("존재하지 않는 명령어입니다.");
            break;
    }
// 레벨업 메시지 확인
    if (!player_.get_levelup_message().empty()) {
        renderer_.add_message(player_.get_levelup_message());
        player_.clear_levelup_message();
    }

}

void Game::handle_action() {
    int px = player_.get_x();
    int py = player_.get_y();
    Tile& tile = map_.get_tile(px, py);

    if (px == Map::MINE_X && py == Map::MINE_Y) {
        system("clear");
        MiniGames::play_lotto_mining(player_);
        renderer_.add_message("광산 탐험을 마쳤습니다.");
        return;
    }

    switch (tile.type) {
        case TileType::SAND:
        case TileType::WATER:
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
            advance_day();
            break;
        case TileType::MARKET:
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
            kitchen_.interact(player_);
            renderer_.add_message("주방 이용을 마쳤습니다.");
            break;
        case TileType::WORKSHOP:
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

        if (crop_id == 0) {
            renderer_.add_message("작업을 취소했습니다.");
            return;
        }

        std::string message;
        CropSystem::plant_crop(player_, tile, crop_id, message);
        renderer_.add_message(message);
        return;
    }

    std::cout << "🌾 [농경지 관리]\n";
    std::cout << "🌱 성장 현황: " << tile.crop_progress << "/" << tile.crop_max_progress << "일\n";
    std::cout << "1. 수확하기\n";
    if (event_system_.is_drought_active()) {
        std::cout << "2. 물 주기 (가뭄 대처, 스태미나 20)\n";
    }
    std::cout << "0. 취소\n>> " << std::flush;
    char sel = getch();

    std::string message;
    if (sel == '1') CropSystem::harvest_crop(player_, tile, message);
    else if (sel == '2' && event_system_.is_drought_active()) CropSystem::water_crop(player_, tile, message);
    else if (sel == '2') message = "가뭄이 아닐 때는 물 주기 행동이 필요하지 않습니다.";
    else message = "작업을 취소했습니다.";
    renderer_.add_message(message);
}

void Game::handle_livestock(Tile& tile) {
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

    std::cout << "🐄 [가축 관리]\n";
    std::cout << "1. 생산물 수집\n";
    std::cout << "2. 가축 제거\n";
    std::cout << "0. 취소\n>> " << std::flush;

    char sel = getch();
    std::string message;
    if (sel == '1') LivestockSystem::collect_product(player_, tile, message);
    else if (sel == '2') {
        std::cout << "\n정말 제거하시겠습니까? (Y/N): " << std::flush;
        char conf = getch();
        if (conf == 'y' || conf == 'Y') LivestockSystem::remove_animal(tile, message);
        else message = "가축 제거를 취소했습니다.";
    } else {
        message = "작업을 취소했습니다.";
    }
    renderer_.add_message(message);
}

void Game::handle_building(Tile& tile) {
    system("clear");

    if (!tile.building_name.empty()) {
        if (tile.building_name == "주방") kitchen_.interact(player_);
        else if (tile.building_name == "작업장") workshop_.interact(player_);
        renderer_.add_message("건물 이용을 마쳤습니다.");
        return;
    }

    std::cout << "🏡 [건축 부지]\n";
    std::cout << "1. 주방 건설\n";
    std::cout << "2. 작업장 건설\n";
    std::cout << "0. 취소\n>> " << std::flush;
    char sel = getch();

    if (sel == '1') {
        tile.building_name = "주방";
        tile.is_occupied = true;
        renderer_.add_message("주방을 건설했습니다. 🍳");
    } else if (sel == '2') {
        tile.building_name = "작업장";
        tile.is_occupied = true;
        renderer_.add_message("작업장을 건설했습니다. 🔨");
    } else {
        renderer_.add_message("건설을 취소했습니다.");
    }
}

void Game::advance_day() {
    // 가뭄 판정은 map_.advance_day()보다 먼저 처리합니다.
    // map_.advance_day()가 watered_today를 false로 초기화하기 때문에, 순서가 반대면
    // 낮에 물을 준 작물까지 모두 말라 죽는 문제가 생깁니다.
    player_.sleep_anywhere();
    market_.start_new_day();

    std::string event_message;
    event_system_.process_new_day(player_, map_, event_message);

    map_.advance_day();

    std::string message = "침대에서 잠을 잤습니다. 하루가 지나 작물과 가축이 성장했습니다. 🌅";
    if (!event_message.empty()) message += " " + event_message;
    renderer_.add_message(message);
}

void Game::show_inventory() {
    renderer_.draw_inventory_screen(player_);
    getch();
}

void Game::quit() {
    system("clear");
    std::cout << "정말 게임을 종료하시겠습니까? 저장 기능은 아직 없습니다. (Y/N): " << std::flush;
    char ans = getch();
    if (ans == 'y' || ans == 'Y') {
        is_running_ = false;
        std::cout << "\n게임을 종료합니다. 다음에 또 만나요! 👋\n";
    } else {
        renderer_.add_message("게임 종료를 취소했습니다.");
    }
}

void Game::check_game_end() {
    if (player_.get_day() > MAX_DAY) {
        is_running_ = false;
        renderer_.draw_ending_screen(player_);
        std::cout << "\n💯 100일간의 시뮬레이션이 종료되었습니다!\n";
    }
}
