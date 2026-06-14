#pragma once

#include <string>

enum class ItemType {
    SEED_POTATO,
    SEED_CARROT,
    SEED_WHEAT,
    SEED_RICE,
    SEED_APPLE,

    POTATO,
    CARROT,
    WHEAT,
    RICE,
    APPLE,

    BABY_CHICKEN,
    BABY_PIG,
    BABY_COW,

    EGG,
    MILK,
    PORK,

    WOOD,
    COPPER,
    IRON,
    GOLD,

    FISH_MACKEREL,
    FISH_FLATFISH,
    FISH_TUNA,

    HOE_COPPER,
    HOE_IRON,
    HOE_GOLD,
    ROD_COPPER,
    ROD_IRON,
    ROD_GOLD,
    PICK_COPPER,
    PICK_IRON,
    PICK_GOLD,

    FOOD_POTATO_SOUP,
    FOOD_CARROT_EGG_ROLL,
    FOOD_MACKEREL_CARROT,
    FOOD_PORK_POTATO,
    FOOD_POTATO_WHEAT_PANCAKE,
    FOOD_CARROT_FLATFISH,
    FOOD_3COLOR_CARROT_RICE,
    FOOD_MASHED_BEEF,
    FOOD_CARROT_APPLE_JUICE,
    FOOD_TUNA_POTATO,
    FOOD_HAILIGHT_RICE,
    FOOD_APPLE_POTATO_CAKE,

    MONSTER_FOOD_LOW,
    MONSTER_FOOD_MID,
    MONSTER_FOOD_HIGH,
};

inline std::string item_name(ItemType type) {
    switch (type) {
        case ItemType::SEED_POTATO: return "감자 씨앗";
        case ItemType::SEED_CARROT: return "당근 씨앗";
        case ItemType::SEED_WHEAT: return "밀 씨앗";
        case ItemType::SEED_RICE: return "쌀 씨앗";
        case ItemType::SEED_APPLE: return "사과 묘목";
        case ItemType::POTATO: return "감자";
        case ItemType::CARROT: return "당근";
        case ItemType::WHEAT: return "밀";
        case ItemType::RICE: return "쌀";
        case ItemType::APPLE: return "사과";
        case ItemType::BABY_CHICKEN: return "병아리";
        case ItemType::BABY_PIG: return "새끼 돼지";
        case ItemType::BABY_COW: return "송아지";
        case ItemType::EGG: return "달걀";
        case ItemType::MILK: return "우유";
        case ItemType::PORK: return "돼지고기";
        case ItemType::WOOD: return "나무";
        case ItemType::COPPER: return "구리 광석";
        case ItemType::IRON: return "철 광석";
        case ItemType::GOLD: return "금 광석";
        case ItemType::FISH_MACKEREL: return "고등어";
        case ItemType::FISH_FLATFISH: return "광어";
        case ItemType::FISH_TUNA: return "참치";
        case ItemType::HOE_COPPER: return "구리 괭이";
        case ItemType::HOE_IRON: return "철 괭이";
        case ItemType::HOE_GOLD: return "금 괭이";
        case ItemType::ROD_COPPER: return "구리 낚싯대";
        case ItemType::ROD_IRON: return "철 낚싯대";
        case ItemType::ROD_GOLD: return "금 낚싯대";
        case ItemType::PICK_COPPER: return "구리 곡괭이";
        case ItemType::PICK_IRON: return "철 곡괭이";
        case ItemType::PICK_GOLD: return "금 곡괭이";
        case ItemType::FOOD_POTATO_SOUP: return "감자 수프";
        case ItemType::FOOD_CARROT_EGG_ROLL: return "당근 달걀 롤";
        case ItemType::FOOD_MACKEREL_CARROT: return "고등어 당근 조림";
        case ItemType::FOOD_PORK_POTATO: return "돼지고기 감자 짜글이";
        case ItemType::FOOD_POTATO_WHEAT_PANCAKE: return "감자 밀 부침개";
        case ItemType::FOOD_CARROT_FLATFISH: return "당근 광어 스테이크";
        case ItemType::FOOD_3COLOR_CARROT_RICE: return "3색 당근 쌀밥";
        case ItemType::FOOD_MASHED_BEEF: return "감자 매쉬드 비프";
        case ItemType::FOOD_CARROT_APPLE_JUICE: return "당근 사과 주스 만찬";
        case ItemType::FOOD_TUNA_POTATO: return "참치 감자 스테이크";
        case ItemType::FOOD_HAILIGHT_RICE: return "특선 하이라이스";
        case ItemType::FOOD_APPLE_POTATO_CAKE: return "사과 감자 무스 케이크";
        case ItemType::MONSTER_FOOD_LOW: return "하급 몬스터 푸드";
        case ItemType::MONSTER_FOOD_MID: return "중급 몬스터 푸드";
        case ItemType::MONSTER_FOOD_HIGH: return "고급 몬스터 푸드";
    }
    return "알 수 없는 아이템";
}
