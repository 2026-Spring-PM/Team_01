#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "LevelSystem.hpp"
#include <iostream>
#include <map>
#include <string>

class EventManager;
class LottoGame;

class Player {
    friend class EventManager;
    friend class LottoGame;

private:
    int maxStamina;
    int stamina;
    int gold;
    int currentToolGrade;
    LevelSystem levelSystem;
    
    int x, y; 
    
    // 🌟 추가됨: 플레이어 인벤토리 (씨앗, 가축 등 보관)
    std::map<std::string, int> inventory;

    void handleLevelUp(int reachedLevel);

public:
    Player();

    bool consumeStamina(int amount);
    void sleep();
    void earnXP(int amount);

    int getStamina() const;
    int getMaxStamina() const;
    int getGold() const;
    void addGold(int amount);
    bool spendGold(int amount);
    int getToolGrade() const;
    void setToolGrade(int grade);
    int getLevel() const;

    int getX() const { return x; }
    int getY() const { return y; }
    void setPos(int newX, int newY) { x = newX; y = newY; }

    // 🌟 인벤토리 관리 함수
    void addItem(const std::string& name, int count) { inventory[name] += count; }
    void removeItem(const std::string& name, int count) { inventory[name] -= count; }
    int getItemCount(const std::string& name) { return inventory[name]; }
};

#endif