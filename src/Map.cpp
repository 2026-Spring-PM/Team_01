#include "Map.h"
#include "CropSystem.h"
#include "LivestockSystem.h"

#include <cstdlib>

namespace {
constexpr int MAX_TREE_COUNT = 6;
constexpr int INITIAL_TREE_COUNT = 4;
constexpr int TREE_SPAWN_ROLLS_PER_DAY = 2;
constexpr int TREE_SPAWN_CHANCE_PERCENT = 35;
}

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

static void mark_empty_build_plot(std::vector<std::vector<Tile>>& tiles, int ax, int ay) {
    for (int yy = ay; yy < ay + Map::BUILD_PLOT_SIZE; ++yy) {
        for (int xx = ax; xx < ax + Map::BUILD_PLOT_SIZE; ++xx) {
            reset_tile(tiles[yy][xx], TileType::BUILDING);
            Tile& t = tiles[yy][xx];
            t.building_origin = (xx == ax && yy == ay);
            t.building_origin_x = ax;
            t.building_origin_y = ay;
            t.building_level = 1;
        }
    }
}

static void place_fixed_2x2_building(std::vector<std::vector<Tile>>& tiles,
                                     int ax, int ay, TileType type,
                                     const std::string& name) {
    for (int yy = ay; yy < ay + Map::BUILD_PLOT_SIZE; ++yy) {
        for (int xx = ax; xx < ax + Map::BUILD_PLOT_SIZE; ++xx) {
            reset_tile(tiles[yy][xx], type);
            Tile& t = tiles[yy][xx];
            t.is_occupied = true;
            t.building_name = name;
            t.building_origin = (xx == ax && yy == ay);
            t.building_origin_x = ax;
            t.building_origin_y = ay;
            t.building_level = 1;
        }
    }
    const int door_x = ax;
    const int door_y = ay + Map::BUILD_PLOT_SIZE;
    if (door_y >= 0 && door_y < Map::HEIGHT && door_x >= 0 && door_x < Map::WIDTH) {
        Tile& door = tiles[door_y][door_x];
        if (door.type != TileType::WATER && door.type != TileType::BUILDING && door.building_name.empty()) {
            door.door_target_name = name;
        }
    }
}

