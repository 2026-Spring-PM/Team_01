#include "MiniGames.h"
#include "Input.h"
#include "ItemType.h"

#include <cstdlib>
#include <iostream>
#include <set>
#include <vector>
#include <unistd.h>
#include <algorithm>

namespace {
void clear_console() {
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
}

namespace MiniGames {

void play_fishing(Player& player) {
    clear_console();
    int rod_grade = player.get_tool_grade(); // 4단계 전까지 통합 도구 등급 사용
    int cost = (rod_grade >= 3) ? 3 : 5;     // 금 낚싯대면 스태미나 3 소모
    
    if (!player.use_stamina(cost)) {
        std::cout << "\n스태미나가 부족합니다. (필요: " << cost << ")\n";
        wait_key(); return;
    }

    int bar_width = 30;
    int target_center = bar_width / 2;
    int target_start = target_center - 1;
    int target_end = target_center + 1;

    int pos = 0;
    int dir = 1;
    CursorGuard cg;
    drain_input();

    while (true) {
        std::cout << "\033[H\033[J";
        std::cout << "🎣 [낚시 미니게임] (스태미나 -" << cost << ")\n";
        std::cout << "목표: 중앙 3칸 (가운데=Perfect, 양옆=Great)\n";
        if (rod_grade == 0) std::cout << "※ 낚싯대가 없어 Perfect 판정이 불가능합니다.\n";
        std::cout << "\n[";
        
        for (int i = 0; i < bar_width; ++i) {
            if (i == pos) std::cout << "🐟";
            else if (i == target_center) std::cout << "⭐";
            else if (i == target_start || i == target_end) std::cout << "▒";
            else std::cout << "-";
        }
        std::cout << "]\n스페이스바를 눌러 찌를 건지세요!\n";

        if (kbhit()) {
            char c = getch();
            if (c == ' ' || c == '\n') break;
        }
        pos += dir;
        if (pos <= 0 || pos >= bar_width - 1) dir = -dir;
        usleep(30000);
    }

    std::string rank = "Fail";
    if (pos >= target_start && pos <= target_end) {
        if (pos == target_center && rod_grade > 0) rank = "Perfect";
        else rank = "Great";
    }

    std::cout << "\n판정: [" << rank << "]!\n";
    if (rank == "Fail") {
        std::cout << "물고기가 미끼만 먹고 도망갔습니다...\n";
        wait_key(); return;
    }

    // 주사위 확률 기반 생선 지급
    int mackerel = 1; // 100% 1마리
    if (std::rand() % 100 < 30) mackerel += 1; // 30% 확률로 1마리 더
    
    int flatfish = 0;
    if (std::rand() % 100 < 30) flatfish = 1;  // 30% 확률로 광어 1마리
    
    int tuna = 0;
    if (std::rand() % 100 < 5) tuna = 1;       // 5% 확률로 참치 1마리

    std::cout << "\n[어획 결과]\n";
    player.add_item(ItemType::FISH_MACKEREL, mackerel);
    std::cout << "🐟 고등어 x" << mackerel << "\n";
    
    if (flatfish > 0) { player.add_item(ItemType::FISH_FLATFISH, flatfish); std::cout << "🐟 광어 x" << flatfish << "\n"; }
    if (tuna > 0) { player.add_item(ItemType::FISH_TUNA, tuna); std::cout << "🐟 참치 x" << tuna << "\n"; }

    // 보물상자 지급 로직
    if (rank == "Perfect") {
        if (rod_grade == 1) { player.add_item(ItemType::TREASURE_BOX_BASIC, 1); std::cout << "🎁 초급 보물상자 획득!\n"; }
        else if (rod_grade == 2) { player.add_item(ItemType::TREASURE_BOX_NORMAL, 1); std::cout << "🎁 중급 보물상자 획득!\n"; }
        else if (rod_grade >= 3) { player.add_item(ItemType::TREASURE_BOX_ADVANCED, 1); std::cout << "🎁 고급 보물상자 획득!\n"; }
    } else if (rank == "Great") {
        if (rod_grade == 2) { player.add_item(ItemType::TREASURE_BOX_BASIC, 1); std::cout << "🎁 초급 보물상자 획득!\n"; }
        else if (rod_grade >= 3) { player.add_item(ItemType::TREASURE_BOX_NORMAL, 1); std::cout << "🎁 중급 보물상자 획득!\n"; }
    }
    wait_key();
}


void play_lotto_mining(Player& player) {
    clear_console();
    int pickaxe_grade = player.get_tool_grade(); // 4단계 전까지 통합 도구 등급 사용
    int cost = (pickaxe_grade >= 3) ? 3 : 5;     // 금 곡괭이면 스태미나 3 소모

    if (!player.use_stamina(cost)) {
        std::cout << "\n스태미나가 부족합니다. (필요: " << cost << ")\n";
        wait_key(); return;
    }

    int collapse_pressure = 16 + (std::rand() % 15); // 16 ~ 30 사이 랜덤
    int current_pressure = 0;
    int hit_count = 0;

    std::cout << "⛏️ [광산 탐험 시작] (스태미나 -" << cost << ")\n";
    
    while (hit_count < 5) {
        std::cout << "\n====================================\n";
        std::cout << "현재 채광 횟수: " << hit_count << " / 5\n\n";
        
        // 곡괭이 등급에 따른 힌트 제공
        if (pickaxe_grade >= 1) {
            int margin = collapse_pressure - current_pressure;
            std::cout << "💡 [현재 갱도 상태]\n";
            if (margin >= 11) std::cout << "\"갱도가 아주 조용합니다. 바위 틈새로 은은한 광취가 풍깁니다.\"\n";
            else if (margin >= 6) std::cout << "\"쿠구구... 천장에서 가벼운 흙먼지가 조금씩 떨어지기 시작합니다.\"\n";
            else if (margin >= 1) std::cout << "🚨 \"위험! 갱도가 심하게 흔들리며 벽면에 거대한 금이 가기 시작합니다!\"\n";
            else std::cout << "🚨 \"곧 무너질 것 같습니다!!\"\n";
        }
        if (pickaxe_grade >= 2) {
            int margin = collapse_pressure - current_pressure;
            int min_guess = std::max(0, margin - (std::rand() % 6));
            int max_guess = min_guess + 5;
            std::cout << "🎯 [탐지된 여유 압력 범위]: 대략 " << min_guess << " ~ " << max_guess << " 사이\n";
        }

        std::cout << "\n1. 더 판다\n2. 그만 파고 탈출한다\n>> " << std::flush;
        char sel = getch();

        if (sel == '2') {
            std::cout << "\n🏃 광산에서 안전하게 탈출했습니다!\n";
            break;
        } else if (sel == '1') {
            hit_count++;
            int increase = (pickaxe_grade >= 3) ? (1 + std::rand() % 10) : (1 + std::rand() % 11);
            current_pressure += increase;
            std::cout << "\n쾅! 💥 (압력이 상승했습니다)\n";

            if (current_pressure >= collapse_pressure) {
                std::cout << "\n☠️ 콰아아아앙! 암반이 압력을 버티지 못하고 무너져 내립니다!\n";
                std::cout << "가진 것을 모두 잃고 광산에서 쫓겨났습니다...\n";
                wait_key();
                return; // 보상 없이 종료
            }
        }
    }

    if (hit_count == 5) std::cout << "\n🚧 5회 채광에 도달하여 강제 탈출합니다!\n";

    std::cout << "\n[채광 보상 정산]\n";
    if (hit_count == 1 || hit_count == 2) {
        int amt = (hit_count == 1) ? 1 : 2;
        player.add_item(ItemType::COPPER, amt);
        std::cout << "🥉 구리 광석 x" << amt << " 획득!\n";
    } else if (hit_count == 3 || hit_count == 4) {
        int amt = (hit_count == 3) ? 1 : 2;
        player.add_item(ItemType::IRON, amt);
        std::cout << "🥈 철 광석 x" << amt << " 획득!\n";
    } else if (hit_count >= 5) {
        player.add_item(ItemType::GOLD, 1);
        std::cout << "🥇 금 광석 x1 획득!\n";
    } else {
        std::cout << "획득한 광석이 없습니다.\n";
    }

    wait_key();
}

} // namespace MiniGames