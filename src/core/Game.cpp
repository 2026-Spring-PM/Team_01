#include "Game.h"

#include <iostream>
#include <string>

Game::Game()
    : map_(),
      player_(),
      renderer_(),
      is_running_(true) {
}

void Game::run() {
    // === 시작 화면: 이름 입력 ===
    renderer_.draw_title_screen();

    std::string player_name;
    std::getline(std::cin, player_name);

    // 빈 입력 처리
    if (player_name.empty()) {
        player_name = "농부";
    }

    player_.set_name(player_name);

    // 환영 메시지
    renderer_.add_message("허허, " + player_name + "이로구먼! 환영하네. 🌱");

    // === 메인 루프 ===
    while (is_running_) {
        renderer_.draw(player_, map_);

        char input;
        std::cin >> input;

        handle_input(input);
        check_game_end();
    }
}

void Game::handle_input(char input) {
    switch (input) {
        case 'w': case 'W':
            if (player_.try_move(0, -1, map_)) {
                renderer_.add_message("");
            } else {
                renderer_.add_message("그쪽은 갈 수 없어요. 🚫");
            }
            break;

        case 's': case 'S':
            if (player_.try_move(0, 1, map_)) {
                renderer_.add_message("");
            } else {
                renderer_.add_message("그쪽은 갈 수 없어요. 🚫");
            }
            break;

        case 'a': case 'A':
            if (player_.try_move(-1, 0, map_)) {
                renderer_.add_message("");
            } else {
                renderer_.add_message("그쪽은 갈 수 없어요. 🚫");
            }
            break;

        case 'd': case 'D':
            if (player_.try_move(1, 0, map_)) {
                renderer_.add_message("");
            } else {
                renderer_.add_message("그쪽은 갈 수 없어요. 🚫");
            }
            break;

        case 'b': case 'B':
            if (player_.try_sleep(map_)) {
                renderer_.add_message("잘 잤는가, " + player_.get_name() + "이! 🌅");
            } else {
                renderer_.add_message("침대로 가야 잠을 잘 수 있다네. 🛏️");
            }
            break;

        case 'i': case 'I':
            show_inventory();
            break;

        case 'q': case 'Q':
            quit();
            break;

        default:
            renderer_.add_message("그런 명령은 모르겠어요. 🤔");
            break;
    }
}

void Game::show_inventory() {
    renderer_.draw_inventory_screen(player_);

    // 아무 키나 받기
    char dummy;
    std::cin >> dummy;
}

void Game::quit() {
    is_running_ = false;
    std::cout << "\n게임을 종료합니다. 다음에 또 만나요! 👋\n";
}

void Game::check_game_end() {
    if (player_.get_day() > MAX_DAY) {
        renderer_.draw_ending_screen(player_);
        is_running_ = false;

        // 종료 전 잠시 대기
        std::cout << "\n계속하려면 아무 키나... > ";
        char dummy;
        std::cin >> dummy;
    }
}