Map::Map() : tiles_(HEIGHT, std::vector<Tile>(WIDTH)), out_of_bounds_tile_{} {
    fill_region(tiles_, 0, 0, WIDTH - 1, HEIGHT - 1, TileType::PATH);

    fill_region(tiles_, 0, 0, 0, 11, TileType::WATER);
    fill_region(tiles_, 1, 0, 1, 11, TileType::SAND);

    // 기존 메인 맵 사육 타일은 축사 내부 맵으로 대체한다. 이 영역은 길/건축 부지로 사용한다.

    fill_region(tiles_, 3, 4, 6, 6, TileType::FARM);
    fill_region(tiles_, 3, 8, 6, 10, TileType::FARM);

    // 건축 부지는 요청 좌표 (11,1), (11,4), (11,7)를 0-based 좌표로 변환해 배치한다.
    // 세 부지 모두 2x2 크기이며, 건물 건설 전에는 플레이어가 부지 위로 올라가 건설할 수 있다.
    const int anchors[BUILD_PLOT_COUNT][2] = {
        {BUILD_PLOT_AX0, BUILD_PLOT_AY0},
        {BUILD_PLOT_AX1, BUILD_PLOT_AY1},
        {BUILD_PLOT_AX2, BUILD_PLOT_AY2}
    };
    for (const auto& a : anchors) {
        mark_empty_build_plot(tiles_, a[0], a[1]);
    }

    // 상점은 요청 좌표 (9,4)를 0-based 좌표 (8,3)으로 변환하여 2x2 건물로 고정 배치한다.
    // 상점 위로는 이동할 수 없고, 바로 아래 문 앞 타일에서만 상호작용한다.
    place_fixed_2x2_building(tiles_, MARKET_X, MARKET_Y, TileType::MARKET, "상점");

    // 축사는 게임 시작 시 기본 설치된다.
    place_fixed_2x2_building(tiles_, BARN_X, BARN_Y, TileType::BUILDING, "축사");

    // 침대는 광산 근처에 배치한다.
    reset_tile(tiles_[BED_Y][BED_X], TileType::BED);

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

bool Map::is_build_plot_anchor(int ax, int ay) const {
    return (ax == BUILD_PLOT_AX0 && ay == BUILD_PLOT_AY0) ||
           (ax == BUILD_PLOT_AX1 && ay == BUILD_PLOT_AY1) ||
           (ax == BUILD_PLOT_AX2 && ay == BUILD_PLOT_AY2);
}

bool Map::find_build_plot_anchor(int x, int y, int& ax, int& ay) const {
    const int anchors[BUILD_PLOT_COUNT][2] = {
        {BUILD_PLOT_AX0, BUILD_PLOT_AY0},
        {BUILD_PLOT_AX1, BUILD_PLOT_AY1},
        {BUILD_PLOT_AX2, BUILD_PLOT_AY2}
    };
    for (const auto& a : anchors) {
        if (x >= a[0] && x < a[0] + BUILD_PLOT_SIZE &&
            y >= a[1] && y < a[1] + BUILD_PLOT_SIZE) {
            ax = a[0];
            ay = a[1];
            return true;
        }
    }
    return false;
}

bool Map::is_building_door(int x, int y) const {
    return is_in_bounds(x, y) && !tiles_[y][x].door_target_name.empty();
}

const std::string& Map::door_target_at(int x, int y) const {
    static const std::string empty;
    if (!is_in_bounds(x, y)) return empty;
    return tiles_[y][x].door_target_name;
}

bool Map::place_facility_2x2(int ax, int ay, const std::string& name) {
    if (!is_build_plot_anchor(ax, ay)) return false;
    for (int yy = ay; yy < ay + BUILD_PLOT_SIZE; ++yy) {
        for (int xx = ax; xx < ax + BUILD_PLOT_SIZE; ++xx) {
            if (!is_in_bounds(xx, yy)) return false;
            Tile& t = tiles_[yy][xx];
            if (t.type != TileType::BUILDING || !t.building_name.empty()) return false;
        }
    }

    for (int yy = ay; yy < ay + BUILD_PLOT_SIZE; ++yy) {
        for (int xx = ax; xx < ax + BUILD_PLOT_SIZE; ++xx) {
            Tile& t = tiles_[yy][xx];
            t.type = TileType::BUILDING;
            t.is_occupied = true;
            t.building_name = name;
            t.building_origin = (xx == ax && yy == ay);
            t.building_origin_x = ax;
            t.building_origin_y = ay;
            t.building_level = 1;
        }
    }

    // 문은 2x2 건물의 아래쪽 왼쪽 길 타일에 둔다. 그 위치에서만 상호작용 가능하다.
    const int door_x = ax;
    const int door_y = ay + BUILD_PLOT_SIZE;
    if (is_in_bounds(door_x, door_y)) {
        Tile& door = tiles_[door_y][door_x];
        if (door.type != TileType::WATER && door.type != TileType::BUILDING) {
            door.door_target_name = name;
        }
    }
    return true;
}

int Map::count_trees() const {
    int count = 0;
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            if (tiles_[y][x].type == TileType::TREE) ++count;
        }
    }
    return count;
}

bool Map::spawn_one_tree() {
    if (count_trees() >= MAX_TREE_COUNT) return false;
    for (int attempt = 0; attempt < 60; ++attempt) {
        int x = std::rand() % WIDTH;
        int y = std::rand() % HEIGHT;
        Tile& tile = tiles_[y][x];
        if (tile.type != TileType::PATH) continue;
        if (x == MINE_X && y == MINE_Y) continue;
        if (x == BED_X && y == BED_Y) continue;
        if (!tile.building_name.empty() || !tile.door_target_name.empty() || tile.has_crop() || tile.has_animal()) continue;
        tile.type = TileType::TREE;
        return true;
    }
    return false;
}

void Map::spawn_initial_trees() {
    while (count_trees() < INITIAL_TREE_COUNT) {
        if (!spawn_one_tree()) break;
    }
}

void Map::spawn_random_trees() {
    for (int roll = 0; roll < TREE_SPAWN_ROLLS_PER_DAY; ++roll) {
        if (std::rand() % 100 >= TREE_SPAWN_CHANCE_PERCENT) continue;
        if (!spawn_one_tree()) break;
    }
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
    spawn_random_trees();
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
