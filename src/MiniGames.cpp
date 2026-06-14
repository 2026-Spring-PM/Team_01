#include "MiniGames.h"

#include "Input.h"
#include "ItemType.h"

#include <cstdlib>
#include <iostream>
#include <unistd.h>

namespace {
void clear_console() {
    std::cout << "\033[2J\033[H" << std::flush;
}

void draw_fishing_bar(int pos, int width, int great_start, int great_end, int perfect_pos, bool has_perfect) {
    std::cout << "\033[H\033[J";
    std::cout << "[낚시] 움직이는 바늘(^)이 목표 구간에 들어왔을 때 아무 키나 누르세요.\n";
    std::cout << "목표: [";
    for (int i = 0; i < width; ++i) {
        if (has_perfect && i == perfect_pos) std::cout << 'P';
        else if (i >= great_start && i <= great_end) std::cout << '=';
        else std::cout << '-';
    }
    std::cout << "]\n바늘: [";
    for (int i = 0; i < width; ++i) std::cout << (i == pos ? '^' : ' ');
    std::cout << "]\n" << std::flush;
}

void give_fish_rewards(Player& player, bool perfect) {
    int luck_bonus = 0;
    if (player.has_luk_node(3)) luck_bonus += 10;
    if (player.has_luk_node(6)) luck_bonus += 10;

    player.add_item(ItemType::FISH_MACKEREL, 1);
    std::cout << "고등어 1마리 획득!\n";
    if (std::rand() % 100 < 30 + luck_bonus) {
        player.add_item(ItemType::FISH_MACKEREL, 1);
        std::cout << "고등어 추가 획득!\n";
    }
    if (std::rand() % 100 < 30 + luck_bonus) {
        player.add_item(ItemType::FISH_FLATFISH, 1);
        std::cout << "광어 1마리 획득!\n";
    }
    if (std::rand() % 100 < 5 + luck_bonus) {
        player.add_item(ItemType::FISH_TUNA, 1);
        std::cout << "참치 1마리 획득!\n";
    }
    if (perfect && player.has_luk_node(6) && std::rand() % 100 < 10) {
        player.add_item(ItemType::FISH_MACKEREL, 1);
        player.add_item(ItemType::FISH_FLATFISH, 1);
        player.add_item(ItemType::FISH_TUNA, 1);
        std::cout << "황금 어장 보너스! 생선 3종을 추가로 얻었습니다.\n";
    }
}

bool add_item_checked(Player& player, ItemType item, int qty, std::ostream& out) {
    if (player.add_item(item, qty)) return true;
    out << "인벤토리 공간이 부족해 " << item_name(item) << "을(를) 받을 수 없습니다.\n";
    return false;
}

void give_mining_reward(Player& player, int digs, int multiplier) {
    if (digs <= 0) {
        player.add_exp(1);
        std::cout << "아무 광맥도 캐지 못했습니다. XP +1\n";
        return;
    }
    int copper_total = 0;
    int iron_total = 0;
    int gold_total = 0;
    int exp_total = 0;
    for (int step = 1; step <= digs && step <= 5; ++step) {
        if (step <= 2) {
            copper_total += step;
            exp_total += 4 * step;
        } else if (step <= 4) {
            iron_total += step - 2;
            exp_total += 6 * step;
        } else {
            gold_total += 1;
            exp_total += 30;
        }
    }

    bool gained_any = false;
    if (copper_total > 0 && add_item_checked(player, ItemType::COPPER, copper_total * multiplier, std::cout)) {
        std::cout << "구리 광석 " << (copper_total * multiplier) << "개 획득!\n";
        gained_any = true;
    }
    if (iron_total > 0 && add_item_checked(player, ItemType::IRON, iron_total * multiplier, std::cout)) {
        std::cout << "철 광석 " << (iron_total * multiplier) << "개 획득!\n";
        gained_any = true;
    }
    if (gold_total > 0 && add_item_checked(player, ItemType::GOLD, gold_total * multiplier, std::cout)) {
        std::cout << "금 광석 " << (gold_total * multiplier) << "개 획득!\n";
        gained_any = true;
    }
    if (gained_any) {
        player.add_exp(exp_total);
        std::cout << "누적 채광 보상 XP +" << exp_total << "\n";
    }
}

std::string pressure_hint(int margin) {
    if (margin >= 11) return "갱도가 아주 조용합니다. 바위 틈새로 은은한 광취가 풍깁니다.";
    if (margin >= 6) return "쿠구구... 천장에서 가벼운 흙먼지가 조금씩 떨어집니다.";
    if (margin >= 1) return "위험! 갱도가 심하게 흔들리며 벽면에 거대한 금이 갑니다.";
    return "콰아아아앙! 암반이 압력을 버티지 못하고 무너져 내립니다!";
}
}

