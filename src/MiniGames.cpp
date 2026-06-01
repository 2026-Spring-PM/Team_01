#include "MiniGames.h"
#include "Input.h"
#include "ItemType.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>

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

// ============================================================
// 낚시 미니게임 보조
// ============================================================

// 낚싯대 없을 때: Perfect 존 없음, 전부 Great 판정
// 동제: Great 존 15+15 = 30%, Perfect 존 10%
// 철제: 더 넓게
// 금제: 가장 넓게
struct FishingZones {
    int bar_width;
    int great_start;    // Great 존 시작 (왼쪽)
    int great_end;
    int perfect_start;  // Perfect 존 시작 (Great 안쪽)
    int perfect_end;
    bool has_perfect;
};

FishingZones make_zones(int rod_grade) {
    FishingZones z;
    z.bar_width = 20;

    int center = z.bar_width / 2;

    z.great_start = center - 2;
    z.great_end = center + 1;

    if (rod_grade == 0) {
        // 낚싯대 없음: Perfect 불가, 중앙 포함 전부 Great 판정
        z.has_perfect = false;
        z.perfect_start = -1;
        z.perfect_end = -1;
    } else {
        // 낚싯대 있음: 중앙 1칸 Perfect
        z.has_perfect = true;
        z.perfect_start = center;
        z.perfect_end = center;
    }
    
    return z;
}

void render_fishing_frame(int pos, const FishingZones& z) {
    std::cout << "\033[H\033[J";
    std::cout << "🎣 [낚시 진행 중]\n";
    std::cout << "목표 구간에 바늘(^)이 들어왔을 때 아무 키나 누르세요.\n";
    if (z.has_perfect)
        std::cout << " P=Perfect 존 | =Great 존\n\n";
    else
        std::cout << " =Great 존 (낚싯대 없음: Perfect 불가)\n\n";

    std::cout << "타겟: [";
    for (int i = 0; i < z.bar_width; ++i) {
        if (z.has_perfect && i >= z.perfect_start && i <= z.perfect_end) std::cout << 'P';
        else if (i >= z.great_start && i <= z.great_end) std::cout << '=';
        else std::cout << '-';
    }
    std::cout << "]\n";

    std::cout << "바늘: [";
    for (int i = 0; i < z.bar_width; ++i) {
        if (i == pos) std::cout << '^';
        else std::cout << ' ';
    }
    std::cout << "]\n" << std::flush;
}

// 물고기 독립 주사위 (LUK 보너스 적용)
void roll_fish_dice(Player& player, int luk_bonus) {
    // 고등어: 100% + 30% 추가
    {
        player.add_item(ItemType::FISH_MACKEREL, 1);
        std::cout << "🐟 고등어 1마리 획득!\n";
        int extra_roll = std::rand() % 100;
        if (extra_roll < 30 + luk_bonus) {
            player.add_item(ItemType::FISH_MACKEREL, 1);
            std::cout << "🐟 고등어 추가 1마리 획득!\n";
        }
    }
    // 광어: 30%
    {
        int roll = std::rand() % 100;
        if (roll < 30 + luk_bonus) {
            player.add_item(ItemType::FISH_FLATFISH, 1);
            std::cout << "🐠 광어 1마리 획득!\n";
        }
    }
    // 참치: 5%
    {
        int roll = std::rand() % 100;
        if (roll < 5 + luk_bonus) {
            player.add_item(ItemType::FISH_TUNA, 1);
            std::cout << "🐋 참치 1마리 획득!\n";
        }
    }
}


