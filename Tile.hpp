#ifndef TILE_HPP
#define TILE_HPP

#include <string>

class Crop;
class Livestock;

// 지형 종류를 정의하는 enum class
enum class TileType {
    Water, Riverbank, Livestock, Agriculture, Construction, Fence, Bush, GravelPath
};

// 각 칸(타일)의 데이터 구조체
struct Tile {
    TileType type;
    bool isWalkable;
    bool isOccupied;
    
    // 타일 위에 존재하는 객체들
    Crop* cropPtr;
    Livestock* animalPtr;
    std::string buildingName;

    // 기본 생성자
    Tile() : type(TileType::Agriculture), isWalkable(true), isOccupied(false), 
             cropPtr(nullptr), animalPtr(nullptr), buildingName("") {}
};

#endif // TILE_HPP