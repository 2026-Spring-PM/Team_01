#include "KitchenSystem.h"

#include "Input.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace {
constexpr int KITCHEN_UPGRADE_COST[] = {0, 300, 600, 1000};
constexpr int KITCHEN_UPGRADE_LEVEL[] = {1, 6, 10, 16};

const RecipeInfo RECIPES[] = {
    {1, "감자 수프", ItemType::FOOD_POTATO_SOUP, 30, {ItemType::POTATO, ItemType::POTATO}},
    {1, "당근 달걀 롤", ItemType::FOOD_CARROT_EGG_ROLL, 75, {ItemType::CARROT, ItemType::EGG, ItemType::CARROT}},
    {1, "고등어 당근 조림", ItemType::FOOD_MACKEREL_CARROT, 70, {ItemType::FISH_MACKEREL, ItemType::CARROT, ItemType::CARROT}},
    {2, "돼지고기 감자 짜글이", ItemType::FOOD_PORK_POTATO, 390, {ItemType::PORK, ItemType::POTATO, ItemType::POTATO}},
    {2, "감자 밀 부침개", ItemType::FOOD_POTATO_WHEAT_PANCAKE, 130, {ItemType::WHEAT, ItemType::POTATO, ItemType::EGG, ItemType::WHEAT}},
    {2, "당근 광어 스테이크", ItemType::FOOD_CARROT_FLATFISH, 210, {ItemType::FISH_FLATFISH, ItemType::CARROT, ItemType::CARROT}},
    {3, "3색 당근 쌀밥", ItemType::FOOD_3COLOR_CARROT_RICE, 220, {ItemType::RICE, ItemType::CARROT, ItemType::EGG, ItemType::RICE}},
    {3, "감자 매쉬드 비프", ItemType::FOOD_MASHED_BEEF, 540, {ItemType::MILK, ItemType::PORK, ItemType::POTATO, ItemType::POTATO}},
    {3, "당근 사과 주스 만찬", ItemType::FOOD_CARROT_APPLE_JUICE, 320, {ItemType::APPLE, ItemType::CARROT, ItemType::MILK, ItemType::APPLE}},
    {4, "참치 감자 스테이크", ItemType::FOOD_TUNA_POTATO, 1050, {ItemType::FISH_TUNA, ItemType::POTATO, ItemType::CARROT, ItemType::POTATO}},
    {4, "특선 하이라이스", ItemType::FOOD_HAILIGHT_RICE, 760, {ItemType::RICE, ItemType::PORK, ItemType::CARROT, ItemType::MILK, ItemType::RICE, ItemType::CARROT}},
    {4, "사과 감자 무스 케이크", ItemType::FOOD_APPLE_POTATO_CAKE, 460, {ItemType::APPLE, ItemType::WHEAT, ItemType::POTATO, ItemType::MILK, ItemType::APPLE, ItemType::WHEAT, ItemType::POTATO}},
};

constexpr int RECIPE_COUNT = static_cast<int>(sizeof(RECIPES) / sizeof(RECIPES[0]));

bool has_ingredients(const Player& player, const std::vector<ItemType>& ingredients) {
    for (ItemType item : ingredients) {
        int needed = static_cast<int>(std::count(ingredients.begin(), ingredients.end(), item));
        if (player.count_item(item) < needed) return false;
    }
    return true;
}

std::string ingredient_text(const std::vector<ItemType>& ingredients) {
    std::ostringstream oss;
    for (std::size_t i = 0; i < ingredients.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << item_name(ingredients[i]);
    }
    return oss.str();
}

int ingredient_rarity(ItemType item) {
    switch (item) {
        case ItemType::POTATO:
        case ItemType::CARROT:
        case ItemType::EGG:
        case ItemType::FISH_MACKEREL:
            return 1;
        case ItemType::WHEAT:
        case ItemType::RICE:
        case ItemType::PORK:
        case ItemType::FISH_FLATFISH:
            return 2;
        case ItemType::APPLE:
        case ItemType::MILK:
        case ItemType::FISH_TUNA:
            return 3;
        default:
            return 0;
    }
}

ItemType monster_food_for_sum(int rarity_sum) {
    if (rarity_sum <= 5) return ItemType::MONSTER_FOOD_LOW;
    if (rarity_sum <= 10) return ItemType::MONSTER_FOOD_MID;
    return ItemType::MONSTER_FOOD_HIGH;
}

bool consume_ingredients(Player& player, const std::vector<ItemType>& ingredients, bool allow_luk_save, std::string* saved_text) {
    int saved_index = -1;
    if (allow_luk_save && player.should_save_cooking_ingredient() && !ingredients.empty()) {
        saved_index = std::rand() % static_cast<int>(ingredients.size());
    }
    for (std::size_t i = 0; i < ingredients.size(); ++i) {
        if (static_cast<int>(i) == saved_index) continue;
        player.remove_item(ingredients[i], 1);
    }
    if (saved_index >= 0 && saved_text) {
        *saved_text = item_name(ingredients[saved_index]);
    }
    return saved_index >= 0;
}
}

KitchenSystem::KitchenSystem() : level_(1) {}

int KitchenSystem::recipe_count() {
    return RECIPE_COUNT;
}

void KitchenSystem::interact(Player& player) {
    show_menu(player);
}

