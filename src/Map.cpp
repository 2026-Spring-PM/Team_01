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
    fill_region(tiles_, 0, 0, WIDTH - 1, HEIGHT - 1, TileType::PATH);

    fill_region(tiles_, 0, 0, 0, 11, TileType::WATER);
    fill_region(tiles_, 1, 0, 1, 11, TileType::SAND);

    fill_region(tiles_, 3, 0, 7, 2, TileType::LIVESTOCK);

    fill_region(tiles_, 3, 4, 6, 6, TileType::FARM);
    fill_region(tiles_, 3, 8, 6, 10, TileType::FARM);

    fill_region(tiles_, 9, 0, 11, 3, TileType::BUILDING);
    reset_tile(tiles_[BED_Y][BED_X], TileType::BED);

    reset_tile(tiles_[MARKET_Y][MARKET_X], TileType::MARKET);
    // 주방과 작업장은 처음부터 존재하지 않습니다.
    // BUILDING 타일에서 J 상호작용으로 직접 건설해야 🍳/🔨 건물이 생깁니다.
    // 기존 KITCHEN_X/Y, WORKSHOP_X/Y 좌표는 상수로만 남겨 두고, 시작 맵에는 배치하지 않습니다.
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
