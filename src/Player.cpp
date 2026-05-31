#include "Player.h"
#include "LevelSystem.h"

#include <algorithm>

Player::Player()
    : x_(Map::BED_X),
      y_(Map::BED_Y),
      money_(INITIAL_MONEY),
      stamina_(LevelSystem::INITIAL_STAMINA),
      day_(1),
      hoe_grade_(0),
      rod_grade_(0),
      pick_grade_(0),
      storage_level_(0),
      facing_('f'),
      stamina_limit_(0),
      sick_today_(false),
      wildlife_today_(false),
      starry_night_days_(0),
      skill_points_(0),
      level_system_(),
      inventory_(INITIAL_INVENTORY_SLOTS),
      name_("초보 농부") {}

bool Player::try_move(int dx, int dy, const Map& map) {
    if (dy < 0) facing_ = 'b';
    else if (dy > 0) facing_ = 'f';
    else if (dx < 0) facing_ = 'l';
    else if (dx > 0) facing_ = 'r';

    int new_x = x_ + dx;
    int new_y = y_ + dy;
    if (!map.is_in_bounds(new_x, new_y)) return false;
    if (!map.get_tile(new_x, new_y).is_walkable()) return false;
    x_ = new_x;
    y_ = new_y;
    return true;
}

bool Player::try_sleep(const Map& map) {
    (void)map;
    sleep_anywhere();
    return true;
}

void Player::sleep_anywhere() {
    ++day_;
    recover_daily_status();
}

void Player::add_money(int amount) {
    if (amount > 0) money_ += amount;
}

bool Player::spend_money(int amount) {
    if (amount < 0 || money_ < amount) return false;
    money_ -= amount;
    return true;
}

bool Player::use_stamina(int cost) {
    if (cost < 0 || stamina_ < cost) return false;
    stamina_ -= cost;
    return true;
}

void Player::restore_stamina(int amount) {
    if (amount <= 0) return;
    stamina_ = std::min(get_stamina_max(), stamina_ + amount);
}

bool Player::add_exp(int exp) {
    int old_level = get_level();
    bool leveled = level_system_.add_exp(exp);
    if (leveled) {
        stamina_ = get_stamina_max();
        int new_level = get_level();
        levelup_message_ = "레벨업! Lv." + std::to_string(new_level) + " 달성! 스태미나 MAX 회복!";
        // 스킬 포인트 지급: 보너스 레벨(6,10,16,20,26,30)에서 지급
        for (int lv = old_level + 1; lv <= new_level; ++lv) {
            if (LevelSystem::is_bonus_level(lv)) {
                ++skill_points_;
                levelup_message_ += " 🎯 스킬 포인트 +1!";
            }
        }
    }
    return leveled;
}

void Player::expand_inventory_slots(int additional) {
    inventory_.expand(additional);
}

bool Player::add_item(ItemType type, int qty) {
    return inventory_.add(type, qty);
}

bool Player::remove_item(ItemType type, int qty) {
    return inventory_.remove(type, qty);
}

int Player::count_item(ItemType type) const {
    return inventory_.count(type);
}

void Player::set_storage_level(int level) {
    if (level < 0) level = 0;
    if (level > 3) level = 3;
    storage_level_ = level;
}

int Player::get_true_stamina_max() const {
    int base = level_system_.get_max_stamina();
    // STR 노드 1(거대 위장): +5
    if (str_nodes_[1]) base += 5;
    // STR 노드 4(강철 폐활량): +10
    if (str_nodes_[4]) base += 10;
    return base;
}


int Player::get_harvest_stamina_cost() const {
    return 1;
}

int Player::get_fishing_stamina_cost() const {
    // 금제 낚싯대는 -2 감소 (5->3)
    if (rod_grade_ >= 3) return 3;
    return 5;
}

int Player::get_mining_stamina_cost() const {
    // 금제 곡괭이는 -2 감소 (5->3)
    if (pick_grade_ >= 3) return 3;
    return 5;
}

