#ifndef FARMMAP_HPP
#define FARMMAP_HPP

#include "Tile.hpp"
#include <iostream>

class FarmMap {
private:
    Tile map[12][12];

    void initializeMap();

public:
    FarmMap();

    // 플레이어의 위치를 받아와서 같이 그리는 기능으로 변경됨
    void drawMap(int playerX, int playerY) const;

    Tile& getTile(int x, int y);
    const Tile& getTile(int x, int y) const;
    
    bool occupyTile(int x, int y);
    void freeTile(int x, int y);

    // 해당 좌표로 이동 가능한지(물이나 울타리가 아닌지) 확인하는 함수 추가
    bool isWalkable(int x, int y) const;
};

#endif