void give_treasure(Player& player, int rod_grade, bool is_perfect) {
    if (rod_grade == 0) return;

    int box_type = 0; // 0: 없음, 1: 초급, 2: 중급, 3: 고급
    
    // 농기구 효과에 따른 보물상자 등급 결정
    if (is_perfect) {
        if (rod_grade == 1) box_type = 1;      // 동제
        else if (rod_grade == 2) box_type = 2; // 철제
        else if (rod_grade >= 3) box_type = 3; // 금제
    } else {
        // Great 시
        if (rod_grade == 2) box_type = 1;      // 철제
        else if (rod_grade >= 3) box_type = 2; // 금제
    }

    if (box_type == 0) return;

    std::string box_name = (box_type == 1) ? "초급 보물상자" :
                           (box_type == 2) ? "중급 보물상자" : "고급 보물상자";

    int roll = std::rand() % 100;
    std::cout << "🎁 " << box_name << " 발견!\n";

    // 5% 확률
    if (roll < 5) {
        int gold = (box_type == 1) ? 500 : (box_type == 2) ? 700 : 1200;
        std::cout << "   -> " << gold << "G 획득!\n";
    } 
    // 10% 확률 (누적 15%)
    else if (roll < 15) {
        if (box_type == 1) {
            player.add_item(ItemType::FISH_MACKEREL, 1);
            std::cout << "   -> 🐟 고등어 1마리 획득!\n";
        } else if (box_type == 2) {
            player.add_item(ItemType::FISH_FLATFISH, 1);
            std::cout << "   -> 🐠 광어 1마리 획득!\n";
        } else {
            player.add_item(ItemType::FISH_TUNA, 1);
            std::cout << "   -> 🐋 참치 1마리 획득!\n";
        }
    } 
    // 15% 확률 (누적 30%)
    else if (roll < 30) {
        int gold = (box_type == 1) ? 300 : (box_type == 2) ? 500 : 700;
        std::cout << "   -> " << gold << "G 획득!\n";
    } 
    // 55% 확률 (누적 85%)
    else if (roll < 85) {
        int gold = (box_type == 1) ? 200 : (box_type == 2) ? 400 : 500;
        std::cout << "   -> " << gold << "G 획득!\n";
    } 
    // 15% 확률 (누적 100%)
    else {
        if (box_type == 1) {
            std::cout << "   -> 100G 획득!\n";
        } else if (box_type == 2) {
            player.add_item(ItemType::FISH_MACKEREL, 1);
            std::cout << "   -> 🐟 고등어 1마리 획득!\n";
        } else {
            player.add_item(ItemType::FISH_FLATFISH, 1);
            std::cout << "   -> 🐠 광어 1마리 획득!\n";
        }
    }
}

// ============================================================
// 광산 미니게임 보조 (블랙잭형)
// ============================================================

const char* mine_hint(int slack, int pick_grade) {
    // slack = 붕괴 압력 - 누적 압력 (여유분)
    if (pick_grade == 0) return nullptr; // 곡괭이 없음: 힌트 없음

    // 1단계: 대사
    if (pick_grade >= 1) {
        if (slack >= 11) return "갱도가 아주 조용합니다. 바위 틈새로 은은한 광취가 풍깁니다.";
        if (slack >= 6)  return "쿠구구... 천장에서 가벼운 흙먼지가 조금씩 떨어지기 시작합니다.";
        if (slack >= 1)  return "🚨 위험! 갱도가 심하게 흔들리며 벽면에 거대한 금이 가기 시작합니다!";
        return "콰아아아앙! 암반이 압력을 버티지 못하고 무너져 내립니다!";
    }
    return nullptr;
}

void print_mine_status(int turns, int pressure, int collapse, int pick_grade, int lo, int hi) {
    std::cout << "\033[H\033[J";
    std::cout << "⛏️  [광산]\n";
    std::cout << "채광 횟수: " << turns << "/5  |  붕괴 압력: " << collapse << "\n";

    if (pick_grade == 0) {
        std::cout << "누적 압력: ??? (곡괭이 없음)\n";
    } else if (pick_grade == 1) {
        int slack = collapse - pressure;
        const char* hint = mine_hint(slack, pick_grade);
        std::cout << "갱도 상태: " << (hint ? hint : "알 수 없음") << "\n";
    } else {
        // 철제/금제 곡괭이
        int slack = collapse - pressure;
        const char* hint = mine_hint(slack, pick_grade);
        std::cout << "갱도 상태: " << (hint ? hint : "") << "\n";
        std::cout << "누적 압력 추정 범위: [" << lo << " ~ " << hi << "]\n";
    }
    std::cout << "\n";
}

} // anonymous namespace

