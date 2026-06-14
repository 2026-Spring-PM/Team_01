#include "CropSystem.h"

#include <cstdlib>
#include <sstream>

static const CropInfo CROPS[] = {
    {1, ItemType::SEED_POTATO, ItemType::POTATO, "감자", 1, false, 2, 4, 30},
    {2, ItemType::SEED_CARROT, ItemType::CARROT, "당근", 2, false, 2, 4, 40},
    {3, ItemType::SEED_WHEAT, ItemType::WHEAT, "밀", 3, false, 3, 5, 50},
    {4, ItemType::SEED_RICE, ItemType::RICE, "쌀", 4, false, 2, 4, 60},
    {5, ItemType::SEED_APPLE, ItemType::APPLE, "사과나무", 5, true, 1, 3, 80},
};

static constexpr int PERENNIAL_COOLDOWN = 3;
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
        message = "씨앗은 밭 타일에만 심을 수 있습니다.";
        return false;
    }
    if (tile.has_crop()) {
        message = "이미 작물이 심어져 있습니다.";
        return false;
    }

    const CropInfo* info = get_crop_info(crop_id);
    if (!info) {
        message = "알 수 없는 작물입니다.";
        return false;
    }
    if (player.count_item(info->seed) <= 0) {
        message = item_name(info->seed) + "이(가) 없습니다.";
        return false;
    }
    if (!player.use_stamina(PLANT_STAMINA_COST)) {
        message = "스태미나가 부족합니다. 필요: " + std::to_string(PLANT_STAMINA_COST);
        return false;
    }

    player.remove_item(info->seed, 1);
    tile.crop_id = info->id;
    tile.crop_progress = 0;
    tile.crop_max_progress = info->growth_days;
    tile.crop_perennial = info->perennial;
    tile.crop_harvest_cooldown = 0;
    tile.is_occupied = true;

    message = std::string(info->name) + "을(를) 심었습니다. "
            + std::to_string(info->growth_days) + "일 후 수확할 수 있습니다. 스태미나 -"
            + std::to_string(PLANT_STAMINA_COST);
    return true;
}

bool CropSystem::harvest_crop(Player& player, Tile& tile, std::string& message) {
    if (!tile.has_crop()) {
        message = "수확할 작물이 없습니다.";
        return false;
    }

    const CropInfo* info = get_crop_info(tile.crop_id);
    if (!info) {
        message = "작물 정보가 올바르지 않습니다.";
        return false;
    }
    if (tile.crop_progress < tile.crop_max_progress) {
        message = std::string(info->name) + "은(는) 아직 자라는 중입니다. ("
                + std::to_string(tile.crop_progress) + "/"
                + std::to_string(tile.crop_max_progress) + ")";
        return false;
    }
    if (tile.crop_harvest_cooldown > 0) {
        message = std::string(info->name) + "은(는) 다시 열매를 맺는 중입니다. 남은 일수: "
                + std::to_string(tile.crop_harvest_cooldown);
        return false;
    }

    int stamina_cost = player.get_harvest_stamina_cost();
    bool free_harvest = player.should_skip_harvest_stamina();
    if (!free_harvest && !player.use_stamina(stamina_cost)) {
        message = "스태미나가 부족합니다. 필요: " + std::to_string(stamina_cost);
        return false;
    }

    int yield = info->min_yield + std::rand() % (info->max_yield - info->min_yield + 1);
    if (!player.add_item(info->product, yield)) {
        if (!free_harvest) player.restore_stamina(stamina_cost);
        message = "인벤토리 공간이 부족합니다.";
        return false;
    }

    bool refunded_seed = player.should_refund_seed();
    if (refunded_seed) player.add_item(info->seed, 1);

    player.add_exp(info->xp);

    std::ostringstream oss;
    oss << info->name << " 수확 x" << yield << ". XP +" << info->xp
        << ", 스태미나 -" << (free_harvest ? 0 : stamina_cost);
    if (free_harvest) oss << " (대지의 아들)";
    if (refunded_seed) oss << " (씨앗 환급)";

    if (info->perennial) {
        tile.crop_progress = 0;
        tile.crop_harvest_cooldown = PERENNIAL_COOLDOWN;
        oss << " " << PERENNIAL_COOLDOWN << "일 뒤 다시 수확할 수 있습니다.";
    } else {
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
        message = "오늘 이미 물을 준 작물입니다.";
        return false;
    }
    if (!player.use_stamina(WATER_STAMINA_COST)) {
        message = "스태미나가 부족합니다. 필요: " + std::to_string(WATER_STAMINA_COST);
        return false;
    }

    tile.watered_today = true;
    message = "물을 주었습니다. 오늘 가뭄 피해를 막습니다. 스태미나 -"
            + std::to_string(WATER_STAMINA_COST);
    return true;
}
