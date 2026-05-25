#ifndef EVENTMANAGER_HPP
#define EVENTMANAGER_HPP

#include <vector>
#include "Crop.hpp"

class Player; // 전방 선언

class EventManager {
private:
    int droughtTimer;
    bool summerDroughtOccurred;

public:
    EventManager() : droughtTimer(0), summerDroughtOccurred(false) {}
    
    // 🌟 수정됨: 복잡한 변수 대신 Player 객체를 통째로 받습니다.
    void triggerEvents(Player& player, bool isSummer);
    
    void processCrops(std::vector<Crop*>& crops, bool watered);
};

#endif