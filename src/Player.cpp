#include "Player.h"

#include <algorithm>
#include <cstdlib>

namespace {
int clamp_grade(int grade) {
    return std::max(0, std::min(3, grade));
}
}

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
      str_nodes_{false, false, false, false, false, false, false},
      int_nodes_{false, false, false, false, false, false, false, false, false},
      luk_nodes_{false, false, false, false, false, false, false},
      sold_monster_food_(false),
      shop_trust_(100),
      daily_sales_income_(0),
      pending_dividend_(0),
      level_system_(),
      inventory_(INITIAL_INVENTORY_SLOTS),
      name_("초보 농부") {}

void Player::set_position(int x, int y) {
    x_ = std::max(0, std::min(Map::WIDTH - 1, x));
    y_ = std::max(0, std::min(Map::HEIGHT - 1, y));
}

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
    if (!leveled) return false;

    stamina_ = get_stamina_max();
    int new_level = get_level();
    levelup_message_ = "레벨업! Lv." + std::to_string(new_level) + " 달성! 스태미나가 회복되었습니다.";
    for (int lv = old_level + 1; lv <= new_level; ++lv) {
        if (LevelSystem::is_bonus_level(lv)) {
            ++skill_points_;
            levelup_message_ += " 스킬 포인트 +1";
        }
    }
    return true;
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
    storage_level_ = std::max(0, std::min(3, level));
}

void Player::set_hoe_grade(int grade) {
    hoe_grade_ = clamp_grade(grade);
}

void Player::set_rod_grade(int grade) {
    rod_grade_ = clamp_grade(grade);
}

void Player::set_pick_grade(int grade) {
    pick_grade_ = clamp_grade(grade);
}

int Player::get_true_stamina_max() const {
    int base = level_system_.get_max_stamina();
    if (has_str_node(1)) base += 5;
    if (has_str_node(4)) base += 10;
    return base;
}

int Player::get_harvest_stamina_cost() const {
    switch (hoe_grade_) {
        case 1: return 2;
        case 2: return 1;
        case 3: return 1;
        default: return 2;
    }
}

int Player::get_fishing_stamina_cost() const {
    return rod_grade_ >= 3 ? 1 : 2;
}

int Player::get_mining_stamina_cost() const {
    switch (pick_grade_) {
        case 1: return 2;
        case 2: return 1;
        case 3: return 1;
        default: return 2;
    }
}

bool Player::has_str_node(int node_id) const {
    return node_id >= 1 && node_id <= 6 && str_nodes_[node_id];
}

bool Player::has_int_node(int node_id) const {
    return node_id >= 1 && node_id <= 8 && int_nodes_[node_id];
}

bool Player::has_luk_node(int node_id) const {
    return node_id >= 1 && node_id <= 6 && luk_nodes_[node_id];
}

bool Player::invest_str_node(int node_id, std::string& message) {
    if (skill_points_ <= 0) { message = "스킬 포인트가 부족합니다."; return false; }
    if (node_id < 1 || node_id > 6) { message = "없는 STR 스킬입니다."; return false; }
    if (str_nodes_[node_id]) { message = "이미 배운 STR 스킬입니다."; return false; }
    if ((node_id == 2 || node_id == 3) && !str_nodes_[1]) { message = "STR 1번 스킬이 필요합니다."; return false; }
    if (node_id == 4 && !str_nodes_[2] && !str_nodes_[3]) { message = "STR 2번 또는 3번 스킬이 필요합니다."; return false; }
    if ((node_id == 5 || node_id == 6) && !str_nodes_[4]) { message = "STR 4번 스킬이 필요합니다."; return false; }

    --skill_points_;
    str_nodes_[node_id] = true;
    static const char* names[] = {"", "건강한 몸", "감기 저항", "채굴 집중", "강인한 체력", "농사 숙련", "광산 숙련"};
    message = std::string("STR 스킬 해금: ") + names[node_id];
    stamina_ = std::min(stamina_, get_stamina_max());
    return true;
}

bool Player::invest_int_node(int node_id, std::string& message) {
    if (skill_points_ <= 0) { message = "스킬 포인트가 부족합니다."; return false; }
    if (node_id < 1 || node_id > 8) { message = "없는 INT 스킬입니다."; return false; }
    if (int_nodes_[node_id]) { message = "이미 배운 INT 스킬입니다."; return false; }
    if (node_id >= 2 && node_id <= 4 && !int_nodes_[node_id - 1]) { message = "이전 INT 스킬이 필요합니다."; return false; }
    if (node_id >= 6 && node_id <= 8 && !int_nodes_[node_id - 1]) { message = "이전 INT 스킬이 필요합니다."; return false; }

    --skill_points_;
    int_nodes_[node_id] = true;
    static const char* names[] = {"", "공정 최적화", "재무 설계", "할인 감각", "협동 조합", "몬스터 조리", "제조 전문가", "손실 방지", "관리 루틴"};
    message = std::string("INT 스킬 해금: ") + names[node_id];
    return true;
}

