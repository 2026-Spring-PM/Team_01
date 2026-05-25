#pragma once

// 맵의 한 칸이 어떤 종류인지 표현
enum class TileType {
    WATER,      // 🟦 물가 (낚시터, 이동 불가)
    SAND,       // 🟨 모래사장 (이동 불가)
    LIVESTOCK,  // 🟫 사육 공간
    FARM,       // 🟩 농업 공간
    PATH,       // ⬜ 마당 자갈길
    BUILDING,   // 🏡 건축 공간
    BED,        // 🛏️ 침대 (잠자기)
    MARKET,     // 🏪 시장 (피에르/루이스)
    KITCHEN,    // 🍳 주방 (요리)
    WORKSHOP,   // 🔨 제작소 (농기구)
};

// 맵의 한 칸을 표현하는 구조체
struct Tile {
    TileType type;

    bool is_walkable() const {
        return type != TileType::WATER && type != TileType::SAND;
    }

    bool is_bed() const {
        return type == TileType::BED;
    }

    bool is_market() const {
        return type == TileType::MARKET;
    }

    bool is_kitchen() const {
        return type == TileType::KITCHEN;
    }

    bool is_workshop() const {
        return type == TileType::WORKSHOP;
    }
};