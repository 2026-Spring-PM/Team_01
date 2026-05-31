// [KitchenSystem.cpp]
#include "KitchenSystem.h"
#include "Player.h"  // <--- ★ 이 헤더가 없으면 perform_action 내부에서 에러가 납니다!
#include "Input.h"   // read_int_safe, wait_key 사용을 위해 필요
#include <iostream>
#include <vector>

KitchenSystem::KitchenSystem() : level_(1) {}


// [텍스트 터미널 모드용 상호작용 인터페이스]
void KitchenSystem::interact(Player& player) {
    while (true) {
        system("clear");
        std::cout << "🍳 [주방 - 현재 레벨 " << level_ << "] 스태미나: " << player.get_stamina() << "/" << player.get_stamina_max() << "\n";
        std::cout << "───────────────────────────────────────────────\n";
        std::cout << "[주방 Lv.1]\n";
        std::cout << " 1. 감자 수프 (감자 2) | 판매가: 30G\n";
        std::cout << " 2. 당근 달걀 롤 (당근 2 + 달걀 1) | 판매가: 75G\n";
        std::cout << " 3. 고등어 당근 조림 (고등어 1 + 당근 1) | 판매가: 70G\n\n";
        std::cout << "[주방 Lv.2]\n";
        std::cout << " 4. 돼지고기 감자 짜글이 (돼지 1 + 감자 2) | 판매가: 390G\n";
        std::cout << " 5. 감자 밀 부침개 (밀 2 + 감자 1 + 달걀 1) | 판매가: 130G\n";
        std::cout << " 6. 당근 광어 스테이크 (광어 1 + 당근 2) | 판매가: 210G\n\n";
        std::cout << "[주방 Lv.3]\n";
        std::cout << " 7. 3색 당근 쌀밥 (쌀 2 + 당근 2 + 달걀 1) | 판매가: 220G\n";
        std::cout << " 8. 매쉬드 비프 스테이크 (우유 1 + 돼지 1 + 감자 2) | 판매가: 540G\n";
        std::cout << " 9. 당근 사과 주스 만찬 (사과 2 + 당근 2 + 우유 1) | 판매가: 320G\n\n";
        std::cout << "[주방 Lv.4]\n";
        std::cout << "10. 국왕의 참치 스테이크 (참치 1 + 감자 3 + 당근 1) | 판매가: 1050G\n";
        std::cout << "11. 특선 하이라이스 만찬 (쌀 2 + 돼지 1 + 당근 2 + 우유 1) | 판매가: 760G\n";
        std::cout << "12. 사과 감자 무스 케이크 (사과 2 + 밀 2 + 감자 2 + 우유 1) | 판매가: 460G\n";
        std::cout << "───────────────────────────────────────────────\n";
        std::cout << "13. 주방 업그레이드 (레벨별 골드 및 캐릭터 레벨 필요)\n";
        std::cout << "0. 나가기\n>> " << std::flush;

        int choice = read_int_safe(-1);
        if (choice == 0) return;
        if (choice < 1 || choice > 13) continue;

        std::string message;
        perform_action(player, choice, message);
        std::cout << "\n" << message << "\n";
        wait_key();
    }
}

