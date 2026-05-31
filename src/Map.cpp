#include "Map.h"
#include "CropSystem.h"
#include "LivestockSystem.h"

static void reset_tile(Tile& tile, TileType type) {
    tile = Tile{};
    tile.type = type;
}

static void fill_region(std::vector<std::vector<Tile>>& tiles,
                        int x1, int y1, int x2, int y2,
                        TileType type) {
    for (int y = y1; y <= y2; ++y) {
        for (int x = x1; x <= x2; ++x) {
            reset_tile(tiles[y][x], type);
        }
    }
}

Map::Map() : tiles_(HEIGHT, std::vector<Tile>(WIDTH)), out_of_bounds_tile_{} {
    // (맵 전체는 기본적으로 자갈길로 생성되므로, 덮어쓰기 방지를 위해 전체 덮기 코드는 삭제했습니다.)

    // 1. 물가와 모래사장 세팅
    fill_region(tiles_, 0, 0, 0, 11, TileType::WATER);
    fill_region(tiles_, 1, 0, 1, 11, TileType::SAND);

    // 2. 사육장
    fill_region(tiles_, 3, 0, 7, 2, TileType::LIVESTOCK);

    // 3. 농경지
    fill_region(tiles_, 3, 4, 6, 6, TileType::FARM);
    fill_region(tiles_, 3, 8, 6, 10, TileType::FARM);

    // 4. 건물 2x2 부지 할당
    fill_region(tiles_, BED_X, BED_Y, BED_X + 1, BED_Y + 1, TileType::BED);
    fill_region(tiles_, MARKET_X, MARKET_Y, MARKET_X + 1, MARKET_Y + 1, TileType::MARKET);
    fill_region(tiles_, KITCHEN_X, KITCHEN_Y, KITCHEN_X + 1, KITCHEN_Y + 1, TileType::KITCHEN);
    fill_region(tiles_, WORKSHOP_X, WORKSHOP_Y, WORKSHOP_X + 1, WORKSHOP_Y + 1, TileType::WORKSHOP);
    fill_region(tiles_, STORAGE_X, STORAGE_Y, STORAGE_X + 1, STORAGE_Y + 1, TileType::STORAGE);
}

const Tile& Map::get_tile(int x, int y) const {
    if (!is_in_bounds(x, y)) return out_of_bounds_tile_;
    return tiles_[y][x];
}

Tile& Map::get_tile(int x, int y) {
    if (!is_in_bounds(x, y)) return out_of_bounds_tile_;
    return tiles_[y][x];
}

bool Map::is_in_bounds(int x, int y) const {
    return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT;
}

void Map::advance_day() {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            Tile& tile = tiles_[y][x];
            if (tile.has_crop()) CropSystem::grow_crop(tile);
            if (tile.has_animal()) LivestockSystem::grow_animal(tile);
            tile.watered_today = false;
        }
    }
}

void Map::kill_unwatered_crops_during_drought() {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            Tile& tile = tiles_[y][x];
            if (tile.has_crop() && !tile.watered_today) {
                tile.crop_id = 0;
                tile.crop_progress = 0;
                tile.crop_max_progress = 0;
                tile.crop_perennial = false;
                tile.crop_harvest_cooldown = 0;
                tile.is_occupied = tile.has_animal() || !tile.building_name.empty();
            }
        }
    }
}