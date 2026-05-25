#pragma once

#include <string>

// 게임 내 아이템 종류
// 작물, 가축 산물, 자재, 요리 등 모든 아이템 통합
enum class ItemType {
    // 씨앗 (피에르 잡화상에서 구매)
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

    // 가축 산물
    EGG,
    MILK,
    PORK,

    // 자재
    WOOD,
    COPPER,
    IRON,
    GOLD,

    // 요리
    BREAD,
    FISH_KEBAB,
    MEAT_PIE,
    CAKE,
    APPLE_PIE,

    // 생선
    FISH_MACKEREL,
    FISH_FLATFISH,
    FISH_TUNA,
};

// 아이템 한글 이름 (화면 출력용)
inline std::string item_name(ItemType type) {
    switch (type) {
        case ItemType::SEED_POTATO: return "감자 씨앗";
        case ItemType::SEED_CARROT: return "당근 씨앗";
        case ItemType::SEED_WHEAT:  return "밀 씨앗";
        case ItemType::SEED_RICE:   return "쌀 씨앗";
        case ItemType::SEED_APPLE:  return "사과 묘목";
        case ItemType::POTATO:      return "🥔 감자";
        case ItemType::CARROT:      return "🥕 당근";
        case ItemType::WHEAT:       return "🌾 밀";
        case ItemType::RICE:        return "🍚 쌀";
        case ItemType::APPLE:       return "🍎 사과";
        case ItemType::EGG:         return "🥚 달걀";
        case ItemType::MILK:        return "🥛 우유";
        case ItemType::PORK:        return "🥩 돼지고기";
        case ItemType::WOOD:        return "🪵 나무";
        case ItemType::COPPER:      return "🥉 구리";
        case ItemType::IRON:        return "🥈 철";
        case ItemType::GOLD:        return "🥇 금";
        case ItemType::BREAD:       return "🥖 빵";
        case ItemType::FISH_KEBAB:  return "🍢 생선케밥";
        case ItemType::MEAT_PIE:    return "🥩 미트파이";
        case ItemType::CAKE:        return "🎂 케이크";
        case ItemType::APPLE_PIE:   return "🥧 사과파이";
        case ItemType::FISH_MACKEREL: return "🐟 고등어";
        case ItemType::FISH_FLATFISH: return "🐠 광어";
        case ItemType::FISH_TUNA:     return "🐡 참치";
    }
    return "???";
}