#include "MarketSystem.h"

#include "Input.h"
#include "Season.h"

#include <cstdlib>
#include <iostream>
#include <sstream>

// 선물 아이템 정의: 주방 Lv.2 → 돼지고기 감자 짜글이(+5), Lv.3 → 감자 매쉬드 비프(+10), Lv.4 → 참치 감자 스테이크(+15)
const GiftItem MarketSystem::GIFT_ITEMS[3] = {
    {ItemType::FOOD_PORK_POTATO,    "돼지고기 감자 짜글이", 2, 5},
    {ItemType::FOOD_MASHED_BEEF,    "감자 매쉬드 비프",     3, 10},
    {ItemType::FOOD_TUNA_POTATO,    "참치 감자 스테이크",   4, 15},
};

namespace {
bool is_monster_food(ItemType item) {
    return item == ItemType::MONSTER_FOOD_LOW ||
           item == ItemType::MONSTER_FOOD_MID ||
           item == ItemType::MONSTER_FOOD_HIGH;
}

// 신용도 구간 반환 (0=추방, 1=1-30, 2=31-70, 3=71-100)
int trust_tier(int trust) {
    if (trust <= 0)  return 0;
    if (trust <= 30) return 1;
    if (trust <= 70) return 2;
    return 3;
}

const char* entry_line(int tier) {
    switch (tier) {
        case 3: return "\"우리 마을 최고의 농부 아닌가! 어서오게나.\"";
        case 2: return "\"왔나. 필요한 물건 있으면 골라보게.\"";
        case 1: return "\"또 너냐? 이 사기꾼 놈, 내 상점에서 얼른 나가!\"";
        default: return "\"더는 못 참아! 당장 내 상점에서 나가! 자네가 가져오는 건 이제 냄새만 맡아도 속이 메스꺼우니, 꼴도 보기 싫으니까 당장 꺼지게!\"";
    }
}

const char* success_line(int tier) {
    switch (tier) {
        case 3: return "\"오, 이게 말로만 듣던 그 귀한 고급 요리란 말이지? 자네가 만든 거니 의심 없이 비싸게 사지! 자, 여기 있네!\"";
        case 2: return "\"음... 저번에 준 음식은 가족들이 먹고 배가 좀 아프다던데... 이번 건 진짜 확실한 거 맞나? ...하긴, 눈빛을 보니 거짓말을 하는 것 같진 않군. 이번 딱 한 번만 믿고 사 주겠네.\"";
        case 1: return "\"자네가 가져온 건 쳐다보기도 싫네만... 윽, 대체 뭘 넣었길래 이렇게 군침 도는 냄새가 나는 거지? ...사기 치는 게 아니길 바라네. 이번 한 번만 속아 넘어가 줄 테니 당장 돈 받고 나가게!\"";
        default: return "";
    }
}

const char* fail_line(int tier) {
    switch (tier) {
        case 3: return "\"음? 자네 실력답지 않게 오늘은 요리 모양새가 왜 이리 고약한가? 아무리 자네라지만 이 상태인 걸 돈 주고 사 가긴 조금 곤란하겠어. 다음엔 제대로 만들어 오게나.\"";
        case 2: return "\"잠깐, 이 냄새는 도저히 사람이 먹을 만한 게 아니잖아! 자네 요즘 나한테 자꾸 이상한 걸 음식이라고 속여서 밀어 넘기려는 것 같은데... 내 눈이 호락호락해 보이나?\"";
        case 1: return "\"어디서 또 이 쓰레기를 음식이라고 속여 팔려고 해?! 경찰에 신고하기 전에 당장 내 눈앞에서 치워! 이제부터 자네가 가져오는 건 다 쓰레기로 볼 줄 알게!\"";
        default: return "";
    }
}

// 선물 대사: trust_was_reduced = 이전에 신용도가 한 번이라도 내려간 적 있는가
const char* gift_line(int trust_restore, bool trust_was_reduced) {
    if (trust_restore == 5) {
        return trust_was_reduced
            ? "\"음... 나한테 준 상처가 있으니 아직 완전히 풀린 건 아니네만, 그래도 이렇게 챙겨주니 자네를 다시 보게 되는구만. 잘 먹겠네. (신용도 소폭 상승)\""
            : "\"어, 흠... 나를 주려고 가져온 건가? 고, 고맙네. 갑자기 이런 걸 주다니... 혹시 나한테 무슨 할 말이라도 있는 건가? 왠지 기분이 묘하게 좋아지는군!\"";
    } else if (trust_restore == 10) {
        return trust_was_reduced
            ? "\"안 그래도 자네를 계속 의심하느라 마음이 복잡했는데, 이 정성을 보니 자네가 확실히 믿음직스러워졌습니다. 고맙게 받겠네. (신용도 크게 상승)\""
            : "\"아니, 냄새부터 장난이 아니구만! 자네, 혹시 매일 내 얼굴 보러 오려고 이렇게 정성을 들이는 건가? 하하, 이 아저씨 인기가 아직 안 죽었군! 기분이 정말 좋아졌다네!\"";
    } else {
        return trust_was_reduced
            ? "\"이런 엄청난 요리를 나한테 그냥 주다니... 내가 그동안 자네를 너무 매정하게 몰아세웠나 보군. 그래, 사람이 살다 보면 실수도 하는 법이지! 예전처럼 다시 잘 지내보세나. (신용도 대폭 상승)\""
            : "\"맙소사, 이건 국왕 부럽지 않은 최고의 만찬이군! ...잠깐, 설마 자네 나를... 좋아하는 건가?! 아, 아무리 내가 매력적이어도 우린 상인과 농부 사이일 뿐이네! 어흠, 어쨌든 고맙네!\"";
    }
}
}

