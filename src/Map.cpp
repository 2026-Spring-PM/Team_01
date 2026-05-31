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

    // 1. 침대 (2x2 고정)
    fill_region(tiles_, 10, 0, 11, 1, TileType::BED);
    
    // 2. 상점 (2x2 고정)
    fill_region(tiles_, 10, 7, 11, 8, TileType::MARKET);
    
    // 3. 주방 (x: 8~9, y: 2~3)
    fill_region(tiles_, 10, 3, 11, 3, TileType::KITCHEN);
    for(int y=3; y<=3; ++y) for(int x=10; x<=11; ++x) tiles_[y][x].building_name = "주방";

    // 4. 작업장 
    fill_region(tiles_, 10, 5, 11, 5, TileType::WORKSHOP);
    for(int y=5; y<=5; ++y) for(int x=10; x<=11; ++x) tiles_[y][x].building_name = "작업장";

    // 5. 광산 
    fill_region(tiles_, 10, 10, 11, 11, TileType::PATH);
    for(int y=10; y<=11; ++y) for(int x=10; x<=11; ++x) tiles_[y][x].building_name = "광산";
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
    int tree_count = 0;
    std::vector<std::pair<int, int>> empty_paths;

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            Tile& tile = tiles_[y][x];
            if (tile.has_crop()) CropSystem::grow_crop(tile);
            if (tile.has_animal()) LivestockSystem::grow_animal(tile);
            tile.watered_today = false;

            // 나무 개수 카운트 및 빈 타일 수집
            if (tile.type == TileType::PATH && tile.building_name == "나무") {
                tree_count++;
            } else if (tile.type == TileType::PATH && tile.building_name.empty() && !tile.is_occupied) {
                empty_paths.push_back({x, y});
            }
        }
    }

    // 🌲 랜덤 나무 생성 (최대 10그루, 매일 0~2그루 스폰)
    int max_trees = 10;
    int trees_to_spawn = std::rand() % 3; 
    while (tree_count < max_trees && trees_to_spawn > 0 && !empty_paths.empty()) {
        int idx = std::rand() % empty_paths.size();
        int px = empty_paths[idx].first;
        int py = empty_paths[idx].second;
        
        tiles_[py][px].building_name = "나무";

        empty_paths[idx] = empty_paths.back();
        empty_paths.pop_back();

        tree_count++;
        trees_to_spawn--;
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
