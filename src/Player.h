#pragma once

#include "Map.h"
#include "Inventory.h"
#include "ItemType.h"
#include "LevelSystem.h"

#include <string>

class Player {
public:
    static constexpr int INITIAL_MONEY = 50;
    static constexpr int INITIAL_INVENTORY_SLOTS = 40;

    Player();

    void set_name(const std::string& name) { name_ = name; }
    const std::string& get_name() const { return name_; }

    bool try_move(int dx, int dy, const Map& map);
    bool try_sleep(const Map& map);
    void sleep_anywhere();

    void add_money(int amount);
    bool spend_money(int amount);
    bool use_stamina(int cost);
    void restore_stamina(int amount);
    bool add_exp(int exp);
    void expand_inventory_slots(int additional);

    bool add_item(ItemType type, int qty = 1);
    bool remove_item(ItemType type, int qty = 1);
    int count_item(ItemType type) const;
    const Inventory<ItemType>& inventory() const { return inventory_; }

    int get_x() const { return x_; }
    int get_y() const { return y_; }
    void set_position(int x, int y);
    int get_money() const { return money_; }
    int get_stamina() const { return stamina_; }
    int get_stamina_max() const { return stamina_limit_ > 0 ? stamina_limit_ : level_system_.get_max_stamina(); }
    int get_true_stamina_max() const { return level_system_.get_max_stamina(); }
    int get_day() const { return day_; }
    int get_level() const { return level_system_.get_level(); }
    int get_exp() const { return level_system_.get_exp(); }
    int get_exp_to_next() const { return level_system_.get_exp_to_next_level(); }
    int get_storage_level() const { return storage_level_; }
    void set_storage_level(int level);

    int get_tool_grade() const { return tool_grade_; }
    char get_facing() const { return facing_; } // f/front, b/back, l/left, r/right
    void set_facing(char facing) { facing_ = facing; }
    void set_tool_grade(int grade);
    int get_harvest_stamina_cost() const;

    void apply_cold_penalty();
    void recover_daily_status();
    bool is_sick_today() const { return sick_today_; }
    const std::string& get_levelup_message() const { return levelup_message_; }
    void clear_levelup_message() { levelup_message_.clear(); }
private:
    int x_;
    int y_;
    int money_;
    int stamina_;
    int day_;
    int tool_grade_;       // 0 기본, 1 구리, 2 철, 3 금
    int storage_level_;     // 0 없음, 1~3 저장소 레벨
    char facing_;          // GUI용 바라보는 방향: f/b/l/r
    int stamina_limit_;    // 0이면 제한 없음, 감기 이벤트 때 임시 제한
    bool sick_today_;
    std::string levelup_message_;

    LevelSystem level_system_;
    Inventory<ItemType> inventory_;
    std::string name_;

};