MarketSystem::MarketSystem() {
    buy_items_ = {
        {ItemType::SEED_POTATO, "감자 씨앗", 5, 0, 1},
        {ItemType::SEED_CARROT, "당근 씨앗", 10, 0, 1},
        {ItemType::SEED_WHEAT, "밀 씨앗", 15, 0, 6},
        {ItemType::SEED_RICE, "쌀 씨앗", 25, 0, 6},
        {ItemType::SEED_APPLE, "사과 묘목", 250, 0, 16},
        {ItemType::WOOD, "나무", 15, 0, 1},
        {ItemType::BABY_CHICKEN, "병아리", 40, 0, 1},
        {ItemType::BABY_PIG, "새끼 돼지", 100, 0, 6},
        {ItemType::BABY_COW, "송아지", 500, 0, 16},
    };

    sell_items_ = {
        {ItemType::POTATO, "감자", 0, 12, 1},
        {ItemType::CARROT, "당근", 0, 25, 1},
        {ItemType::WHEAT, "밀", 0, 35, 1},
        {ItemType::RICE, "쌀", 0, 60, 1},
        {ItemType::APPLE, "사과", 0, 150, 1},
        {ItemType::EGG, "달걀", 0, 20, 1},
        {ItemType::MILK, "우유", 0, 80, 1},
        {ItemType::PORK, "돼지고기", 0, 300, 1},
        {ItemType::FISH_MACKEREL, "고등어", 0, 40, 1},
        {ItemType::FISH_FLATFISH, "광어", 0, 75, 1},
        {ItemType::FISH_TUNA, "참치", 0, 250, 1},
        {ItemType::FOOD_POTATO_SOUP, "감자 수프", 0, 30, 1},
        {ItemType::FOOD_CARROT_EGG_ROLL, "당근 달걀 롤", 0, 75, 1},
        {ItemType::FOOD_MACKEREL_CARROT, "고등어 당근 조림", 0, 70, 1},
        {ItemType::FOOD_PORK_POTATO, "돼지고기 감자 짜글이", 0, 390, 1},
        {ItemType::FOOD_POTATO_WHEAT_PANCAKE, "감자 밀 부침개", 0, 130, 1},
        {ItemType::FOOD_CARROT_FLATFISH, "당근 광어 스테이크", 0, 210, 1},
        {ItemType::FOOD_3COLOR_CARROT_RICE, "3색 당근 쌀밥", 0, 220, 1},
        {ItemType::FOOD_MASHED_BEEF, "감자 매쉬드 비프", 0, 540, 1},
        {ItemType::FOOD_CARROT_APPLE_JUICE, "당근 사과 주스 만찬", 0, 320, 1},
        {ItemType::FOOD_TUNA_POTATO, "참치 감자 스테이크", 0, 1050, 1},
        {ItemType::FOOD_HAILIGHT_RICE, "특선 하이라이스", 0, 760, 1},
        {ItemType::FOOD_APPLE_POTATO_CAKE, "사과 감자 무스 케이크", 0, 460, 1},
        {ItemType::MONSTER_FOOD_LOW, "하급 몬스터 푸드", 0, 200, 1},
        {ItemType::MONSTER_FOOD_MID, "중급 몬스터 푸드", 0, 500, 1},
        {ItemType::MONSTER_FOOD_HIGH, "고급 몬스터 푸드", 0, 2000, 1},
        {ItemType::COPPER, "구리 광석", 0, 15, 1},
        {ItemType::IRON, "철 광석", 0, 30, 1},
        {ItemType::GOLD, "금 광석", 0, 75, 1},
    };
}

