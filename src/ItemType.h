#pragma once

#include <string>

// 게임 내 아이템 종류
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

    // 농기구 (괭이/낚싯대/곡괭이 각 3등급 = 9개)
    HOE_COPPER,    // 동제 괭이
    HOE_IRON,      // 철제 괭이
    HOE_GOLD,      // 금제 괭이
    ROD_COPPER,    // 동제 낚싯대
    ROD_IRON,      // 철제 낚싯대
    ROD_GOLD,      // 금제 낚싯대
    PICK_COPPER,   // 동제 곡괭이
    PICK_IRON,     // 철제 곡괭이
    PICK_GOLD,     // 금제 곡괭이

    // 요리 (주방 Lv.1~4, 12종)
    FOOD_POTATO_SOUP,           // 감자 수프
    FOOD_CARROT_EGG_ROLL,       // 당근 달걀 롤
    FOOD_MACKEREL_CARROT,       // 고등어 당근 조림
    FOOD_PORK_POTATO,           // 돼지고기 감자 짜글이
    FOOD_POTATO_WHEAT_PANCAKE,  // 감자 밀 부침개
    FOOD_CARROT_FLATFISH,       // 당근 광어 스테이크
    FOOD_3COLOR_CARROT_RICE,    // 3색 당근 쌀밥
    FOOD_MASHED_BEEF,           // 감자 매쉬드 비프 스테이크
    FOOD_CARROT_APPLE_JUICE,    // 당근 사과 주스 만찬
    FOOD_TUNA_POTATO,           // 국왕의 참치 감자 가니쉬 스테이크
    FOOD_HAILIGHT_RICE,         // 대농장 특선 하이라이스 만찬
    FOOD_APPLE_POTATO_CAKE,     // 특선 사과 감자 무스 케이크

    MONSTER_FOOD_LOW,    // 하급 괴식 
    MONSTER_FOOD_MID,    // 중급 괴식 
    MONSTER_FOOD_HIGH,   // 고급 괴식
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

        case ItemType::HOE_COPPER:     return "🔧 동제 괭이";
        case ItemType::HOE_IRON:       return "🔧 철제 괭이";
        case ItemType::HOE_GOLD:       return "🔧 금제 괭이";
        case ItemType::ROD_COPPER:     return "🎣 동제 낚싯대";
        case ItemType::ROD_IRON:       return "🎣 철제 낚싯대";
        case ItemType::ROD_GOLD:       return "🎣 금제 낚싯대";
        case ItemType::PICK_COPPER:    return "⛏️ 동제 곡괭이";
        case ItemType::PICK_IRON:      return "⛏️ 철제 곡괭이";
        case ItemType::PICK_GOLD:      return "⛏️ 금제 곡괭이";

        case ItemType::FOOD_POTATO_SOUP:          return "🥣 감자 수프";
        case ItemType::FOOD_CARROT_EGG_ROLL:      return "🌮 당근 달걀 롤";
        case ItemType::FOOD_MACKEREL_CARROT:      return "🍱 고등어 당근 조림";
        case ItemType::FOOD_PORK_POTATO:          return "🍲 돼지고기 감자 짜글이";
        case ItemType::FOOD_POTATO_WHEAT_PANCAKE: return "🥞 감자 밀 부침개";
        case ItemType::FOOD_CARROT_FLATFISH:      return "🥩 당근 광어 스테이크";
        case ItemType::FOOD_3COLOR_CARROT_RICE:   return "🍙 3색 당근 쌀밥";
        case ItemType::FOOD_MASHED_BEEF:          return "🥩 감자 매쉬드 비프";
        case ItemType::FOOD_CARROT_APPLE_JUICE:   return "🧃 당근 사과 주스 만찬";
        case ItemType::FOOD_TUNA_POTATO:          return "🍽️ 국왕의 참치 감자 스테이크";
        case ItemType::FOOD_HAILIGHT_RICE:        return "🍛 대농장 특선 하이라이스";
        case ItemType::FOOD_APPLE_POTATO_CAKE:    return "🎂 특선 사과 감자 무스 케이크";

        case ItemType::MONSTER_FOOD_LOW:   return "🤢 하급 괴식 (수상한 모방품)";
        case ItemType::MONSTER_FOOD_MID:   return "🤢 중급 괴식 (그럴싸한 위조 만찬)";
        case ItemType::MONSTER_FOOD_HIGH:  return "🤢 고급 괴식 (환상의 미식)";
    }
    return "알 수 없는 아이템";
}
