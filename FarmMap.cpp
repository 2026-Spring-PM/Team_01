#include "FarmMap.hpp"
#include "Crop.hpp"      
#include "Livestock.hpp" 

FarmMap::FarmMap() {
    initializeMap();
}

void FarmMap::initializeMap() {
    for (int y = 0; y < 12; ++y) {
        for (int x = 0; x < 12; ++x) {
            Tile& tile = map[y][x];
            tile.isOccupied = false;
            tile.isWalkable = true;

            if (x <= 1) {
                tile.type = TileType::Water;
                tile.isWalkable = false;
            } else if (x == 2) {
                tile.type = TileType::Riverbank;
            } else if (x >= 3 && x <= 6 && y <= 4) {
                tile.type = TileType::Livestock;
            } else if (x >= 3 && x <= 6 && y >= 5) {
                tile.type = TileType::Agriculture;
            } else if (x >= 7 && x <= 11) {
                tile.type = TileType::Construction;
            }
            
            if (x == 7 && y <= 4) {
                tile.type = TileType::Fence;
                tile.isWalkable = false;
            }
            if (x == 7 && y >= 5) {
                if (y % 2 == 0) {
                    tile.type = TileType::Bush;
                    tile.isWalkable = false;
                } else {
                    tile.type = TileType::GravelPath;
                }
            }
        }
    }
}

void FarmMap::drawMap(int playerX, int playerY) const {
    std::cout << "\n🗺️ 팜 빌리지 맵 (W/A/S/D 이동)\n";
    std::cout << "  0 1 2 3 4 5 6 7 8 9 1011(X)\n"; 
    
    for (int y = 0; y < 12; ++y) {
        if (y < 10) std::cout << y << " ";
        else std::cout << y;
        
        for (int x = 0; x < 12; ++x) {
            if (x == playerX && y == playerY) {
                std::cout << "🧑‍🌾";
                continue;
            }

            // 🌟 [추가 변경 사항] 11, 11 좌표는 항상 광산지대(⛏️) 고유 표식 렌더링
            if (x == 11 && y == 11) {
                std::cout << "⛏️";
                continue;
            }

            if (map[y][x].isOccupied) {
                if (map[y][x].cropPtr != nullptr) {
                    if (map[y][x].cropPtr->getIsReady()) {
                        std::string cName = map[y][x].cropPtr->getName();
                        if (cName == "감자") std::cout << "🥔";
                        else if (cName == "당근") std::cout << "🥕";
                        else if (cName == "밀" || cName == "쌀") std::cout << "🌾";
                        else if (cName == "사과나무") std::cout << "🍎";
                        else std::cout << "🌿";
                    } else {
                        std::cout << "🌱"; 
                    }
                }
                else if (map[y][x].animalPtr != nullptr) {
                    std::string aName = map[y][x].animalPtr->getName();
                    if (map[y][x].animalPtr->getIsAdult()) {
                        if (aName == "닭") std::cout << "🐔";
                        else if (aName == "돼지") std::cout << "🐖";
                        else if (aName == "소") std::cout << "🐄";
                        else std::cout << "🐾";
                    } else {
                        if (aName == "닭") std::cout << "🐣";   
                        else if (aName == "돼지") std::cout << "🐷"; 
                        else if (aName == "소") std::cout << "🐂";   
                        else std::cout << "🐾";
                    }
                }
                else if (map[y][x].buildingName != "") {
                    if (map[y][x].buildingName == "주방") std::cout << "🍳";
                    else if (map[y][x].buildingName == "작업장") std::cout << "🔨";
                    else std::cout << "🏠";
                }
                continue;
            }

            switch (map[y][x].type) {
                case TileType::Water:        std::cout << "🟦"; break;
                case TileType::Riverbank:    std::cout << "🟨"; break;
                case TileType::Livestock:    std::cout << "🟫"; break;
                case TileType::Agriculture:  std::cout << "🟩"; break;
                case TileType::Construction: std::cout << "🟪"; break;
                case TileType::Fence:        std::cout << "🪵"; break;
                case TileType::Bush:         std::cout << "🌿"; break;
                case TileType::GravelPath:   std::cout << "⬜"; break;
                default:                     std::cout << "⬛"; break;
            }
        }
        std::cout << "\n";
    }
}

Tile& FarmMap::getTile(int x, int y) { return map[y][x]; }
const Tile& FarmMap::getTile(int x, int y) const { return map[y][x]; }

bool FarmMap::occupyTile(int x, int y) {
    if (x < 0 || x >= 12 || y < 0 || y >= 12) return false;
    if (map[y][x].isOccupied || !map[y][x].isWalkable) return false;
    map[y][x].isOccupied = true;
    return true;
}

void FarmMap::freeTile(int x, int y) {
    if (x >= 0 && x < 12 && y >= 0 && y < 12) {
        map[y][x].isOccupied = false;
    }
}

bool FarmMap::isWalkable(int x, int y) const {
    if (x < 0 || x >= 12 || y < 0 || y >= 12) return false;
    return map[y][x].isWalkable;
}