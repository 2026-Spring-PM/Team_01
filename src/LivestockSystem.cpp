#include "LivestockSystem.h"

#include <cstdlib>
#include <sstream>

static constexpr int LIVESTOCK_ACTION_STAMINA = 10;

static const LivestockInfo LIVESTOCKS[] = {
    {1, ItemType::BABY_CHICKEN, ItemType::EGG,  "닭", 3, 1, 10, 10, false},
    {2, ItemType::BABY_PIG,     ItemType::PORK, "돼지", 5, 2, 20, 30, true},
    {3, ItemType::BABY_COW,     ItemType::MILK, "소", 4, 1, 15, 20, false}
};

const LivestockInfo* LivestockSystem::get_info(int animal_type) {
    for (const auto& info : LIVESTOCKS) {
        if (info.type == animal_type) return &info;
    }
    return nullptr;
}

static int required_level_for_animal(int animal_type) {
    if (animal_type == 1) return 1;   // 닭
    if (animal_type == 2) return 6;   // 돼지
    if (animal_type == 3) return 16;  // 소
    return 1;
}

bool LivestockSystem::place_animal(Player& player, Tile& tile, int animal_type, std::string& message) {
    if (tile.type != TileType::LIVESTOCK) {
        message = "여기는 사육장이 아닙니다.";
        return false;
    }
    if (tile.has_animal()) {
        message = "이미 가축이 있습니다.";
        return false;
    }

    const LivestockInfo* info = get_info(animal_type);
    if (!info) {
        message = "알 수 없는 가축입니다.";
        return false;
    }
    int required_level = required_level_for_animal(animal_type);
    if (player.get_level() < required_level) {
        message = std::string(info->name) + "은(는) Lv." + std::to_string(required_level) + "부터 사육할 수 있습니다.";
        return false;
    }
    if (player.count_item(info->baby_item) <= 0) {
        message = std::string(item_name(info->baby_item)) + "을(를) 보유하고 있지 않습니다.";
        return false;
    }
    if (!player.use_stamina(LIVESTOCK_ACTION_STAMINA)) {
        message = "스태미나가 부족합니다. (필요: " + std::to_string(LIVESTOCK_ACTION_STAMINA) + ")";
        return false;
    }

    player.remove_item(info->baby_item, 1);
    tile.is_occupied = true;
    tile.animal_placed = true;
    tile.animal_type = animal_type;
    tile.animal_age = 0;
    tile.animal_adult_age = info->adult_age;
    tile.animal_produced_today = false;

    message = std::string("새끼 ") + info->name + "을(를) 사육장에 배치했습니다. 스태미나 -" + std::to_string(LIVESTOCK_ACTION_STAMINA);
    return true;
}

bool LivestockSystem::collect_product(Player& player, Tile& tile, std::string& message) {
    if (!tile.has_animal()) {
        message = "가축이 없습니다.";
        return false;
    }

    const LivestockInfo* info = get_info(tile.animal_type);
    if (!info) {
        message = "가축 정보가 손상되었습니다.";
        return false;
    }
    if (tile.animal_age < info->adult_age) {
        message = "아직 아기 상태라 생산물을 얻을 수 없습니다.";
        return false;
    }
    if (tile.animal_produced_today) {
        message = "이 가축은 오늘 이미 생산을 마쳤습니다.";
        return false;
    }
    if (!player.use_stamina(LIVESTOCK_ACTION_STAMINA)) {
        message = "스태미나가 부족합니다. (필요: " + std::to_string(LIVESTOCK_ACTION_STAMINA) + ")";
        return false;
    }
    if (!player.add_item(info->product_item, info->product_amount)) {
        player.restore_stamina(LIVESTOCK_ACTION_STAMINA);
        message = "가방 공간이 부족합니다.";
        return false;
    }

    player.add_exp(info->xp);
    tile.animal_produced_today = true;

    std::ostringstream oss;
    oss << info->name << "에게서 " << item_name(info->product_item) << " " << info->product_amount
        << "개를 얻었습니다. XP +" << info->xp << ", 스태미나 -" << LIVESTOCK_ACTION_STAMINA;

    if (info->remove_after_product) {
        tile.animal_placed = false;
        tile.animal_type = 0;
        tile.animal_age = 0;
        tile.animal_adult_age = 0;
        tile.animal_produced_today = false;
        tile.is_occupied = tile.has_crop() || !tile.building_name.empty();
        oss << " 돼지는 생산 후 사육장에서 제거되었습니다.";
    }

    message = oss.str();
    return true;
}

bool LivestockSystem::remove_animal(Tile& tile, std::string& message) {
    if (!tile.has_animal()) {
        message = "제거할 가축이 없습니다.";
        return false;
    }
    tile.animal_placed = false;
    tile.animal_type = 0;
    tile.animal_age = 0;
    tile.animal_adult_age = 0;
    tile.animal_produced_today = false;
    tile.is_occupied = tile.has_crop() || !tile.building_name.empty();
    message = "가축을 사육장에서 제거했습니다.";
    return true;
}

void LivestockSystem::grow_animal(Tile& tile) {
    if (!tile.has_animal()) return;
    ++tile.animal_age;
    tile.animal_produced_today = false;
}

bool LivestockSystem::hatch_egg(Player& player, std::string& message) {
    if (player.count_item(ItemType::EGG) <= 0) {
        message = "부화시킬 달걀이 없습니다.";
        return false;
    }
    if (!player.use_stamina(LIVESTOCK_ACTION_STAMINA)) {
        message = "스태미나가 부족합니다. (필요: " + std::to_string(LIVESTOCK_ACTION_STAMINA) + ")";
        return false;
    }
    player.remove_item(ItemType::EGG, 1);
    int dice = std::rand() % 100;
    if (dice < 35) {
        player.add_item(ItemType::BABY_CHICKEN, 1);
        message = "부화 성공! 새끼 닭을 얻었습니다. 스태미나 -" + std::to_string(LIVESTOCK_ACTION_STAMINA);
        return true;
    }
    message = "부화에 실패했습니다. 달걀은 사라졌습니다. 스태미나 -" + std::to_string(LIVESTOCK_ACTION_STAMINA);
    return false;
}