void MarketSystem::start_new_day() {}

void MarketSystem::show_monster_food_shop(Player& player, int kitchen_level) {
    (void)kitchen_level; // non-GUI 버전에서 사용하지 않음
    int tier = trust_tier(player.get_shop_trust());

    // 신용도 0 → 추방
    if (tier == 0) {
        system("clear");
        std::cout << entry_line(0) << "\n\n";
        std::cout << "상점에서 쫓겨났습니다. 선물로 신용도를 회복하세요.\n";
        wait_key();
        return;
    }

    // 입장 멘트
    system("clear");
    std::cout << "[괴식 판매] 신용도: " << player.get_shop_trust() << "/100\n";
    std::cout << entry_line(tier) << "\n\n";
    wait_key();

    // 판매할 괴식 목록
    const ItemType MONSTER_FOODS[] = {
        ItemType::MONSTER_FOOD_LOW,
        ItemType::MONSTER_FOOD_MID,
        ItemType::MONSTER_FOOD_HIGH,
    };
    const char* MONSTER_FOOD_NAMES[] = {"하급 몬스터 푸드", "중급 몬스터 푸드", "고급 몬스터 푸드"};
    const int MONSTER_FOOD_PRICES[] = {200, 500, 2000};

    while (true) {
        system("clear");
        // 신용도가 변동되었을 수 있으므로 재확인
        tier = trust_tier(player.get_shop_trust());
        if (tier == 0) {
            std::cout << entry_line(0) << "\n";
            wait_key();
            return;
        }

        std::cout << "[괴식 판매] 신용도: " << player.get_shop_trust()
                  << "/100 | 성공률: " << player.monster_food_success_chance() << "%\n";
        for (int i = 0; i < 3; ++i) {
            int have = player.count_item(MONSTER_FOODS[i]);
            int price = player.monster_food_sell_price(MONSTER_FOOD_PRICES[i]);
            if (have > 0)
                std::cout << (i + 1) << ". " << MONSTER_FOOD_NAMES[i]
                          << " - " << price << "G (보유 " << have << ")\n";
        }
        std::cout << "0. 뒤로\n>> " << std::flush;

        int choice = read_int_safe(-1);
        if (choice == 0) return;
        if (choice < 1 || choice > 3) continue;

        int idx = choice - 1;
        if (player.count_item(MONSTER_FOODS[idx]) <= 0) {
            std::cout << "보유한 " << MONSTER_FOOD_NAMES[idx] << "이(가) 없습니다.\n";
            wait_key();
            continue;
        }

        // 수량 입력
        std::cout << "수량: " << std::flush;
        int qty = read_int_safe(0);
        if (qty <= 0) continue;
        if (player.count_item(MONSTER_FOODS[idx]) < qty) {
            std::cout << "\n아이템 수량이 부족합니다.\n";
            wait_key();
            continue;
        }

        int price = player.monster_food_sell_price(MONSTER_FOOD_PRICES[idx]);
        int chance = player.monster_food_success_chance();
        int success = 0, fail = 0, protected_fail = 0, total_gold = 0;

        for (int i = 0; i < qty; ++i) {
            player.remove_item(MONSTER_FOODS[idx], 1);
            player.set_sold_monster_food(true);

            int cur_tier = trust_tier(player.get_shop_trust());

            if (std::rand() % 100 < chance) {
                ++success;
                total_gold += price;
                // 성공: 신용도 3-7 랜덤 하락
                int drop = 3 + std::rand() % 5; // 3~7
                player.lower_shop_trust(drop);
                system("clear");
                std::cout << "[판매 성공] " << MONSTER_FOOD_NAMES[idx] << "\n";
                std::cout << success_line(cur_tier) << "\n";
                std::cout << "(+" << price << "G | 신용도 -" << drop
                          << " → " << player.get_shop_trust() << "/100)\n";
                wait_key();
            } else {
                ++fail;
                if (player.protects_monster_food_failure()) {
                    ++protected_fail;
                    system("clear");
                    std::cout << "[판매 실패 - 능수능란한 언변으로 위기 모면]\n";
                    std::cout << fail_line(cur_tier) << "\n";
                    std::cout << "(신용도 변동 없음)\n";
                    wait_key();
                } else {
                    // 실패: 신용도 13-17 랜덤 하락
                    int drop = 13 + std::rand() % 5; // 13~17
                    player.lower_shop_trust(drop);
                    system("clear");
                    std::cout << "[판매 실패] " << MONSTER_FOOD_NAMES[idx] << "\n";
                    std::cout << fail_line(cur_tier) << "\n";
                    std::cout << "(신용도 -" << drop << " → " << player.get_shop_trust() << "/100)\n";
                    wait_key();
                }
            }

            // 신용도 0 → 즉시 퇴장
            if (player.get_shop_trust() <= 0) {
                system("clear");
                std::cout << entry_line(0) << "\n";
                wait_key();
                if (total_gold > 0) {
                    player.add_money(total_gold);
                    player.record_sale_income(total_gold);
                    player.add_exp(success * 2);
                }
                return;
            }
        }

        if (total_gold > 0) {
            player.add_money(total_gold);
            player.record_sale_income(total_gold);
            player.add_exp(success * 2);
        }

        system("clear");
        std::cout << "[괴식 거래 결과] 성공 " << success << "회 / 실패 " << fail << "회\n";
        std::cout << "획득 골드: +" << total_gold << "G | 현재 신용도: "
                  << player.get_shop_trust() << "/100\n";
        if (protected_fail > 0)
            std::cout << "능수능란한 언변으로 신용도 하락 " << protected_fail << "회 방어!\n";
        wait_key();
    }
}