namespace MiniGames {

void play_fishing(Player& player) {
    clear_console();
    show_cursor(true);
    drain_input();

    int stamina_cost = player.get_fishing_stamina_cost();
    std::cout << "\n🎣 강변입니다. 낚시를 시작하시겠습니까? (스태미나 " << stamina_cost << " 소모) (Y/N): " << std::flush;
    char ans = getch();
    if (ans != 'y' && ans != 'Y') {
        drain_input();
        clear_console();
        return;
    }

    if (!player.use_stamina(stamina_cost)) {
        std::cout << "\n스태미나가 부족합니다. (필요: " << stamina_cost << ")\n";
        wait_key();
        clear_console();
        return;
    }

    int rod_grade = player.get_rod_grade();
    FishingZones z = make_zones(rod_grade);

    const int BAR_WIDTH = z.bar_width;
    int pos = 0;
    int direction = 1;

    drain_input();
    {
        CursorGuard cg;
        while (!kbhit()) {
            render_fishing_frame(pos, z);
            usleep(25000);
            pos += direction;
            if (pos >= BAR_WIDTH - 1) { pos = BAR_WIDTH - 1; direction = -1; }
            else if (pos <= 0) { pos = 0; direction = 1; }
        }
    }
    getch();
    drain_input();
    clear_console();

    bool in_great   = (pos >= z.great_start && pos <= z.great_end);
    bool in_perfect = z.has_perfect && (pos >= z.perfect_start && pos <= z.perfect_end);

    if (in_great || in_perfect) {
        if (in_perfect) std::cout << "\n✨ PERFECT! 정확하게 맞췄습니다!\n";
        else std::cout << "\n👍 GREAT! 목표 구간 내에 들어왔습니다!\n";

        // LUK 보너스
        int luk_bonus = 0;
        if (player.has_luk_node(3)) luk_bonus += 10;
        if (player.has_luk_node(6)) luk_bonus += 10;

        // 독립 주사위로 생선 지급
        roll_fish_dice(player, luk_bonus);

        if (player.has_luk_node(6)) {
            int net_roll = std::rand() % 100;
            if (net_roll < 10) {
                player.add_item(ItemType::FISH_MACKEREL, 1);
                player.add_item(ItemType::FISH_FLATFISH, 1);
                player.add_item(ItemType::FISH_TUNA, 1);
                std::cout << "🎣 거대한 그물을 끌어올렸습니다! 고등어+광어+참치 각 1마리 추가!\n";
            }
        }

        // 보물상자
        if (rod_grade > 0) {
            give_treasure(player, rod_grade, in_perfect);
        }

        player.add_exp(6);
        std::cout << "XP +6\n";
    } else {
        std::cout << "\n앗! 물고기가 도망쳤습니다.\n";
        player.add_exp(3);
        std::cout << "XP +3\n";
    }

    wait_key();
    clear_console();
    drain_input();
}

void play_lotto_mining(Player& player) {
    clear_console();
    drain_input();

    int stamina_cost = player.get_mining_stamina_cost();
    std::cout << "\n⛏️  광산입니다. 채굴을 시작하시겠습니까? (스태미나 " << stamina_cost << " 소모) (Y/N): " << std::flush;
    char ans = getch();
    if (ans != 'y' && ans != 'Y') {
        drain_input();
        clear_console();
        return;
    }

    if (!player.use_stamina(stamina_cost)) {
        std::cout << "\n스태미나가 부족합니다. (필요: " << stamina_cost << ")\n";
        wait_key();
        clear_console();
        return;
    }

    // 붕괴 압력: 16~30 무작위
    int collapse_pressure = 16 + std::rand() % 15;
    int accumulated = 0;
    int turns = 0;
    int pick_grade = player.get_pick_grade();
    bool collapsed = false;
    
    // 구간 저장 변수 (초기값 설정)
    int current_lo = 0;
    int current_hi = 5;

    // ★ 수정 2: Y를 누른 직후 쓸데없이 멈춰있던 진입 화면 삭제! 바로 메인 루프로 들어감.
    // std::cout << "\n⛏️  채굴을 시작합니다. 붕괴 압력: " << collapse_pressure << "\n";
    // wait_key(); 

    while (turns < 5) {
        print_mine_status(turns, accumulated, collapse_pressure, pick_grade, current_lo, current_hi);

        std::cout << "1. 더 판다 (압력 +1~11 무작위 상승)\n";
        std::cout << "2. 그만 파고 탈출한다\n>> " << std::flush;
        char sel = getch();
        std::cout << "\n";

        if (sel == '2') {
            break;
        }
        if (sel != '1') continue;

        int add_pressure;
        if (pick_grade >= 3) {
            add_pressure = 1 + std::rand() % 10;
        } else {
            add_pressure = 1 + std::rand() % 11;
        }
        accumulated += add_pressure;
        ++turns;
        
        // 랜덤한 크기 6짜리 구간 생성
        if (pick_grade >= 2) {
            int min_lo = std::max(0, accumulated - 5);
            int max_lo = accumulated;
            
            current_lo = min_lo + (std::rand() % (max_lo - min_lo + 1));
            current_hi = current_lo + 5; // 구간 크기 6 보장
        }

        // 압력 변화 및 힌트 출력
        std::cout << " 누적: ";
        if (pick_grade == 0) std::cout << "???";
        else if (pick_grade >= 2) {
            std::cout << "[" << current_lo << "~" << current_hi << "]";
        } else {
            const char* hint = mine_hint(collapse_pressure - accumulated, pick_grade);
            std::cout << (hint ? hint : "???");
        }
        std::cout << "\n";

        // 붕괴 여부 체크
        if (accumulated >= collapse_pressure) {
            // 붕괴!
            bool survived = false;
            if (player.has_str_node(3)) {
                int roll = std::rand() % 100;
                if (roll < 30) {
                    survived = true;
                    std::cout << "콰아앙! 갱도가 무너지려 했으나, 당신의 강인한 어깨로 무너지는 바위를 순간적으로 받쳐냈습니다!\n";
                    std::cout << "기회는 단 한 번뿐입니다. 당장 탈출하십시오!\n";
                    wait_key();
                    --turns; // 마지막 채광은 취소
                    break;
                }
            }
            if (!survived && !survived && player.has_str_node(6)) {
                survived = true;
                accumulated -= add_pressure; // 마지막 압력 취소
                std::cout << "압력 초과! 하지만 당신은 헤라클레스입니다.\n";
                std::cout << "무너지는 암반을 힘으로 밀어올리며 마지막 발파를 무효화했습니다!\n";
                std::cout << "현재 누적 압력은 정확히 " << accumulated << " 입니다.\n";
                std::cout << "채광 횟수는 인정되었으니, 한 번 더 파내거나 안전하게 탈출하십시오.\n";
                wait_key();
                continue; 
            }
            if (!survived) {
                collapsed = true;
                std::cout << "콰아아아앙! 암반이 압력을 버티지 못하고 무너져 내립니다!\n";
                wait_key();
                break;
            }
        } else {
            // ★ 수정 1: 붕괴하지 않았을 경우 대사 출력 후, 화면이 지워지기 전에 잠깐 대기
            std::string reward_str;
            if (turns == 1) reward_str = "구리 1개";
            else if (turns == 2) reward_str = "구리 2개";
            else if (turns == 3) reward_str = "철 1개";
            else if (turns == 4) reward_str = "철 2개";
            else reward_str = "금 1개";

            std::cout << "✨ 무사히 채광했습니다. [현재 보상: " << reward_str << "]\n";
            
            // 5번째 채광이 아니면 다음 턴으로 가기 전에 결과를 읽을 수 있게 멈춤
            if (turns < 5) {
                std::cout << "(아무 키나 누르면 다음으로 진행합니다)" << std::flush;
                getch();
            }
        }
    }

    clear_console();

    if (collapsed) {
        player.add_exp(2);
        std::cout << "⛏️  광산이 붕괴되었습니다. XP +2\n";
    } else {
        // 보상 지급 로직은 기존과 동일
        bool hercules_double = player.has_str_node(6) && (turns == 5);
        int mul = hercules_double ? 2 : 1;

        if (turns <= 0) {
            std::cout << "채굴하지 않고 탈출했습니다.\n";
        } else if (turns <= 2) {
            int qty = (turns <= 1) ? 1 : 2;
            player.add_item(ItemType::COPPER, qty * mul);
            player.add_exp(2*turns);
            std::cout << "구리 광석 " << qty * mul << "개 획득! XP +" << (2 * turns) << "\n";
        } else if (turns <= 4) {
            int qty = (turns == 3) ? 1 : 2;
            player.add_item(ItemType::IRON, qty * mul);
            player.add_exp(2*turns);
            std::cout << "철 광석 " << qty * mul << "개 획득! XP +" << (2 * turns) << "\n";
        } else {
            player.add_item(ItemType::GOLD, 1 * mul);
            player.add_exp(5);
            std::cout << "금 광석 " << (1 * mul) << "개 획득! XP +10\n";
            if (hercules_double) std::cout << "⚡ 헤라클레스 보상 2배!\n";
        }
    }
    wait_key();
    clear_console();
}

} // namespace MiniGames
