#include "CropSystem.h"

#include <cstdlib>
#include <sstream>

// 작물 데이터 테이블
// { id, seed, product, name, growth_days, perennial, min_yield, max_yield, xp }
static const CropInfo CROPS[] = {
    {1, ItemType::SEED_POTATO, ItemType::POTATO, "감자", 3, false, 2, 4, 3},
    {2, ItemType::SEED_CARROT, ItemType::CARROT, "당근", 4, false, 2, 4, 4},
    {3, ItemType::SEED_WHEAT,  ItemType::WHEAT,  "밀",   5, false, 3, 5, 5},
    {4, ItemType::SEED_RICE,   ItemType::RICE,   "쌀",   6, false, 2, 4, 6},
    {5, ItemType::SEED_APPLE,  ItemType::APPLE,  "사과나무", 7, true, 1, 3, 8},
};

static constexpr int PERENNIAL_COOLDOWN = 3; // 다년생 작물 재성장 대기 일수
static constexpr int PLANT_STAMINA_COST = 1;
static constexpr int WATER_STAMINA_COST = 2;

const CropInfo* CropSystem::get_crop_info(int crop_id) {
    for (const auto& info : CROPS) {
        if (info.id == crop_id) return &info;
    }
    return nullptr;
}

const CropInfo* CropSystem::get_crop_info_by_seed(ItemType seed) {
    for (const auto& info : CROPS) {
        if (info.seed == seed) return &info;
    }
    return nullptr;
}

bool CropSystem::plant_crop(Player& player, Tile& tile, int crop_id, std::string& message) {
    if (tile.type != TileType::FARM) {
        message = "농경지에서만 씨앗을 심을 수 있습니다.";
        return false;
    }
    if (tile.has_crop()) {
        message = "이미 작물이 심겨 있습니다.";
        return false;
    }

    const CropInfo* info = get_crop_info(crop_id);
    if (!info) {
        message = "알 수 없는 작물입니다.";
        return false;
    }
    if (player.count_item(info->seed) <= 0) {
        message = std::string(info->name) + " 씨앗이 없습니다.";
        return false;
    }
    if (!player.use_stamina(PLANT_STAMINA_COST)) {
        message = "스태미나가 부족합니다. (필요: " + std::to_string(PLANT_STAMINA_COST) + ")";
        return false;
    }

    player.remove_item(info->seed, 1);
    tile.crop_id = info->id;
    tile.crop_progress = 0;
    tile.crop_max_progress = info->growth_days;
    tile.crop_perennial = info->perennial;
    tile.crop_harvest_cooldown = 0;
    tile.is_occupied = true;

    message = std::string(info->name) + " 씨앗을 심었습니다. " + std::to_string(info->growth_days) + "일 후 수확 가능합니다. 스태미나 -" + std::to_string(PLANT_STAMINA_COST);
    return true;
}

bool CropSystem::harvest_crop(Player& player, Tile& tile, std::string& message) {
    if (!tile.has_crop()) {
        message = "수확할 작물이 없습니다.";
        return false;
    }

    const CropInfo* info = get_crop_info(tile.crop_id);
    if (!info) {
        message = "작물 정보가 손상되었습니다.";
        return false;
    }
    if (tile.crop_progress < tile.crop_max_progress) {
        message = std::string(info->name) + "이(가) 아직 자라는 중입니다. (";
        message += std::to_string(tile.crop_progress) + "/" + std::to_string(tile.crop_max_progress) + "일)";
        return false;
    }
    if (tile.crop_harvest_cooldown > 0) {
        message = std::string(info->name) + "이(가) 재성장 중입니다. (" + std::to_string(tile.crop_harvest_cooldown) + "일 남음)";
        return false;
    }

    int stamina_cost = player.get_harvest_stamina_cost();
    if (!player.use_stamina(stamina_cost)) {
        message = "스태미나가 부족합니다. (필요: " + std::to_string(stamina_cost) + ")";
        return false;
    }

    int yield = info->min_yield + std::rand() % (info->max_yield - info->min_yield + 1);
    if (!player.add_item(info->product, yield)) {
        player.restore_stamina(stamina_cost);
        message = "가방 공간이 부족합니다.";
        return false;
    }

    player.add_exp(info->xp);

    std::ostringstream oss;
    oss << info->name << " " << yield << "개를 수확했습니다. XP +" << info->xp
        << ", 스태미나 -" << stamina_cost;

    if (info->perennial) {
        // 다년생: 진행도 초기화 후 재성장 대기
        tile.crop_progress = 0;
        tile.crop_harvest_cooldown = PERENNIAL_COOLDOWN;
        oss << " (다년생 작물: " << PERENNIAL_COOLDOWN << "일 후 재수확 가능)";
    } else {
        // 일년생: 작물 제거
        tile.crop_id = 0;
        tile.crop_progress = 0;
        tile.crop_max_progress = 0;
        tile.crop_perennial = false;
        tile.crop_harvest_cooldown = 0;
        tile.is_occupied = tile.has_animal() || !tile.building_name.empty();
    }

    message = oss.str();
    return true;
}

void CropSystem::grow_crop(Tile& tile) {
    if (!tile.has_crop()) return;

    if (tile.crop_harvest_cooldown > 0) {
        // 다년생 재성장 대기 중
        --tile.crop_harvest_cooldown;
        return;
    }
    if (tile.crop_progress < tile.crop_max_progress) {
        ++tile.crop_progress;
    }
}

bool CropSystem::water_crop(Player& player, Tile& tile, std::string& message) {
    if (!tile.has_crop()) {
        message = "물을 줄 작물이 없습니다.";
        return false;
    }
    if (tile.watered_today) {
        message = "오늘 이미 물을 주었습니다.";
        return false;
    }
    if (!player.use_stamina(WATER_STAMINA_COST)) {
        message = "스태미나가 부족합니다. (필요: " + std::to_string(WATER_STAMINA_COST) + ")";
        return false;
    }

    tile.watered_today = true;
    message = "작물에 물을 주었습니다. 가뭄으로부터 보호됩니다. 스태미나 -" + std::to_string(WATER_STAMINA_COST);
    return true;
}