void MarketSystem::show_gift_shop(Player& player, int kitchen_level) {
    bool trust_was_reduced = player.get_shop_trust() < 100;

    while (true) {
        system("clear");
        std::cout << "[선물하기] 신용도: " << player.get_shop_trust() << "/100\n\n";

        bool any = false;
        for (int i = 0; i < 3; ++i) {
            const GiftItem& g = GIFT_ITEMS[i];
            if (kitchen_level < g.kitchen_level_required) continue;
            int have = player.count_item(g.item);
            if (have <= 0) continue;
            std::cout << (i + 1) << ". " << g.name
                      << " (신용도 +" << g.trust_restore << ") [보유 " << have << "]\n";
            any = true;
        }
        if (!any) {
            std::cout << "(선물할 수 있는 음식이 없습니다.)\n";
        }
        std::cout << "0. 뒤로\n>> " << std::flush;

        int choice = read_int_safe(-1);
        if (choice == 0) return;
        if (choice < 1 || choice > 3) continue;

        int idx = choice - 1;
        const GiftItem& g = GIFT_ITEMS[idx];

        if (kitchen_level < g.kitchen_level_required) {
            std::cout << "주방 Lv." << g.kitchen_level_required << " 이상이 필요합니다.\n";
            wait_key();
            continue;
        }
        if (player.count_item(g.item) <= 0) {
            std::cout << "보유한 " << g.name << "이(가) 없습니다.\n";
            wait_key();
            continue;
        }

        player.remove_item(g.item, 1);
        player.restore_shop_trust(g.trust_restore);

        system("clear");
        std::cout << "[선물] " << g.name << " 전달!\n\n";
        std::cout << gift_line(g.trust_restore, trust_was_reduced) << "\n\n";
        std::cout << "신용도: +" << g.trust_restore << " → " << player.get_shop_trust() << "/100\n";
        wait_key();
        trust_was_reduced = player.get_shop_trust() < 100;
    }
}

bool MarketSystem::buy_item(Player& player, std::size_t index, int qty, std::string& message) {
    if (index >= buy_items_.size()) {
        message = "올바르지 않은 아이템입니다.";
        return false;
    }
    return buy(player, buy_items_[index], qty, message);
}

bool MarketSystem::sell_item(Player& player, std::size_t index, int qty, std::string& message) {
    if (index >= sell_items_.size()) {
        message = "올바르지 않은 아이템입니다.";
        return false;
    }
    return sell(player, sell_items_[index], qty, message);
}

