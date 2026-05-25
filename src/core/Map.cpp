#include "Map.h"

static void fill_region(std::vector<std::vector<Tile>>& tiles,
                        int x1, int y1, int x2, int y2,
                        TileType type) {
    for (int y = y1; y <= y2; ++y) {
        for (int x = x1; x <= x2; ++x) {
            tiles[y][x].type = type;
        }
    }
}

Map::Map() {
    // 기본값은 PATH (마당)로 초기화
    tiles_.resize(HEIGHT, std::vector<Tile>(WIDTH, Tile{TileType::PATH}));

    // === 왼쪽 ===
    fill_region(tiles_, 0, 0, 0, 11, TileType::WATER);   // 물가 (x=0)
    fill_region(tiles_, 1, 0, 1, 11, TileType::SAND);    // 모래사장 (x=1)
    // x=2 는 세로 마당 (기본값 PATH 유지)

    // === 사육 공간 5x3 (x=3~7, y=0~2) ===
    fill_region(tiles_, 3, 0, 7, 2, TileType::LIVESTOCK);

    // === 밭 1 (4x3, x=3~6, y=4~6) ===
    fill_region(tiles_, 3, 4, 6, 6, TileType::FARM);

    // === 밭 2 (4x3, x=3~6, y=8~10) ===
    fill_region(tiles_, 3, 8, 6, 10, TileType::FARM);

    // 가로 사잇길 (y=3, y=7, y=11) 은 기본값 PATH 유지

    // === 집 (오른쪽 위, x=9~11, y=0~2) ===
    fill_region(tiles_, 9, 0, 11, 2, TileType::BUILDING);

    // === 특수 타일 ===
    tiles_[BED_Y][BED_X].type = TileType::BED;            // 침대 (10, 1)
    tiles_[MARKET_Y][MARKET_X].type = TileType::MARKET;   // 시장 (10, 5)
    tiles_[KITCHEN_Y][KITCHEN_X].type = TileType::KITCHEN;     // 주방 (9, 10)
    tiles_[WORKSHOP_Y][WORKSHOP_X].type = TileType::WORKSHOP;  // 제작소 (11, 10)

    // 범위 밖 타일은 모래사장(이동 불가)
    out_of_bounds_tile_.type = TileType::SAND;
}

bool Map::is_in_bounds(int x, int y) const {
    return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT;
}

const Tile& Map::get_tile(int x, int y) const {
    if (!is_in_bounds(x, y)) {
        return out_of_bounds_tile_;
    }
    return tiles_[y][x];
}