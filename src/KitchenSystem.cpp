#include "KitchenSystem.h"
#include "Input.h"
#include "ItemType.h"

#include <iostream>

KitchenSystem::KitchenSystem() : level_(1) {}

void KitchenSystem::interact(Player& player) {
    show_menu(player);
}

void KitchenSystem::show_menu(Player& player) {
    while (true) {
        system("clear");
        std::cout << "🍳 [주방] 현재 주방 레벨: " << level_ << " / " << MAX_LEVEL << "\n";
        std::cout << "보유 재료: 밀 " << player.count_item(ItemType::WHEAT)
                  << ", 고등어 " << player.count_item(ItemType::FISH_MACKEREL)
                  << ", 사과 " << player.count_item(ItemType::APPLE) << "\n";
        std::cout << "골드: " << player.get_money() << "G, 스태미나: " << player.get_stamina() << "\n\n";
        std::cout << "1. 빵 만들기      - 밀 2, 스태미나 10, XP 20\n";
        std::cout << "2. 생선구이 만들기 - 고등어 1, 스태미나 15, XP 30\n";
        std::cout << "3. 사과파이 만들기 - 주방 Lv.2, 밀 2 + 사과 3, 스태미나 20, XP 50\n";
        std::cout << "4. 주방 업그레이드 - 500G, 최대 Lv.3\n";
        std::cout << "0. 나가기\n>> " << std::flush;

        char sel = getch();
        std::string msg;
        bool acted = false;
        if (sel == '0') return;
        if (sel == '1') acted = cook_bread(player, msg);
        else if (sel == '2') acted = cook_grilled_fish(player, msg);
        else if (sel == '3') acted = cook_apple_pie(player, msg);
        else if (sel == '4') acted = upgrade(player, msg);
        else msg = "잘못된 선택입니다.";

        (void)acted;
        std::cout << "\n" << msg << "\n";
        wait_key();
    }
}

bool KitchenSystem::upgrade(Player& player, std::string& message) {
    if (level_ >= MAX_LEVEL) {
        message = "이미 최고 레벨 주방입니다.";
        return false;
    }
    if (!player.spend_money(500)) {
        message = "골드가 부족합니다. (필요: 500G)";
        return false;
    }
    ++level_;
    message = "주방을 업그레이드했습니다. 현재 Lv." + std::to_string(level_);
    return true;
}

bool KitchenSystem::cook_bread(Player& player, std::string& message) {
    if (player.count_item(ItemType::WHEAT) < 2) {
        message = "밀이 부족합니다. (필요: 2개)";
        return false;
    }
    if (!player.use_stamina(10)) {
        message = "스태미나가 부족합니다. (필요: 10)";
        return false;
    }
    player.remove_item(ItemType::WHEAT, 2);
    // player.add_item(ItemType::BREAD, 1);
    player.add_exp(20);
    message = "빵을 만들었습니다. XP +20";
    return true;
}

bool KitchenSystem::cook_grilled_fish(Player& player, std::string& message) {
    if (player.count_item(ItemType::FISH_MACKEREL) < 1) {
        message = "고등어가 부족합니다. (필요: 1개)";
        return false;
    }
    if (!player.use_stamina(15)) {
        message = "스태미나가 부족합니다. (필요: 15)";
        return false;
    }
    player.remove_item(ItemType::FISH_MACKEREL, 1);
    // player.add_item(ItemType::GRILLED_FISH, 1);
    player.add_exp(30);
    message = "생선구이를 만들었습니다. XP +30";
    return true;
}

bool KitchenSystem::cook_apple_pie(Player& player, std::string& message) {
    if (level_ < 2) {
        message = "사과파이는 주방 Lv.2 이상에서 만들 수 있습니다.";
        return false;
    }
    if (player.count_item(ItemType::WHEAT) < 2 || player.count_item(ItemType::APPLE) < 3) {
        message = "재료가 부족합니다. (밀 2개, 사과 3개 필요)";
        return false;
    }
    if (!player.use_stamina(20)) {
        message = "스태미나가 부족합니다. (필요: 20)";
        return false;
    }
    player.remove_item(ItemType::WHEAT, 2);
    player.remove_item(ItemType::APPLE, 3);
    // player.add_item(ItemType::APPLE_PIE, 1);
    player.add_exp(50);
    message = "사과파이를 만들었습니다. XP +50";
    return true;
}