void MarketSystem::show_buy_shop(Player& player) {
    while (true) {
        system("clear");
        std::cout << "[상점 - 구매] 골드: " << player.get_money()
                  << "G | 신뢰도: " << player.get_shop_trust() << "/100\n";
        for (std::size_t i = 0; i < buy_items_.size(); ++i) {
            const auto& item = buy_items_[i];
            int price = player.discounted_market_buy_price(item.buy_price);
            std::cout << (i + 1) << ". " << item.name << " - " << price << "G";
            if (player.get_level() < item.required_level) std::cout << " (Lv." << item.required_level << ")";
            if (player.is_shop_restricted() && item.required_level > 1) std::cout << " (신뢰도 제한)";
            std::cout << "\n";
        }
        std::cout << "0. 뒤로\n>> " << std::flush;

        int choice = read_int_safe(-1);
        if (choice == 0) return;
        if (choice < 1 || choice > static_cast<int>(buy_items_.size())) continue;
        std::cout << "수량: " << std::flush;
        int qty = read_int_safe(0);
        std::string msg;
        buy(player, buy_items_[choice - 1], qty, msg);
        std::cout << "\n" << msg << "\n";
        wait_key();
    }
}

void MarketSystem::show_sell_shop(Player& player) {
    while (true) {
        system("clear");
        std::cout << "[상점 - 판매] 골드: " << player.get_money()
                  << "G | 신뢰도: " << player.get_shop_trust() << "/100\n";
        for (std::size_t i = 0; i < sell_items_.size(); ++i) {
            const auto& item = sell_items_[i];
            // 괴식은 별도 창에서 처리
            if (is_monster_food(item.item)) continue;
            int have = player.count_item(item.item);
            if (have <= 0) continue;
            std::cout << (i + 1) << ". " << item.name << " - " << item.sell_price << "G"
                      << " (보유 " << have << ")\n";
        }
        std::cout << "0. 뒤로\n>> " << std::flush;

        int choice = read_int_safe(-1);
        if (choice == 0) return;
        if (choice < 1 || choice > static_cast<int>(sell_items_.size())) continue;
        const auto& item = sell_items_[choice - 1];
        std::cout << "수량: " << std::flush;
        int qty = read_int_safe(0);
        std::string msg;
        sell(player, item, qty, msg);
        std::cout << "\n" << msg << "\n";
        wait_key();
    }
}

bool MarketSystem::buy(Player& player, const MarketItem& item, int qty, std::string& message) {
    if (qty <= 0) {
        message = "수량은 1 이상이어야 합니다.";
        return false;
    }
    if (player.get_level() < item.required_level) {
        message = item.name + "은(는) 플레이어 Lv." + std::to_string(item.required_level) + "이 필요합니다.";
        return false;
    }
    if (player.is_shop_restricted() && item.required_level > 1) {
        message = "상점 신뢰도가 낮습니다. 회복 전까지 기본 품목만 구매할 수 있습니다.";
        return false;
    }
    int unit_price = player.discounted_market_buy_price(item.buy_price);
    int total = unit_price * qty;
    if (!player.spend_money(total)) {
        message = "골드가 부족합니다. 필요: " + std::to_string(total) + "G";
        return false;
    }
    if (!player.add_item(item.item, qty)) {
        player.add_money(total);
        message = "인벤토리 공간이 부족합니다.";
        return false;
    }
    message = item.name + " x" + std::to_string(qty) + " 구매 완료. 비용 "
            + std::to_string(total) + "G.";
    if (unit_price < item.buy_price) message += " INT 할인 적용.";
    return true;
}

bool MarketSystem::sell(Player& player, const MarketItem& item, int qty, std::string& message) {
    if (qty <= 0) {
        message = "수량은 1 이상이어야 합니다.";
        return false;
    }
    if (player.count_item(item.item) < qty) {
        message = "아이템 수량이 부족합니다.";
        return false;
    }

    if (is_monster_food(item.item)) {
        message = "괴식은 [괴식 판매] 메뉴에서 거래하세요.";
        return false;
    }

    float multiplier = 1.0f;
    Season season = season_from_day(player.get_day());
    if (item.item == ItemType::RICE && season == Season::SUMMER) multiplier = 1.5f;
    if (item.item == ItemType::APPLE && season == Season::FALL) multiplier = 1.5f;
    if (item.item == ItemType::WHEAT && season == Season::WINTER) multiplier = 1.3f;

    int total = static_cast<int>(item.sell_price * qty * multiplier);
    player.remove_item(item.item, qty);
    player.add_money(total);
    player.record_sale_income(total);
    player.add_exp(qty);
    message = item.name + " x" + std::to_string(qty) + " 판매 완료. 골드 +"
            + std::to_string(total) + "G.";
    return true;
}
