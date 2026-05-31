#pragma once

#include <string>

enum class TileType {
    WATER,      // 🟦 물가
    SAND,       // 🟨 강변 / 모래사장
    LIVESTOCK,  // 🟫 사육 공간
    FARM,       // 🟩 농업 공간
    PATH,       // ⬜ 자갈길

    BED,        // 🛏️ 침대
    MARKET,     // 🏪 시장
    KITCHEN,    // 🍳 주방
    WORKSHOP,    // 🔨 작업장
    STORAGE     // 📦 저장소 부지
};

struct Tile {
    TileType type = TileType::PATH;

    bool is_occupied = false;
    std::string building_name;
    bool has_wild_tree = false; // 새로 추가!

    // crop_id: 0 없음, 1 감자, 2 당근, 3 밀, 4 쌀, 5 사과나무
    int crop_id = 0;
    int crop_progress = 0;
    int crop_max_progress = 0;
    bool crop_perennial = false;
    int crop_harvest_cooldown = 0;
    bool watered_today = false;

    // animal_type: 0 없음, 1 닭, 2 돼지, 3 소
    bool animal_placed = false;
    int animal_type = 0;
    int animal_age = 0;
    int animal_adult_age = 0;
    bool animal_produced_today = false;

    bool is_walkable() const {
        // 농경지와 사육장은 작물/가축이 있어도 플레이어가 들어가 상호작용할 수 있게 둔다.
        if (type == TileType::WATER) return false;
        if (type == TileType::SAND) return true; // 강변에서 낚시 가능
        return true;
    }

    bool has_crop() const { return crop_id > 0; }
    bool has_animal() const { return animal_placed && animal_type > 0; }
};