bool Player::has_str_node(int node_id) const { return (node_id >= 1 && node_id <= 6) ? str_nodes_[node_id] : false; }
bool Player::has_int_node(int node_id) const { return (node_id >= 1 && node_id <= 8) ? int_nodes_[node_id] : false; }
bool Player::has_luk_node(int node_id) const { return (node_id >= 1 && node_id <= 6) ? luk_nodes_[node_id] : false; }

// STR 스킬 투자 로직 (2단계, 4단계 분기형)
bool Player::invest_str_node(int node_id, std::string& message) {
    if (skill_points_ <= 0) { message = "스킬 포인트가 부족합니다."; return false; }
    if (node_id < 1 || node_id > 6) { message = "잘못된 스킬 번호입니다."; return false; }
    if (str_nodes_[node_id]) { message = "이미 배운 스킬입니다."; return false; }

    // 선행 스킬 조건 검사
    if (node_id == 2 || node_id == 3) { // 2단계
        if (!str_nodes_[1]) { message = "선행 스킬(1단계)이 필요합니다."; return false; }
    } else if (node_id == 4) { // 3단계
        if (!str_nodes_[2] && !str_nodes_[3]) { message = "선행 스킬(2단계 중 하나 이상)이 필요합니다."; return false; }
    } else if (node_id == 5 || node_id == 6) { // 4단계
        if (!str_nodes_[4]) { message = "선행 스킬(3단계)이 필요합니다."; return false; }
    }

    --skill_points_;
    str_nodes_[node_id] = true;

    // 투자 즉시 효과 적용 (최대 스태미나 증가 시 현재 스태미나도 재조정)
    if (node_id == 1) message = "💪 [거대 위장] 해금! 최대 스태미나 +5";
    else if (node_id == 2) message = "💪 [압도적인 기백] 해금! 감기 완화 & 야생동물 무효화";
    else if (node_id == 3) message = "💪 [채광왕] 해금! 광산 30% 확률로 붕괴 버팀";
    else if (node_id == 4) message = "💪 [강철 폐활량] 해금! 최대 스태미나 추가 +10";
    else if (node_id == 5) message = "💪 [대지의 아들] 해금! 수확 시 20% 확률로 스태미나 소모 면제";
    else if (node_id == 6) message = "💪 [헤라클레스] 해금! 광산 붕괴 무조건 버팀 & 보상 2배";
    
    stamina_ = std::min(stamina_, get_stamina_max());
    return true;
}

// INT 스킬 투자 로직 (정상 루트 1~4 / 괴식 루트 5~8)
bool Player::invest_int_node(int node_id, std::string& message) {
    if (skill_points_ <= 0) { message = "스킬 포인트가 부족합니다."; return false; }
    if (node_id < 1 || node_id > 8) { message = "잘못된 스킬 번호입니다."; return false; }
    if (int_nodes_[node_id]) { message = "이미 배운 스킬입니다."; return false; }

    // 선행 스킬 조건 검사
    if (node_id == 2) { 
        if (!int_nodes_[1]) { message = "선행 스킬([공정 최적화])이 필요합니다."; return false; } 
    }
    else if (node_id == 3) { 
        if (!int_nodes_[2]) { message = "선행 스킬([재무 설계])이 필요합니다."; return false; } 
    }
    else if (node_id == 4) { 
        if (!int_nodes_[3]) { message = "선행 스킬([단골 손님])이 필요합니다."; return false; } 
    }
    // 괴식 루트 (히든 빌드)
    else if (node_id == 6) { 
        if (!int_nodes_[5]) { message = "선행 스킬([야바위꾼])이 필요합니다."; return false; } 
    }
    else if (node_id == 7) { 
        if (!int_nodes_[6]) { message = "선행 스킬([위조 전문가])이 필요합니다."; return false; } 
    }
    else if (node_id == 8) { 
        if (!int_nodes_[7]) { message = "선행 스킬([능수능란한 언변])이 필요합니다."; return false; } 
    }

    --skill_points_;
    int_nodes_[node_id] = true;

    // 투자 즉시 메시지 반환
    switch(node_id) {
        case 1: message = "💡 [공정 최적화] 해금! 주방/대장간 업그레이드 및 제작 비용 20% 감소"; break;
        case 2: message = "💡 [재무 설계] 해금! 매일 아침 소지 골드의 1% 이자 획득 (최대 100G)"; break;
        case 3: message = "💡 [단골 손님] 해금! 상점 물품 구매 가격 10% 할인"; break;
        case 4: message = "💡 [농업 협동조합장] 해금! 오늘 판매 총수익의 10%를 내일 아침 배당금으로 획득"; break;
        case 5: message = "💡 [야바위꾼] 해금! 괴식 판매 시 기본 단가 +10%"; break;
        case 6: message = "💡 [위조 전문가] 해금! 괴식 사기 판매 성공률 +20%"; break;
        case 7: message = "💡 [능수능란한 언변] 해금! 괴식 사기 판매 실패 시 50% 확률로 신뢰도 하락 무효"; break;
        case 8: message = "💡 [괴도 루팡] 해금! 괴식 사기 판매 성공률 추가 +20%"; break;
    }
    return true;
}

