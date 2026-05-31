#pragma once

#include <string>

// 게임 내 아이템 종류
// 작물, 씨앗, 새끼 가축, 축산물, 자재, 생선, 요리를 모두 통합한다.
enum class ItemType {
    // 씨앗 / 묘목
    SEED_POTATO,
    SEED_CARROT,
    SEED_WHEAT,
    SEED_RICE,
    SEED_APPLE,

    // 수확물
    POTATO,
    CARROT,
    WHEAT,
    RICE,
    APPLE,

    // 새끼 가축
    BABY_CHICKEN,
    BABY_PIG,
    BABY_COW,

    // 축산물
    EGG,
    MILK,
    PORK,

    // 자재 / 광석
    WOOD,
    COPPER,
    IRON,
    GOLD,

    // 생선
    FISH_MACKEREL,
    FISH_FLATFISH,
    FISH_TUNA,

    // 보물상자 (새로 추가)
    TREASURE_BOX_BASIC,
    TREASURE_BOX_NORMAL,
    TREASURE_BOX_ADVANCED,

    // 요리 (기존 빵 등 삭제, 1~4단계 요리 12종 추가)
    FOOD_POTATO_SOUP, FOOD_CARROT_EGG_ROLL, FOOD_MACKEREL_STEW,
    FOOD_PORK_STEW, FOOD_POTATO_PANCAKE, FOOD_FLATFISH_STEAK,
    FOOD_CARROT_RICE, FOOD_BEEF_STEAK, FOOD_APPLE_JUICE,
    FOOD_TUNA_STEAK, FOOD_HAYASHI_RICE, FOOD_APPLE_MOUSSE
};

inline std::string item_name(ItemType type) {
    switch (type) {
        case ItemType::SEED_POTATO:    return "감자 씨앗";
        case ItemType::SEED_CARROT:    return "당근 씨앗";
        case ItemType::SEED_WHEAT:     return "밀 씨앗";
        case ItemType::SEED_RICE:      return "쌀 씨앗";
        case ItemType::SEED_APPLE:     return "사과 묘목";

        case ItemType::POTATO:         return "🥔 감자";
        case ItemType::CARROT:         return "🥕 당근";
        case ItemType::WHEAT:          return "🌾 밀";
        case ItemType::RICE:           return "🍚 쌀";
        case ItemType::APPLE:          return "🍎 사과";

        case ItemType::BABY_CHICKEN:   return "🐣 새끼 닭";
        case ItemType::BABY_PIG:       return "🐷 새끼 돼지";
        case ItemType::BABY_COW:       return "🐂 새끼 소";

        case ItemType::EGG:            return "🥚 달걀";
        case ItemType::MILK:           return "🥛 우유";
        case ItemType::PORK:           return "🥩 돼지고기";

        case ItemType::WOOD:           return "🪵 나무";
        case ItemType::COPPER:         return "🥉 구리 광석";
        case ItemType::IRON:           return "🥈 철 광석";
        case ItemType::GOLD:           return "🥇 금 광석";

        case ItemType::FISH_MACKEREL:  return "🐟 고등어";
        case ItemType::FISH_FLATFISH:  return "🐠 광어";
        case ItemType::FISH_TUNA:      return "🐋 참치";

        case ItemType::TREASURE_BOX_BASIC:    return "📦 초급 보물상자";
        case ItemType::TREASURE_BOX_NORMAL:   return "📦 중급 보물상자";
        case ItemType::TREASURE_BOX_ADVANCED: return "📦 고급 보물상자";

        case ItemType::FOOD_POTATO_SOUP:      return "🍲 감자 수프";
        case ItemType::FOOD_CARROT_EGG_ROLL:  return "🍳 당근 달걀 롤";
        case ItemType::FOOD_MACKEREL_STEW:    return "🍲 고등어 당근 조림";
        case ItemType::FOOD_PORK_STEW:        return "🍲 돼지고기 감자 짜글이";
        case ItemType::FOOD_POTATO_PANCAKE:   return "🥞 감자 밀 부침개";
        case ItemType::FOOD_FLATFISH_STEAK:   return "🍽️ 당근 광어 스테이크";
        case ItemType::FOOD_CARROT_RICE:      return "🍚 3색 당근 쌀밥";
        case ItemType::FOOD_BEEF_STEAK:       return "🥩 매쉬드 비프 스테이크";
        case ItemType::FOOD_APPLE_JUICE:      return "🧃 당근 사과 주스 만찬";
        case ItemType::FOOD_TUNA_STEAK:       return "👑 참치 가니쉬 스테이크";
        case ItemType::FOOD_HAYASHI_RICE:     return "🍛 특선 하이라이스 만찬";
        case ItemType::FOOD_APPLE_MOUSSE:     return "🍰 사과 감자 무스 케이크";
    }
    return "알 수 없는 아이템";
}
