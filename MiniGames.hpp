#ifndef MINIGAMES_HPP
#define MINIGAMES_HPP

#include <vector>
#include <random>
#include "Player.hpp"

class FishingGame {
private:
    std::mt19937 gen;
public:
    FishingGame();
    // 실시간 타이밍 낚시 게임
    void playFishing(Player& player);
};

class LottoGame {
private:
    std::mt19937 gen;
public:
    LottoGame();
    // 플레이어가 6개의 번호를 직접 입력
    void playLotto(Player& player);
};

#endif