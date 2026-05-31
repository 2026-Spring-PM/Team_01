#include "MiniGames.h"
#include "Input.h"
#include "ItemType.h"

#include <cstdlib>
#include <iostream>
#include <vector>
#include <unistd.h>

namespace {
// 화면을 깔끔하게 지우기 위한 ANSI 탈출 시퀀스 함수
void clear_console() {
    std::cout << "\033[2J\033[H" << std::flush;
}

// [4번 기획] 보물상자 등급별 개봉 시 누적 확률 분포 룰렛 (5%, 10%, 15%, 55%, 15%)
void open_treasure_box(Player& player, ItemType box_type) {
    int roll = std::rand() % 100; // 0 ~ 99 사이 무작위 주사위
    
    if (box_type == ItemType::TREASURE_BOX_BASIC) {
        std::cout << "\n📦 [초급 보물상자]를 조심스럽게 열었습니다...\n";
        if (roll < 5) {
            player.add_money(500);
            std::cout << "✨ 대박! 500G가 쏟아졌습니다!\n";
        } else if (roll < 15) {
            player.add_item(ItemType::FISH_MACKEREL, 1);
            std::cout << "🐟 신선한 고등어 1마리가 들어있습니다.\n";
        } else if (roll < 30) {
            player.add_money(300);
            std::cout << "💰 300G를 획득했습니다.\n";
        } else if (roll < 85) {
            player.add_money(200);
            std::cout << "💰 200G를 획득했습니다.\n";
        } else {
            player.add_money(100);
            std::cout << "💸 소소하게 100G를 건졌습니다.\n";
        }
    } 
    else if (box_type == ItemType::TREASURE_BOX_NORMAL) {
        std::cout << "\n📦 [중급 보물상자]를 조심스럽게 열었습니다...\n";
        if (roll < 5) {
            player.add_money(700);
            std::cout << "✨ 대박! 700G가 쏟아졌습니다!\n";
        } else if (roll < 15) {
            player.add_item(ItemType::FISH_FLATFISH, 1);
            std::cout << "🐠 맛좋은 광어 1마리가 들어있습니다.\n";
        } else if (roll < 30) {
            player.add_money(500);
            std::cout << "💰 500G를 획득했습니다.\n";
        } else if (roll < 85) {
            player.add_money(400);
            std::cout << "💰 400G를 획득했습니다.\n";
        } else {
            player.add_item(ItemType::FISH_MACKEREL, 1);
            std::cout << "🐟 고등어 1마리를 획득했습니다.\n";
        }
    } 
    else if (box_type == ItemType::TREASURE_BOX_ADVANCED) {
        std::cout << "\n📦 [고급 보물상자]를 조심스럽게 열었습니다...\n";
        if (roll < 5) {
            player.add_money(1200);
            std::cout << "✨ 초대박!! 무려 1200G가 들어있습니다!\n";
        } else if (roll < 15) {
            player.add_item(ItemType::FISH_TUNA, 1);
            std::cout << "🐋 거대한 참치 1마리가 번쩍이며 튀어나옵니다!\n";
        } else if (roll < 30) {
            player.add_money(700);
            std::cout << "💰 700G를 획득했습니다.\n";
        } else if (roll < 85) {
            player.add_money(500);
            std::cout << "💰 500G를 획득했습니다.\n";
        } else {
            player.add_item(ItemType::FISH_FLATFISH, 1);
            std::cout << "🐠 광어 1마리를 획득했습니다.\n";
        }
    }
}
} // namespace

