#include "CropSystem.h"

#include <cstdlib>
#include <sstream>

static const CropInfo CROPS[] = {
    {1, ItemType::SEED_POTATO, ItemType::POTATO, "감자", 1, false, 2, 5, 5},
    {2, ItemType::SEED_CARROT, ItemType::CARROT, "당근", 1, false, 1, 3, 5},
    {3, ItemType::SEED_WHEAT,  ItemType::WHEAT,  "밀",   2, false, 2, 4, 20},
    {4, ItemType::SEED_RICE,   ItemType::RICE,   "쌀",   3, false, 4, 6, 20},
    {5, ItemType::SEED_APPLE,  ItemType::APPLE,  "사과", 5, true,  3, 6, 40}
};

const CropInfo* CropSystem::get_crop_info(int crop_id) {
    for (const auto& crop : CROPS) {
        if (crop.id == crop_id) return &crop;
    }
    return nullptr;
}

const CropInfo* CropSystem::get_crop_info_by_seed(ItemType seed) {
    for (const auto& crop : CROPS) {
        if (crop.seed == seed) return &crop;
    }
    return nullptr;
}

static int required_level_for_crop(int crop_id) {
    if (crop_id == 1 || crop_id == 2) return 1;   // 감자, 당근
    if (crop_id == 3 || crop_id == 4) return 6;   // 밀, 쌀
    if (crop_id == 5) return 16;                  // 사과나무
    return 1;
}

bool CropSystem::plant_crop(Player& player, Tile& tile, int crop_id, std::string& message) {
    if (tile.type != TileType::FARM) {
        message = "여기는 농경지가 아닙니다.";
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
    int required_level = required_level_for_crop(crop_id);
    if (player.get_level() < required_level) {
        message = std::string(info->name) + "은(는) Lv." + std::to_string(required_level) + "부터 심을 수 있습니다.";
        return false;
    }
    if (player.count_item(info->seed) <= 0) {
        message = std::string(info->name) + " 씨앗/묘목이 없습니다.";
        return false;
    }
    int stamina_cost = player.get_harvest_stamina_cost();
    if (!player.use_stamina(stamina_cost)) {
        message = "스태미나가 부족합니다. (필요: " + std::to_string(stamina_cost) + ")";
        return false;
    }

    player.remove_item(info->seed, 1);
    tile.is_occupied = true;
    tile.crop_id = info->id;
    tile.crop_progress = 0;
    tile.crop_max_progress = info->growth_days;
    tile.crop_perennial = info->perennial;
    tile.crop_harvest_cooldown = 0;
    tile.watered_today = true;

    message = std::string(info->name) + "을(를) 심었습니다. (스태미나 -" + std::to_string(stamina_cost) + ")";
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
        message = "아직 작물이 충분히 자라지 않았습니다. 🌱";
        return false;
    }
    if (info->perennial && tile.crop_harvest_cooldown > 0) {
        message = "사과나무가 다음 열매를 맺는 중입니다.";
        return false;
    }

    int stamina_cost = player.get_harvest_stamina_cost();
    if (!player.use_stamina(stamina_cost)) {
        message = "수확할 스태미나가 부족합니다. (필요: " + std::to_string(stamina_cost) + ")";
        return false;
    }

    int amount = info->min_yield + (std::rand() % (info->max_yield - info->min_yield + 1));
    if (!player.add_item(info->product, amount)) {
        message = "가방 공간이 부족하여 수확하지 못했습니다.";
        return false;
    }
    player.add_exp(info->xp);

    std::ostringstream oss;
    oss << info->name << " " << amount << "개를 수확했습니다. XP +" << info->xp
        << ", 스태미나 -" << stamina_cost;

    if (info->perennial) {
        tile.crop_harvest_cooldown = 2;
        tile.crop_progress = tile.crop_max_progress;
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
    const CropInfo* info = get_crop_info(tile.crop_id);
    if (!info) return;

    if (tile.crop_progress < tile.crop_max_progress) {
        ++tile.crop_progress;
        return;
    }

    if (info->perennial && tile.crop_harvest_cooldown > 0) {
        --tile.crop_harvest_cooldown;
    }
}

bool CropSystem::water_crop(Player& player, Tile& tile, std::string& message) {
    if (!tile.has_crop()) {
        message = "물을 줄 작물이 없습니다.";
        return false;
    }
    if (tile.watered_today) {
        message = "이미 오늘 물을 주었습니다.";
        return false;
    }
    if (!player.use_stamina(20)) {
        message = "물을 줄 스태미나가 부족합니다. (필요: 20)";
        return false;
    }
    tile.watered_today = true;
    message = "가뭄에 대비해 작물에 물을 주었습니다. (스태미나 -20)";
    return true;
}