namespace MiniGames {

void play_fishing(Player& player) {
    clear_console();
    drain_input();

    int stamina_cost = player.get_fishing_stamina_cost();
    std::cout << "낚시를 시작할까요? 스태미나 " << stamina_cost << " 사용 (Y/N): " << std::flush;
    char ans = getch();
    if (ans != 'y' && ans != 'Y') {
        clear_console();
        return;
    }
    if (!player.use_stamina(stamina_cost)) {
        std::cout << "\n스태미나가 부족합니다.\n";
        wait_key();
        clear_console();
        return;
    }

    const int width = 20;
    const int center = width / 2;
    int great_radius = player.get_rod_grade() >= 2 ? 3 : 2;
    int great_start = center - great_radius;
    int great_end = center + great_radius;
    bool has_perfect = player.get_rod_grade() > 0;
    int perfect_pos = center;

    int pos = 0;
    int direction = 1;
    drain_input();
    while (!kbhit()) {
        draw_fishing_bar(pos, width, great_start, great_end, perfect_pos, has_perfect);
        usleep(25000);
        pos += direction;
        if (pos >= width - 1) { pos = width - 1; direction = -1; }
        if (pos <= 0) { pos = 0; direction = 1; }
    }
    getch();
    drain_input();
    clear_console();

    bool great = pos >= great_start && pos <= great_end;
    bool perfect = has_perfect && pos == perfect_pos;
    if (great || perfect) {
        std::cout << (perfect ? "완벽!\n" : "성공!\n");
        give_fish_rewards(player, perfect);
        player.add_exp(perfect ? 10 : 6);
    } else {
        std::cout << "물고기가 도망갔습니다.\n";
        player.add_exp(3);
    }

    wait_key();
    clear_console();
}

void play_blackjack_mining(Player& player) {
    clear_console();
    drain_input();

    int stamina_cost = player.get_mining_stamina_cost();
    std::cout << "블랙잭 광산을 시작할까요? 스태미나 " << stamina_cost << " 사용 (Y/N): " << std::flush;
    char ans = getch();
    if (ans != 'y' && ans != 'Y') {
        clear_console();
        return;
    }
    if (!player.use_stamina(stamina_cost)) {
        std::cout << "\n스태미나가 부족합니다.\n";
        wait_key();
        clear_console();
        return;
    }

    int collapse_pressure = 16 + std::rand() % 15;
    int pressure = 0;
    int digs = 0;
    bool busted = false;
    bool str_double_reward = false;

    while (true) {
        clear_console();
        std::cout << "[블랙잭 광산]\n";
        std::cout << "현재 붕괴 압력: " << collapse_pressure << "\n";
        std::cout << "채광 횟수: " << digs << " / 5\n";
        if (player.get_pick_grade() >= 3) {
            std::cout << "현재 누적 압력: " << pressure << " (다음 상승 1~10)\n";
        } else if (player.get_pick_grade() >= 2) {
            int low = std::max(0, pressure - (std::rand() % 3));
            std::cout << "곡괭이 감지 범위: " << low << "~" << (low + 5) << "\n";
        } else if (player.get_pick_grade() >= 1) {
            std::cout << "곡괭이 힌트: " << pressure_hint(collapse_pressure - pressure) << "\n";
        }
        std::cout << "보상은 누적됩니다: 1회 구리 1개, 2회 구리 2개, 3회 철 1개, 4회 철 2개, 5회 금 1개\n";
        std::cout << "1. 더 판다\n2. 그만 파고 탈출한다\n>> " << std::flush;
        char sel = getch();
        if (sel == '2') break;
        if (sel != '1') continue;

        int rise_max = player.get_pick_grade() >= 3 ? 10 : 11;
        int rise = 1 + std::rand() % rise_max;
        pressure += rise;
        ++digs;

        if (pressure >= collapse_pressure) {
            if (player.has_str_node(6)) {
                pressure -= rise;
                str_double_reward = true;
                std::cout << "\n압력 초과! 헤라클레스의 힘으로 마지막 채굴을 무효화했습니다.\n";
                std::cout << "현재 누적 압력은 " << pressure << "입니다. 이번 탈출 보상이 2배가 됩니다.\n";
                wait_key();
                break;
            }
            if (player.has_str_node(3) && std::rand() % 100 < 30) {
                std::cout << "\n콰아앙! 무너지려는 갱도를 힘으로 버텼습니다. 지금 탈출하면 이전 보상을 지킵니다.\n";
                --digs;
                wait_key();
                break;
            }
            busted = true;
            break;
        }
        if (digs >= 5) break;
    }

    clear_console();
    if (busted) {
        player.add_exp(2);
        std::cout << "광산이 붕괴했습니다. 보상은 없습니다. XP +2\n";
    } else {
        int multiplier = str_double_reward ? 2 : 1;
        give_mining_reward(player, digs, multiplier);
        if (multiplier > 1) std::cout << "헤라클레스 보너스로 보상이 2배가 되었습니다.\n";
    }

    wait_key();
    clear_console();
}

} // namespace MiniGames