// ==========================================
// 4번: 낚시 미니게임 개편 구현
// ==========================================
void MiniGames::play_fishing(Player& player) {
    clear_console();
    std::cout << "🎣 [낚시 미니게임 시작]\n";

    // 낚싯대 등급 확인
    int rod_grade = player.get_fishing_rod_grade(); 
    // 금제(3단계 이상)는 3 소모, 그 미만은 기본 5 소모
    int stamina_cost = (rod_grade >= 3) ? 3 : 5;

    if (player.get_stamina() < stamina_cost) {
        std::cout << "⚠️ 스태미나가 부족하여 낚싯대를 던질 수 없습니다. (필요: " << stamina_cost << ")\n";
        wait_key();
        return;
    }

    player.use_stamina(stamina_cost);
    std::cout << "찌를 멀리 던졌습니다... (스태미나 -" << stamina_cost << ")\n";
    std::cout << "조금 기다리니 무언가 강력한 신호가 옵니다!\n\n";
    usleep(1000000); // 1초 대기 후 게임 시작

    // 전체 3칸 고정 스케일 (0: Great(좌), 1: Perfect(중앙), 2: Great(우))
    int player_pos = 0;
    int direction = 1;
    bool fishing_done = false;

    std::cout << "[팁] 콘솔창에 키를 입력하여 바가 가운데 녹색(Perfect)일 때 낚아채세요!\n";
    std::cout << "(※ 기본 낚싯대[0등급]는 Perfect 칸도 Great로 판정됩니다.)\n\n";
    std::cout << "준비되셨으면 아무 키나 눌러 타이밍 게이지를 시작하세요.";
    wait_key();

    // 타이밍 루프 (최대 30턴 동안 와다다 좌우 셔틀)
    for (int t = 0; t < 30; ++t) {
        clear_console();
        std::cout << "🎣 [타이밍에 맞춰 키를 누르세요!]\n\n";
        
        // 3칸 고정 스케일 UI 시각화
        if (player_pos == 0)      std::cout << "  [ 🟥 *Great* ] [  Perfect  ] [   Great   ]\n";
        else if (player_pos == 1) std::cout << "  [   Great   ] [ 🟩 *Perfect* ] [   Great   ]\n";
        else                      std::cout << "  [   Great   ] [  Perfect  ] [ 🟥 *Great* ]\n";

        std::cout << "\n👉 지금 낚아채려면 아무 키나 연타하여 입력하세요... ";
        std::flush(std::cout);
        
        usleep(180000); // 게이지 이동 속도 (0.18초)

        if (kbhit()) {
            getch(); // 입력 버퍼 비우기
            fishing_done = true;
            break;
        }

        // 방향 전환 벽 판정
        player_pos += direction;
        if (player_pos >= 2) { player_pos = 2; direction = -1; }
        if (player_pos <= 0) { player_pos = 0; direction = 1; }
    }

    clear_console();
    std::cout << "🎣 [낚시 결과 판정]\n";
    
    // 최종 판정 확정 (기본 낚싯대인 0단계는 1번에 멈춰도 Perfect가 아닌 Great 처리)
    bool is_perfect = (player_pos == 1 && rod_grade > 0);
    
    if (is_perfect) {
        std::cout << "✨ [PERFECT] 완벽한 타이밍에 릴을 감았습니다!\n\n";
    } else {
        std::cout << "👍 [GREAT] 안정적으로 물고기를 낚아 올렸습니다.\n\n";
    }

    // --- 생선 주사위 굴리기 (확정 보상) ---
    // 고등어: 100% 1마리 + 30% 확률로 1마리 더
    int mackerel_qty = 1 + ((std::rand() % 100 < 30) ? 1 : 0);
    player.add_item(ItemType::FISH_MACKEREL, mackerel_qty);
    std::cout << "🐟 고등어 x" << mackerel_qty << "마리를 낚았습니다.\n";

    // 광어: 30% 확률로 1마리
    if (std::rand() % 100 < 30) {
        player.add_item(ItemType::FISH_FLATFISH, 1);
        std::cout << "🐠 광어 x1마리를 낚았습니다. (30% 확률 당첨)\n";
    }
    // 참치: 5% 확률로 1마리
    if (std::rand() % 100 < 5) {
        player.add_item(ItemType::FISH_TUNA, 1);
        std::cout << "🐋 축하합니다! 대물 참어(참치) x1마리를 낚았습니다! (5% 확률 대성공!)\n";
    }

    // --- 보물상자 지급 및 즉시 개봉 로직 ---
    ItemType drop_box = ItemType::SEED_POTATO; // 임시 초기화
    bool has_box_dropped = false;

    if (rod_grade == 1) { // 동제 낚싯대
        if (is_perfect) { drop_box = ItemType::TREASURE_BOX_BASIC; has_box_dropped = true; }
    } 
    else if (rod_grade == 2) { // 철제 낚싯대
        if (is_perfect) { drop_box = ItemType::TREASURE_BOX_NORMAL; has_box_dropped = true; }
        else { drop_box = ItemType::TREASURE_BOX_BASIC; has_box_dropped = true; }
    } 
    else if (rod_grade >= 3) { // 금제 낚싯대
        if (is_perfect) { drop_box = ItemType::TREASURE_BOX_ADVANCED; has_box_dropped = true; }
        else { drop_box = ItemType::TREASURE_BOX_NORMAL; has_box_dropped = true; }
    }

    if (has_box_dropped) {
        open_treasure_box(player, drop_box);
    } else if (rod_grade == 0) {
        std::cout << "\nℹ️ 기본 낚싯대에서는 보물상자가 인양되지 않습니다. (동제 이상 업그레이드 필요)\n";
    }

    player.add_exp(5); // 보너스 숙련도 경험치 지급
    std::cout << "\n[아무 키나 누르면 마을로 돌아갑니다]\n";
    wait_key();
}

