#include "Player.h"

Player::Player()
    : x_(Map::BED_X),
      y_(Map::BED_Y),
      money_(INITIAL_MONEY),
      stamina_(LevelSystem::INITIAL_STAMINA),
      day_(1),
      level_system_(),
      inventory_(INITIAL_INVENTORY_SLOTS) {
}

bool Player::try_move(int dx, int dy, const Map& map) {
    int new_x = x_ + dx;
    int new_y = y_ + dy;

    if (!map.get_tile(new_x, new_y).is_walkable()) {
        return false;
    }

    x_ = new_x;
    y_ = new_y;
    return true;
}

bool Player::try_sleep(const Map& map) {
    if (!map.get_tile(x_, y_).is_bed()) {
        return false;
    }

    ++day_;
    stamina_ = level_system_.get_max_stamina();
    return true;
}

bool Player::spend_money(int amount) {
    if (money_ < amount) {
        return false;
    }
    money_ -= amount;
    return true;
}

bool Player::use_stamina(int cost) {
    if (stamina_ < cost) {
        return false;
    }
    stamina_ -= cost;
    return true;
}

bool Player::add_exp(int exp) {
    return level_system_.add_exp(exp);
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