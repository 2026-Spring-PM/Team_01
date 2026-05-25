#include "MiniGames.hpp"
#include "Player.hpp"
#include <iostream>
#include <vector>
#include <set>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

// 터미널 키 입력을 즉시 감지하기 위한 함수 (Linux/Mac용)
int kbhit(void) {
    struct termios oldt, newt;
    int ch, oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

FishingGame::FishingGame() { std::random_device rd; gen.seed(rd()); }

void FishingGame::playFishing(Player& player) {
    if (!player.consumeStamina(30)) return;

    int pos = 0;
    int direction = 1;
    const int BAR_WIDTH = 20; // 낚시 게이지 전체 길이
    
    // 🌟 [핵심 변경] 레벨 기반 동적 난이도 조절 시스템
    int playerLevel = player.getLevel();
    int hitZoneWidth = 1 + (playerLevel / 10); // 기본 1칸, 10레벨당 1칸씩 증가

    // 게이지 중앙(인덱스 10)을 기준으로 히트 구역 배치
    int hitStart = 10 - (hitZoneWidth / 2);
    int hitEnd = hitStart + hitZoneWidth - 1;

    // 히트 존이 화면 밖으로 나가지 않도록 안전장치
    if (hitStart < 0) hitStart = 0;
    if (hitEnd >= BAR_WIDTH) hitEnd = BAR_WIDTH - 1;

    while (!kbhit()) {
        system("clear"); // 프레임마다 화면을 지워 애니메이션 구현
        std::cout << "🎣 [낚시 미니게임 개시] (현재 낚시 숙련도 Lv." << playerLevel << ")\n";
        std::cout << "찌가 흔들립니다! 화살표(▲)가 히트 구역[🟩]에 도달했을 때 아무 키나 누르세요!\n\n";

        // 게이지 바 출력
        std::cout << "구간: [";
        for (int i = 0; i < BAR_WIDTH; ++i) {
            if (i >= hitStart && i <= hitEnd) {
                std::cout << "🟩"; // 정답 구간
            } else {
                std::cout << "▒▒"; // 일반 구간
            }
        }
        std::cout << "]\n";

        // 화살표 바늘 출력 (한 칸당 띄어쓰기 2칸으로 🟩 이모지와 너비를 맞춤)
        std::cout << "바늘:  ";
        for (int i = 0; i < pos; ++i) std::cout << "  ";
        std::cout << "▲\n";

        usleep(80000); // 바늘 이동 속도 (0.08초)

        // 바늘 왕복 이동 로직
        pos += direction;
        if (pos >= BAR_WIDTH - 1 || pos <= 0) direction *= -1;
    }

    getchar(); // 누른 키 버퍼 비우기

    // 판정 로직
    if (pos >= hitStart && pos <= hitEnd) {
        // 성공 시 어종 추첨 (고등어 70%, 광어 25%, 참치 5%)
        std::discrete_distribution<int> fishDist{70, 25, 5};
        int f = fishDist(gen);
        std::string fishName = "";
        
        if (f == 0) { fishName = "고등어"; std::cout << "\n🐟 성공! 은빛의 고등어를 낚았습니다!\n"; }
        else if (f == 1) { fishName = "광어"; std::cout << "\n🐠 대성공! 펄떡이는 광어를 낚았습니다!\n"; }
        else { fishName = "참치"; std::cout << "\n🐋 [대박!] 거대한 참치를 끌어올렸습니다!\n"; }
        
        player.addItem(fishName, 1);
        player.earnXP(60);
    } else {
        std::cout << "\n💨 앗! 타이밍을 놓쳐 물고기가 도망쳤습니다.\n";
    }
}


LottoGame::LottoGame() { std::random_device rd; gen.seed(rd()); }

void LottoGame::playLotto(Player& player) {
    if (!player.consumeStamina(30)) return;

    std::vector<int> userNums;
    std::cout << ">> 0~9 사이의 로또 번호 6개를 차례대로 입력하고 엔터를 치세요:\n";
    while(userNums.size() < 6) {
        int n;
        std::cin >> n;
        if(n < 0 || n > 9) { 
            std::cout << "0~9만 가능합니다. 다시 입력하세요.\n"; 
            continue; 
        }
        userNums.push_back(n);
    }

    std::set<int> winSet;
    while(winSet.size() < 6) winSet.insert(gen() % 10);

    int match = 0;
    for(int n : userNums) if(winSet.count(n)) match++;

    std::cout << "\n당첨 번호: ";
    for(int n : winSet) std::cout << n << " ";
    std::cout << "\n🎉 " << match << "개 일치!\n";
    
    // 로또 보상 로직
    if (match == 3) {
        player.addItem("구리 광석", 1);
        std::cout << "🎁 보상으로 [구리 광석] 1개를 획득했습니다!\n";
        player.earnXP(150);
    } else if (match == 4) {
        player.addItem("철 광석", 1);
        std::cout << "🎁 보상으로 [철 광석] 1개를 획득했습니다!\n";
        player.earnXP(200);
    } else if (match == 5) {
        player.addItem("금 광석", 1);
        std::cout << "🎁 보상으로 [금 광석] 1개를 획득했습니다!\n";
        player.earnXP(250);
    } else if (match == 6) {
        player.addItem("구리 광석", 1);
        player.addItem("철 광석", 1);
        player.addItem("금 광석", 1);
        std::cout << "🎁 잭팟! 보상으로 [구리, 철, 금 광석]을 모두 1개씩 획득했습니다!\n";
        player.earnXP(300);
    } else {
        std::cout << "아쉽게도 꽝입니다.\n";
    }
}