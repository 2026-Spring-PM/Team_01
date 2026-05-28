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

    // 요리
    BREAD,
    GRILLED_FISH,
    APPLE_PIE
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

        case ItemType::BREAD:          return "🥖 빵";
        case ItemType::GRILLED_FISH:   return "🍽️ 생선구이";
        case ItemType::APPLE_PIE:      return "🥧 사과파이";
    }
    return "알 수 없는 아이템";
}