// [GUI 및 텍스트 공용 핵심 로직]
void KitchenSystem::perform_action(Player& player, int action, std::string& message) {
    // [주방 업그레이드 (action == 13)]
    if (action == 13) {
        if (level_ >= 4) { message = "주방이 이미 최고 레벨(4)입니다."; return; }
        int req_p_lvl = (level_ == 1) ? 10 : (level_ == 2) ? 16 : 20;
        int req_gold = (level_ == 1) ? 1000 : (level_ == 2) ? 2500 : 5000;
        
        if (player.get_level() < req_p_lvl) { message = "레벨이 부족합니다! (필요 Lv." + std::to_string(req_p_lvl) + ")"; return; }
        if (player.get_money() < req_gold) { message = "골드가 부족합니다! (필요 " + std::to_string(req_gold) + "G)"; return; }
        
        player.spend_money(req_gold);
        level_++;
        message = "🎉 주방을 Lv." + std::to_string(level_) + "(으)로 증축했습니다!";
        return;
    }

    // [요리 레시피 테이블]
    struct Recipe {
        int req_level; int stam_cost; ItemType result; std::string name;
        std::vector<std::pair<ItemType, int>> ingredients;
    };

    static const Recipe recipes[] = {
        // 주방 Lv 1 (스태미나 -2)
        {1, 2, ItemType::FOOD_POTATO_SOUP, "감자 수프", {{ItemType::POTATO, 2}}},
        {1, 2, ItemType::FOOD_CARROT_EGG_ROLL, "당근 달걀 롤", {{ItemType::CARROT, 2}, {ItemType::EGG, 1}}},
        {1, 2, ItemType::FOOD_MACKEREL_STEW, "고등어 당근 조림", {{ItemType::FISH_MACKEREL, 1}, {ItemType::CARROT, 1}}},
        // 주방 Lv 2 (스태미나 -2)
        {2, 2, ItemType::FOOD_PORK_STEW, "돼지고기 감자 짜글이", {{ItemType::PORK, 1}, {ItemType::POTATO, 2}}},
        {2, 2, ItemType::FOOD_POTATO_PANCAKE, "감자 밀 부침개", {{ItemType::WHEAT, 2}, {ItemType::POTATO, 1}, {ItemType::EGG, 1}}},
        {2, 2, ItemType::FOOD_FLATFISH_STEAK, "당근 광어 스테이크", {{ItemType::FISH_FLATFISH, 1}, {ItemType::CARROT, 2}}},
        // 주방 Lv 3 (스태미나 -3)
        {3, 3, ItemType::FOOD_CARROT_RICE, "3색 당근 쌀밥", {{ItemType::RICE, 2}, {ItemType::CARROT, 2}, {ItemType::EGG, 1}}},
        {3, 3, ItemType::FOOD_BEEF_STEAK, "매쉬드 비프 스테이크", {{ItemType::MILK, 1}, {ItemType::PORK, 1}, {ItemType::POTATO, 2}}},
        {3, 3, ItemType::FOOD_APPLE_JUICE, "당근 사과 주스 만찬", {{ItemType::APPLE, 2}, {ItemType::CARROT, 2}, {ItemType::MILK, 1}}},
        // 주방 Lv 4 (스태미나 -3)
        {4, 3, ItemType::FOOD_TUNA_STEAK, "국왕의 참치 스테이크", {{ItemType::FISH_TUNA, 1}, {ItemType::POTATO, 3}, {ItemType::CARROT, 1}}},
        {4, 3, ItemType::FOOD_HAYASHI_RICE, "특선 하이라이스 만찬", {{ItemType::RICE, 2}, {ItemType::PORK, 1}, {ItemType::CARROT, 2}, {ItemType::MILK, 1}}},
        {4, 3, ItemType::FOOD_APPLE_MOUSSE, "사과 감자 무스 케이크", {{ItemType::APPLE, 2}, {ItemType::WHEAT, 2}, {ItemType::POTATO, 2}, {ItemType::MILK, 1}}}
    };

    if (action < 1 || action > 12) { message = "잘못된 요리 선택입니다."; return; }
    const Recipe& r = recipes[action - 1];

    if (level_ < r.req_level) { message = "주방 레벨이 낮아 이 요리를 만들 수 없습니다. (현재 Lv." + std::to_string(level_) + " / 필요 Lv." + std::to_string(r.req_level) + ")"; return; }
    if (player.get_stamina() < r.stam_cost) { message = "스태미나가 부족하여 요리할 수 없습니다."; return; }

    // 재료 보유량 체크
    for (const auto& ing : r.ingredients) {
        if (player.count_item(ing.first) < ing.second) {
            message = "재료가 부족합니다! (" + r.name + " 제작 실패)"; return;
        }
    }

    // 스태미나 및 재료 차감, 아이템 지급
    for (const auto& ing : r.ingredients) {
        player.remove_item(ing.first, ing.second);
    }
    player.use_stamina(r.stam_cost);
    player.add_item(r.result, 1);

    message = "🍳 [" + r.name + "] 완성! (스태미나 -" + std::to_string(r.stam_cost) + ")";
}