void KitchenSystem::show_menu(Player& player) {
    while (true) {
        system("clear");
        std::cout << "[주방] Lv." << level_ << " / " << MAX_LEVEL << "\n";
        for (int i = 0; i < RECIPE_COUNT; ++i) {
            const auto& r = RECIPES[i];
            std::cout << (i + 1) << ". " << r.name << " (Lv." << r.kitchen_level << ") - "
                      << ingredient_text(r.ingredients) << "\n";
        }
        std::cout << "13. 주방 업그레이드\n14. 하급 몬스터 푸드 샘플\n15. 중급 몬스터 푸드 샘플\n16. 고급 몬스터 푸드 샘플\n0. 뒤로\n>> " << std::flush;

        int sel = read_int_safe(-1);
        if (sel == 0) return;
        std::string msg;
        perform_action(player, sel, msg);
        std::cout << "\n" << msg << "\n";
        wait_key();
    }
}

bool KitchenSystem::cook(Player& player, std::vector<ItemType> inputs, std::string& message) {
    std::sort(inputs.begin(), inputs.end());
    for (const auto& recipe : RECIPES) {
        auto recipe_inputs = recipe.ingredients;
        std::sort(recipe_inputs.begin(), recipe_inputs.end());
        if (inputs == recipe_inputs) {
            if (level_ < recipe.kitchen_level) {
                message = recipe.name + "은(는) 주방 Lv." + std::to_string(recipe.kitchen_level) + "이 필요합니다.";
                return false;
            }
            if (!has_ingredients(player, recipe.ingredients)) {
                message = "재료가 부족합니다: " + ingredient_text(recipe.ingredients);
                return false;
            }
            std::string saved;
            bool saved_one = consume_ingredients(player, recipe.ingredients, true, &saved);
            player.add_item(recipe.result, 1);
            player.add_exp(25 * recipe.kitchen_level);
            message = recipe.name + " 요리 완료.";
            if (saved_one) message += " LUK 효과로 재료 1개를 아꼈습니다: " + saved + ".";
            return true;
        }
    }

    if (inputs.size() < 2 || inputs.size() > 6) {
        message = "몬스터 푸드는 재료 2~6개가 필요합니다.";
        return false;
    }

    int rarity_sum = 0;
    for (ItemType item : inputs) {
        int rarity = ingredient_rarity(item);
        if (rarity <= 0) {
            message = "작물, 가축 생산물, 생선만 몬스터 푸드 재료로 사용할 수 있습니다.";
            return false;
        }
        rarity_sum += rarity;
    }
    if (!has_ingredients(player, inputs)) {
        message = "몬스터 푸드 재료가 부족합니다: " + ingredient_text(inputs);
        return false;
    }

    consume_ingredients(player, inputs, false, nullptr);
    ItemType result = monster_food_for_sum(rarity_sum);
    player.add_item(result, 1);
    player.add_exp(20 + rarity_sum * 2);
    message = "알 수 없는 레시피에서 " + item_name(result) + " 생성. 희귀도 합 "
            + std::to_string(rarity_sum) + ".";
    return true;
}

bool KitchenSystem::upgrade(Player& player, std::string& message) {
    if (level_ >= MAX_LEVEL) {
        message = "주방이 이미 최고 레벨입니다.";
        return false;
    }

    int target = level_ + 1;
    int base_cost = KITCHEN_UPGRADE_COST[target - 1];
    int cost = player.discounted_upgrade_cost(base_cost);
    int required_level = KITCHEN_UPGRADE_LEVEL[target - 1];
    if (player.get_level() < required_level) {
        message = "플레이어 Lv." + std::to_string(required_level) + "이 필요합니다.";
        return false;
    }
    if (!player.spend_money(cost)) {
        message = "골드가 부족합니다. 필요: " + std::to_string(cost) + "G";
        return false;
    }

    level_ = target;
    player.add_exp(60 * level_);
    message = "주방을 Lv." + std::to_string(level_) + "로 업그레이드했습니다. 비용 "
            + std::to_string(cost) + "G.";
    if (cost < base_cost) message += " INT 할인 적용.";
    return true;
}

std::string KitchenSystem::get_recipe_name(std::vector<ItemType> inputs) {
    std::sort(inputs.begin(), inputs.end());
    for (const auto& recipe : RECIPES) {
        auto recipe_inputs = recipe.ingredients;
        std::sort(recipe_inputs.begin(), recipe_inputs.end());
        if (inputs == recipe_inputs) return recipe.name;
    }
    return "";
}

void KitchenSystem::perform_action(Player& player, int sel, std::string& message) {
    if (sel == RECIPE_COUNT + 1 || sel == 13) {
        upgrade(player, message);
        return;
    }
    if (sel >= 1 && sel <= RECIPE_COUNT) {
        cook(player, RECIPES[sel - 1].ingredients, message);
        return;
    }
    if (sel == 14) {
        cook(player, {ItemType::POTATO, ItemType::CARROT}, message);
        return;
    }
    if (sel == 15) {
        cook(player, {ItemType::WHEAT, ItemType::RICE, ItemType::EGG}, message);
        return;
    }
    if (sel == 16) {
        cook(player, {ItemType::APPLE, ItemType::MILK, ItemType::FISH_TUNA, ItemType::PORK, ItemType::RICE}, message);
        return;
    }
    message = "올바르지 않은 주방 행동입니다.";
}