// ==========================================
// 5번: 광산 미니게임 '유사 블랙잭' 전면 개편 구현
// ==========================================
void MiniGames::play_lotto_mining(Player& player) {
    clear_console();
    std::cout << "⛏️ [광산 미니게임: 붕괴를 피해 광석을 캐라!]\n";

    int pickaxe_grade = player.get_pickaxe_grade();
    // 금제 곡괭이(3등급 이상)는 스태미나 소모 3, 나머지는 기본 5 소모
    int stamina_cost = (pickaxe_grade >= 3) ? 3 : 5;

    if (player.get_stamina() < stamina_cost) {
        std::cout << "⚠️ 스태미나가 부족하여 광산에 진입할 수 없습니다. (필요: " << stamina_cost << ")\n";
        wait_key();
        return;
    }

    player.use_stamina(stamina_cost);
    
    // 게임 변수 세팅
    int collapse_pressure = 16 + (std::rand() % 15); // [기획] 16 ~ 30 사이의 무작위 붕괴 한계치 설정
    int current_pressure = 0;
    int mine_count = 0; // 누적 성공 채광 횟수
    bool is_collapsed = false;
    bool is_escaped = false;

    std::cout << "어두컴컴하고 아슬아슬한 갱도 내부로 진입했습니다. (스태미나 -" << stamina_cost << ")\n";
    std::cout << "갱도가 무너지기 전까지 최대한 깊게 파고 안전하게 탈출해야 합니다!\n";
    wait_key();

    // 최대 5회 채광 찬스 루프
    while (mine_count < 5 && !is_collapsed && !is_escaped) {
        clear_console();
        std::cout << "⛏️ [채광 진행 - 현재 " << mine_count << "번 안쪽까지 진입 성공]\n";
        
        int remaining_pressure = collapse_pressure - current_pressure;

        // [기획] 곡괭이 등급별 실시간 가이드 및 정밀 진단 시스템
        if (pickaxe_grade == 0) {
            std::cout << "💬 (기본 곡괭이로는 무디고 둔하여 갱도의 미세한 압력 변화를 전혀 느낄 수 없습니다.)\n";
        } 
        else {
            // 1등급 이상일 때 여유 압력에 따른 대사 가이드 제공
            std::cout << "💬 [갱도 안정도 진단 대사]:\n ";
            if (remaining_pressure >= 11) {
                std::cout << "-> \"주변이 조용합니다. 바위 틈새로 은은한 원석의 기운이 평화롭게 풍깁니다.\"\n";
            } else if (remaining_pressure >= 6) {
                std::cout << "-> \"쿠구구... 천장에서 가벼운 흙먼지와 작은 돌멩이가 조금씩 떨어집니다.\"\n";
            } else if (remaining_pressure >= 1) {
                std::cout << "🚨 -> \"위험!! 갱도 전체가 요란하게 흔들리며 지지대에 거대한 금이 갑니다!\"\n";
            }

            // 철제(2등급), 금제(3등급)일 때 정밀 오차 범위 수치화 정보 추가 제공
            if (pickaxe_grade >= 2) {
                // 현재 누적 압력을 무조건 포함하는 크기 6짜리 무작위 밴드 가이드 생성
                int offset = std::rand() % 6; // 0~5
                int min_range = current_pressure - offset;
                if (min_range < 0) min_range = 0;
                int max_range = min_range + 5;

                std::cout << "🔍 [곡괭이 정밀 센서 구동] 추정 누적 압력: [" << min_range << " ~ " << max_range << "]\n";
                std::cout << "🎯 목표 임계 압력 한계점: [" << collapse_pressure << "]\n";
            }
        }

        std::cout << "\n-------------------------------------\n";
        std::cout << "현재 채광 성공 횟수: " << mine_count << " / 5회\n";
        std::cout << "1. 더 깊게 판다 (위험 감수 및 압력 가중)\n";
        std::cout << "2. 광석을 챙겨 탈출한다 (현재 안전하게 보상 수령)\n";
        std::cout << "-------------------------------------\n";
        std::cout << "행동을 선택하세요: ";
        
        int menu_select = read_int_safe(2);
        
        if (menu_select == 1) {
            mine_count++;
            // 금제 곡괭이는 1~10 상승 버프 범위, 그 미만은 1~11 무작위 상승
            int pressure_inc = 1 + (std::rand() % ((pickaxe_grade >= 3) ? 10 : 11));
            current_pressure += pressure_inc;

            std::cout << "\n💥 깡!! 벽을 힘차게 쪼자 압력이 누적되었습니다! (압력 +" << pressure_inc << ")\n";
            
            // 붕괴 한계점 도달 여부 확인 (유사 블랙잭 버스트 판정)
            if (current_pressure >= collapse_pressure) {
                is_collapsed = true;
                std::cout << "⚠️ 쿠콰콰쾅!!! 암반이 한계 압력을 버티지 못하고 붕괴되었습니다!\n";
                wait_key();
            } else {
                std::cout << "아직은 무너지지 않고 버텨주었습니다! 안전지대를 확보합니다.\n";
                wait_key();
            }
        } 
        else {
            is_escaped = true; // 안전하게 탈출 선언
        }
    }

    // --- 최종 보상 정산 페이즈 ---
    clear_console();
    std::cout << "⛏️ [광산 미니게임 정산소]\n";
    std::cout << "최종 결과 -> [내 누적 압력]: " << current_pressure << " / [붕괴 한계 압력]: " << collapse_pressure << "\n\n";

    if (is_collapsed || mine_count == 0) {
        std::cout << "❌ 광산이 완전히 폭발·무너지거나 아무 작업도 안 하고 도망쳤습니다.\n";
        std::cout << "허탈하게 맨몸으로 기어 나왔으므로 획득한 광석이 전부 유실되었습니다. (보상 없음)\n";
    } 
    else {
        std::cout << "🎉 안전하게 지상으로 생환했습니다! 채광 성과에 따른 보상이 지급됩니다.\n";
        std::cout << "=========================================\n";
        
        // 기획에 맞춘 누적 횟수별 차등 보상 지급
        if (mine_count == 1) {
            player.add_item(ItemType::COPPER, 1);
            std::cout << "🥉 1차 심도 돌파: 구리 광석 x1개를 배낭에 챙겼습니다.\n";
        } else if (mine_count == 2) {
            player.add_item(ItemType::COPPER, 2);
            std::cout << "🥉 2차 심도 돌파: 구리 광석 x2개를 배낭에 챙겼습니다.\n";
        } else if (mine_count == 3) {
            player.add_item(ItemType::IRON, 1);
            std::cout << "🥈 3차 심도 돌파: 철 광석 x1개를 배낭에 챙겼습니다.\n";
        } else if (mine_count == 4) {
            player.add_item(ItemType::IRON, 2);
            std::cout << "🥈 4차 심도 돌파: 철 광석 x2개를 배낭에 챙겼습니다.\n";
        } else if (mine_count == 5) {
            player.add_item(ItemType::GOLD, 1);
            std::cout << "🥇 5차 최고 심도 정복 완수!!: 귀중한 금 광석 x1개를 캐냈습니다!\n";
        }
        
        player.add_exp(8); // 광산 탐험 완료 경험치 가산
        std::cout << "=========================================\n";
    }

    std::cout << "\n[아무 키나 누르면 광산을 나갑니다]\n";
    wait_key();
}