// LUK 스킬 투자 로직 (2단계, 4단계 분기형)
bool Player::invest_luk_node(int node_id, std::string& message) {
    if (skill_points_ <= 0) { message = "스킬 포인트가 부족합니다."; return false; }
    if (node_id < 1 || node_id > 6) { message = "잘못된 스킬 번호입니다."; return false; }
    if (luk_nodes_[node_id]) { message = "이미 배운 스킬입니다."; return false; }

    // 선행 스킬 조건 검사
    if (node_id == 2 || node_id == 3) { // 2단계 (둘 중 하나, 혹은 둘 다 가능)
        if (!luk_nodes_[1]) { message = "선행 스킬([씨앗 환급])이 필요합니다."; return false; } 
    }
    else if (node_id == 4) { // 3단계 (2단계 중 최소 하나 이상 필요)
        if (!luk_nodes_[2] && !luk_nodes_[3]) { message = "선행 스킬(2단계 스킬 중 최소 1개)이 필요합니다."; return false; } 
    }
    else if (node_id == 5 || node_id == 6) { // 4단계 (둘 중 하나, 혹은 둘 다 가능)
        if (!luk_nodes_[4]) { message = "선행 스킬([알뜰한 손재주])이 필요합니다."; return false; } 
    }

    --skill_points_;
    luk_nodes_[node_id] = true;

    // 투자 즉시 메시지 반환
    switch(node_id) {
        case 1: message = "🍀 [씨앗 환급] 해금! 수확 시 10% 확률로 심었던 씨앗 회수"; break;
        case 2: message = "🍀 [쌍둥이 가축] 해금! 가축 생산물 수확 시 20% 확률로 1개 더 획득"; break;
        case 3: message = "🍀 [강태공의 가호] 해금! 각 생선 지급 확률 +10%"; break;
        case 4: message = "🍀 [알뜰한 손재주] 해금! 요리 시 요구 재료 중 무작위 1개 소모 방어"; break;
        case 5: message = "🍀 [우주적 기연] 해금! '별 내리는 밤' 발생 확률 10%로 대폭 상승"; break;
        case 6: message = "🍀 [황금 어장의 지배자] 해금! 생선 지급 확률 추가 +10% 및 10% 확률로 거대한 그물 발동"; break;
    }
    return true;
}

void Player::apply_cold_penalty() {
    sick_today_ = true;
    int true_max = get_true_stamina_max();
    // STR 2단계: 감기 감소 -50% -> -20%
    if (has_str_node(2)) {
        stamina_limit_ = std::max(1, true_max * 80 / 100);
    } else {
        stamina_limit_ = std::max(1, true_max / 2);
    }
    if (stamina_ > stamina_limit_) stamina_ = stamina_limit_;
}

void Player::recover_daily_status() {
    sick_today_ = false;
    stamina_limit_ = 0;
    wildlife_today_ = false;
    tick_starry_night();
    stamina_ = get_true_stamina_max();
}
