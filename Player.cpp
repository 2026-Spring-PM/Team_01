#include "Player.h"

#include <algorithm>

Player::Player()
    : x_(Map::BED_X),
      y_(Map::BED_Y),
      money_(INITIAL_MONEY),
      stamina_(LevelSystem::INITIAL_STAMINA),
      day_(1),
      tool_grade_(0),
      stamina_limit_(0),
      sick_today_(false),
      level_system_(),
      inventory_(INITIAL_INVENTORY_SLOTS),
      name_("초보 농부") {}

bool Player::try_move(int dx, int dy, const Map& map) {
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

bool Player::add_exp(int exp) {
    bool leveled = level_system_.add_exp(exp);
    if (leveled) {
        stamina_ = get_stamina_max();
        inventory_.expand(2);
    }
    return leveled;
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

void Player::set_tool_grade(int grade) {
    if (grade < 0) grade = 0;
    if (grade > 3) grade = 3;
    tool_grade_ = grade;
}

int Player::get_harvest_stamina_cost() const {
    switch (tool_grade_) {
        case 1: return 7;
        case 2: return 5;
        case 3: return 2;
        default: return 10;
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
