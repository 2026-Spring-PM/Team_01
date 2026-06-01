#pragma once

#include "Map.h"
#include "Inventory.h"
#include "ItemType.h"
#include "LevelSystem.h"

#include <string>

class Player {
public:
    static constexpr int INITIAL_MONEY = 25;
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
    int get_money() const { return money_; }
    int get_stamina() const { return stamina_; }
    int get_stamina_max() const { return stamina_limit_ > 0 ? stamina_limit_ : get_true_stamina_max(); }
    int get_true_stamina_max() const;
    int get_day() const { return day_; }
    int get_level() const { return level_system_.get_level(); }
    int get_exp() const { return level_system_.get_exp(); }
    int get_exp_to_next() const { return level_system_.get_exp_to_next_level(); }
    int get_storage_level() const { return storage_level_; }
    void set_storage_level(int level);

    // 농기구 3종 슬롯 (0=없음, 1=동제, 2=철제, 3=금제)
    int get_hoe_grade() const { return hoe_grade_; }
    int get_rod_grade() const { return rod_grade_; }
    int get_pick_grade() const { return pick_grade_; }
    int get_tool_grade() const { return hoe_grade_; }
    
    void set_hoe_grade(int grade) { hoe_grade_ = (grade<0)?0:(grade>3)?3:grade; }
    void set_rod_grade(int grade) { rod_grade_ = (grade<0)?0:(grade>3)?3:grade; }
    void set_pick_grade(int grade) { pick_grade_ = (grade<0)?0:(grade>3)?3:grade; }

    char get_facing() const { return facing_; }
    int get_harvest_stamina_cost() const;
    int get_fishing_stamina_cost() const;
    int get_mining_stamina_cost() const;

    // 스킬 포인트 시스템
    int get_skill_points() const { return skill_points_; }

    // 트리별 투자 함수 
    bool invest_str_node(int node_id, std::string& message);
    bool invest_int_node(int node_id, std::string& message);
    bool invest_luk_node(int node_id, std::string& message);

    // 각 스킬의 해금 여부를 확인하는 getter
    bool has_str_node(int node_id) const;
    bool has_int_node(int node_id) const;
    bool has_luk_node(int node_id) const;

    void apply_cold_penalty();
    void recover_daily_status();
    bool is_sick_today() const { return sick_today_; }
    const std::string& get_levelup_message() const { return levelup_message_; }
    void clear_levelup_message() { levelup_message_.clear(); }

    // 별 내리는 밤 이벤트 활성화 여부
    bool is_starry_night() const { return starry_night_days_ > 0; }
    void activate_starry_night(int days) { starry_night_days_ = days; }
    void tick_starry_night() { if (starry_night_days_ > 0) --starry_night_days_; }

    // 야생동물 출몰 이벤트
    bool is_wildlife_today() const { return wildlife_today_; }
    void set_wildlife_today(bool v) { wildlife_today_ = v; }

    // 괴식 판매
    bool has_sold_monster_food() const { return sold_monster_food_; }
    void set_sold_monster_food(bool sold) { sold_monster_food_ = sold; }

private:
    int x_;
    int y_;
    int money_;
    int stamina_;
    int day_;
    int hoe_grade_;    // 0~3
    int rod_grade_;
    int pick_grade_;
    int storage_level_;
    char facing_;
    int stamina_limit_;
    bool sick_today_;
    bool wildlife_today_;
    int starry_night_days_;
    std::string levelup_message_;

    // 스킬 포인트
    int skill_points_;
    bool str_nodes_[7] = { false, false, false, false, false, false, false };
    bool int_nodes_[9] = { false, false, false, false, false, false, false, false, false };
    bool luk_nodes_[7] = { false, false, false, false, false, false, false };

    bool sold_monster_food_ = false;

    LevelSystem level_system_;
    Inventory<ItemType> inventory_;
    std::string name_;
};
