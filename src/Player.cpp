#include "Player.h"

#include <algorithm>

Player::Player()
    : x_(Map::BED_X),
      y_(Map::BED_Y),
      money_(INITIAL_MONEY),
      stamina_(LevelSystem::INITIAL_STAMINA),
      day_(1),
      tool_grade_(0),
      hoe_grade_(0),
      fishing_rod_grade_(0),
      pickaxe_grade_(0),
      storage_level_(0),
      facing_('f'),
      stamina_limit_(0),
      sick_today_(false),
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
    bool leveled = level_system_.add_exp(exp);
    if (leveled) {
        stamina_ = get_stamina_max();
        levelup_message_ = "레벨업! Lv." + std::to_string(get_level()) + " 달성! 스태미나 MAX 회복!";

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

void Player::set_tool_grade(int grade) {
    if (grade < 0) grade = 0;
    if (grade > 3) grade = 3;
    tool_grade_ = grade;
}

int Player::get_harvest_stamina_cost() const {
    switch (hoe_grade_) {
        case 1: return 2;
        case 2: return 1;
        case 3: return 0;
        default: return 3;
    }
}

void Player::apply_cold_penalty() {
    sick_today_ = true;
    stamina_limit_ = std::max(1, level_system_.get_max_stamina() / 2);
    if (stamina_ > stamina_limit_) stamina_ = stamina_limit_;
}

void Player::recover_daily_status() {
    sick_today_ = false;
    stamina_limit_ = 0;
    stamina_ = level_system_.get_max_stamina();
}
