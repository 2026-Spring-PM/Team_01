#include "MiniGames.h"
#include "Input.h"
#include "ItemType.h"

#include <cstdlib>
#include <iostream>
#include <set>
#include <vector>
#include <unistd.h>

namespace {
void clear_console() {
    // ANSI clear + cursor home. system("clear")보다 연속 실행 시 깜빡임과 잔상 문제가 적습니다.
    std::cout << "\033[2J\033[H" << std::flush;
}

void show_cursor(bool show) {
    std::cout << (show ? "\033[?25h" : "\033[?25l") << std::flush;
}

class CursorGuard {
public:
    CursorGuard() { show_cursor(false); }
    ~CursorGuard() { show_cursor(true); }
};

int fishing_hit_width_for_level(int level) {
    if (level >= 20) return 3;
    if (level >= 10) return 2;
    return 1;
}

void render_fishing_frame(int level, int pos, int hit_start, int hit_end, int bar_width) {
    std::cout << "\033[H\033[J";
    std::cout << "🎣 [낚시 진행 중]\n";
    std::cout << "레벨: " << level << " | 스태미나 30 사용 완료 | 목표 칸: "
              << (hit_end - hit_start + 1) << "칸\n";
    std::cout << "목표 구간(=)에 바늘(^)이 들어왔을 때 아무 키나 누르세요.\n\n";

    std::cout << "타겟: [";
    for (int i = 0; i < bar_width; ++i) {
        if (i >= hit_start && i <= hit_end) std::cout << '=';
        else std::cout << '-';
    }
    std::cout << "]\n";

    std::cout << "바늘: [";
    for (int i = 0; i < bar_width; ++i) {
        if (i == pos) std::cout << '^';
        else std::cout << ' ';
    }
    std::cout << "]\n" << std::flush;
}
}

namespace MiniGames {

void play_fishing(Player& player) {
    clear_console();
    show_cursor(true);
    drain_input();

    std::cout << "\n🎣 강변입니다. 낚시를 시작하시겠습니까? (Y/N): " << std::flush;
    char ans = getch();
    if (ans != 'y' && ans != 'Y') {
        drain_input();
        clear_console();
        return;
    }

    if (!player.use_stamina(30)) {
        std::cout << "\n스태미나가 부족합니다. (필요: 30)\n";
        wait_key();
        clear_console();
        return;
    }

    const int BAR_WIDTH = 20;
    int pos = 0;
    int direction = 1;
    int level = player.get_level();
    int hit_width = fishing_hit_width_for_level(level);
    int hit_start = (BAR_WIDTH / 2) - (hit_width / 2);
    int hit_end = hit_start + hit_width - 1;

    drain_input();
    {
        CursorGuard cursor_guard;
        while (!kbhit()) {
            render_fishing_frame(level, pos, hit_start, hit_end, BAR_WIDTH);
            usleep(25000); // 목표 칸이 작아졌으므로 빠른 속도로 난이도를 유지합니다.
            pos += direction;
            if (pos >= BAR_WIDTH - 1) {
                pos = BAR_WIDTH - 1;
                direction = -1;
            } else if (pos <= 0) {
                pos = 0;
                direction = 1;
            }
        }
    }

    getch();
    drain_input();
    clear_console();

    if (pos >= hit_start && pos <= hit_end) {
        int dice = std::rand() % 100;
        if (dice < 70) {
            player.add_item(ItemType::FISH_MACKEREL, 1);
            std::cout << "\n🐟 고등어를 낚았습니다!\n";
        } else if (dice < 95) {
            player.add_item(ItemType::FISH_FLATFISH, 1);
            std::cout << "\n🐠 광어를 낚았습니다!\n";
        } else {
            player.add_item(ItemType::FISH_TUNA, 1);
            std::cout << "\n🐋 참치를 낚았습니다!\n";
        }
        player.add_exp(60);
        std::cout << "XP +60\n";
    } else {
        std::cout << "\n앗! 물고기가 도망쳤습니다.\n";
    }

    wait_key();
    clear_console();
    drain_input();
}

void play_lotto_mining(Player& player) {
    clear_console();
    drain_input();
    std::cout << "\n💎 광산입니다. 로또 채굴을 시작하시겠습니까? (Y/N): " << std::flush;
    char ans = getch();
    if (ans != 'y' && ans != 'Y') {
        drain_input();
        return;
    }

    if (!player.use_stamina(30)) {
        std::cout << "\n스태미나가 부족합니다. (필요: 30)\n";
        wait_key();
        return;
    }

    std::vector<int> user_numbers;
    std::cout << "\n0~9 사이 번호 6개를 입력하세요.\n";
    for (int i = 0; i < 6; ++i) {
        std::cout << (i + 1) << "번째 번호: " << std::flush;
        int n = read_int_safe(0);
        if (n < 0 || n > 9) n = 0;
        user_numbers.push_back(n);
    }

    std::set<int> win_set;
    while (win_set.size() < 6) win_set.insert(std::rand() % 10);

    int match = 0;
    for (int n : user_numbers) {
        if (win_set.count(n)) ++match;
    }

    std::cout << "\n당첨 번호: ";
    for (int n : win_set) std::cout << n << " ";
    std::cout << "\n일치 개수: " << match << "개\n";

    if (match == 3) {
        player.add_item(ItemType::COPPER, 1);
        player.add_exp(150);
        std::cout << "구리 광석 1개 획득! XP +150\n";
    } else if (match == 4) {
        player.add_item(ItemType::IRON, 1);
        player.add_exp(200);
        std::cout << "철 광석 1개 획득! XP +200\n";
    } else if (match == 5) {
        player.add_item(ItemType::GOLD, 1);
        player.add_exp(250);
        std::cout << "금 광석 1개 획득! XP +250\n";
    } else if (match == 6) {
        player.add_item(ItemType::COPPER, 1);
        player.add_item(ItemType::IRON, 1);
        player.add_item(ItemType::GOLD, 1);
        player.add_exp(300);
        std::cout << "잭팟! 구리/철/금 광석을 모두 획득! XP +300\n";
    } else {
        std::cout << "아쉽게도 광맥을 찾지 못했습니다.\n";
    }
    wait_key();
    clear_console();
}

}