bool Player::invest_luk_node(int node_id, std::string& message) {
    if (skill_points_ <= 0) { message = "스킬 포인트가 부족합니다."; return false; }
    if (node_id < 1 || node_id > 6) { message = "없는 LUK 스킬입니다."; return false; }
    if (luk_nodes_[node_id]) { message = "이미 배운 LUK 스킬입니다."; return false; }
    if ((node_id == 2 || node_id == 3) && !luk_nodes_[1]) { message = "LUK 1번 스킬이 필요합니다."; return false; }
    if (node_id == 4 && !luk_nodes_[2] && !luk_nodes_[3]) { message = "LUK 2번 또는 3번 스킬이 필요합니다."; return false; }
    if ((node_id == 5 || node_id == 6) && !luk_nodes_[4]) { message = "LUK 4번 스킬이 필요합니다."; return false; }

    --skill_points_;
    luk_nodes_[node_id] = true;
    static const char* names[] = {"", "씨앗 회수", "축산 행운", "낚시 감각", "요리 재능", "별자리 기운", "황금 어장"};
    message = std::string("LUK 스킬 해금: ") + names[node_id];
    return true;
}

void Player::apply_cold_penalty() {
    sick_today_ = true;
    int true_max = get_true_stamina_max();
    stamina_limit_ = has_str_node(2) ? std::max(1, true_max * 80 / 100) : std::max(1, true_max / 2);
    if (stamina_ > stamina_limit_) stamina_ = stamina_limit_;
}

void Player::recover_daily_status() {
    sick_today_ = false;
    stamina_limit_ = 0;
    wildlife_today_ = false;
    stamina_ = get_true_stamina_max();
}

void Player::lower_shop_trust(int amount) {
    if (amount <= 0) return;
    shop_trust_ = std::max(0, shop_trust_ - amount);
}

void Player::restore_shop_trust(int amount) {
    if (amount <= 0) return;
    shop_trust_ = std::min(100, shop_trust_ + amount);
}

void Player::record_sale_income(int amount) {
    if (amount > 0) daily_sales_income_ += amount;
}

std::string Player::start_new_day_economy() {
    std::string message;

    if (pending_dividend_ > 0) {
        add_money(pending_dividend_);
        message += "Co-op dividend +" + std::to_string(pending_dividend_) + "G. ";
        pending_dividend_ = 0;
    }

    if (has_int_node(2)) {
        int interest = std::min(100, std::max(0, money_ / 100));
        if (interest > 0) {
            add_money(interest);
            message += "Interest +" + std::to_string(interest) + "G. ";
        }
    }

    if (has_int_node(4) && daily_sales_income_ > 0) {
        pending_dividend_ = daily_sales_income_ / 10;
        if (pending_dividend_ > 0) {
            message += "Co-op dividend reserved +" + std::to_string(pending_dividend_) + "G for tomorrow. ";
        }
    }

    daily_sales_income_ = 0;
    return message;
}

int Player::discounted_upgrade_cost(int base_cost) const {
    if (base_cost <= 0) return base_cost;
    return has_int_node(1) ? std::max(1, base_cost * 80 / 100) : base_cost;
}

int Player::discounted_market_buy_price(int base_price) const {
    if (base_price <= 0) return base_price;
    return has_int_node(3) ? std::max(1, base_price * 90 / 100) : base_price;
}

int Player::monster_food_sell_price(int base_price) const {
    if (base_price <= 0) return base_price;
    return has_int_node(5) ? base_price * 110 / 100 : base_price;
}

int Player::monster_food_success_chance() const {
    int chance = 20;
    if (has_int_node(6)) chance += 20;
    if (has_int_node(8)) chance += 20;
    return std::min(95, chance);
}

bool Player::protects_monster_food_failure() const {
    return has_int_node(7) && (std::rand() % 100 < 50);
}

bool Player::should_refund_seed() const {
    return has_luk_node(1) && (std::rand() % 100 < 10);
}

bool Player::should_skip_harvest_stamina() const {
    return has_str_node(5) && (std::rand() % 100 < 20);
}

bool Player::should_double_livestock_product() const {
    return has_luk_node(2) && (std::rand() % 100 < 20);
}

bool Player::should_save_cooking_ingredient() const {
    return has_luk_node(4);
}

bool Player::should_trigger_starry_night() const {
    int chance = has_luk_node(5) ? 10 : 1;
    return std::rand() % 100 < chance;
}
