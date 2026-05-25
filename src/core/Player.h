#pragma once

#include "Map.h"
#include "Inventory.h"
#include "ItemType.h"
#include "LevelSystem.h"

#include <string>

// 플레이어 클래스
class Player {
public:
    static constexpr int INITIAL_MONEY = 500;
    static constexpr int INITIAL_INVENTORY_SLOTS = 20;

    Player();

    // === 이름 ===
    void set_name(const std::string& name) { name_ = name; }
    const std::string& get_name() const { return name_; }

    // === 이동 / 잠자기 ===
    bool try_move(int dx, int dy, const Map& map);
    bool try_sleep(const Map& map);

    // === 자원 관리 ===
    void add_money(int amount) { money_ += amount; }
    bool spend_money(int amount);
    bool use_stamina(int cost);
    bool add_exp(int exp);

    // === 인벤토리 ===
    bool add_item(ItemType type, int qty = 1);
    bool remove_item(ItemType type, int qty = 1);
    int count_item(ItemType type) const;
    const Inventory<ItemType>& inventory() const { return inventory_; }

    // === 게터 ===
    int get_x() const { return x_; }
    int get_y() const { return y_; }
    int get_money() const { return money_; }
    int get_stamina() const { return stamina_; }
    int get_stamina_max() const { return level_system_.get_max_stamina(); }
    int get_day() const { return day_; }
    int get_level() const { return level_system_.get_level(); }
    int get_exp() const { return level_system_.get_exp(); }
    int get_exp_to_next() const { return level_system_.get_exp_to_next_level(); }

private:
    // 위치
    int x_;
    int y_;

    // 자원
    int money_;
    int stamina_;

    // 시간
    int day_;

    // 시스템
    LevelSystem level_system_;
    Inventory<ItemType> inventory_;

    // 이름
    std::string name_;  // ← 핵심!
};