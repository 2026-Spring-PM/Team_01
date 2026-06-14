#pragma once

#include "GameObject.h"
#include "Map.h"
#include "Inventory.h"
#include "ItemType.h"
#include "LevelSystem.h"

#include <string>

class Player : public GameObject {
public:
    static constexpr int INITIAL_MONEY = 50;
    static constexpr int INITIAL_INVENTORY_SLOTS = 40;

    Player();

    void set_name(const std::string& name) { name_ = name; }
    const std::string& get_name() const { return name_; }

    bool try_move(int dx, int dy, const Map& map);
    bool try_sleep(const Map& map);
    void sleep_anywhere();

    // GameObject interface: used through a base-class pointer when a day passes.
    void update() override;
    Position get_position() const override { return Position{x_, y_}; }
    void set_position(const Position& position) override;
    std::string debug_name() const override { return name_; }

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
    int get_stamina_max() const { return stamina_limit_ > 0 ? stamina_limit_ : get_true_stamina_max(); }
    int get_true_stamina_max() const;
    int get_day() const { return day_; }
    int get_level() const { return level_system_.get_level(); }
    int get_exp() const { return level_system_.get_exp(); }
    int get_exp_to_next() const { return level_system_.get_exp_to_next_level(); }
    int get_storage_level() const { return storage_level_; }
    void set_storage_level(int level);

    int get_tool_grade() const { return hoe_grade_; }
    int get_hoe_grade() const { return hoe_grade_; }
    int get_rod_grade() const { return rod_grade_; }
    int get_pick_grade() const { return pick_grade_; }
    void set_tool_grade(int grade) { set_hoe_grade(grade); }
    void set_hoe_grade(int grade);
    void set_rod_grade(int grade);
    void set_pick_grade(int grade);

    char get_facing() const { return facing_; }
    void set_facing(char facing) { facing_ = facing; }
    int get_harvest_stamina_cost() const;
    int get_fishing_stamina_cost() const;
    int get_mining_stamina_cost() const;

    int get_skill_points() const { return skill_points_; }
    bool invest_str_node(int node_id, std::string& message);
    bool invest_int_node(int node_id, std::string& message);
    bool invest_luk_node(int node_id, std::string& message);
    bool has_str_node(int node_id) const;
    bool has_int_node(int node_id) const;
    bool has_luk_node(int node_id) const;

    void apply_cold_penalty();
    void recover_daily_status();
    bool is_sick_today() const { return sick_today_; }
    const std::string& get_levelup_message() const { return levelup_message_; }
    void clear_levelup_message() { levelup_message_.clear(); }

    bool is_starry_night() const { return starry_night_days_ > 0; }
    void activate_starry_night(int days) { starry_night_days_ = days; }
    void tick_starry_night() { if (starry_night_days_ > 0) --starry_night_days_; }

    bool is_wildlife_today() const { return wildlife_today_; }
    void set_wildlife_today(bool value) { wildlife_today_ = value; }

    bool has_sold_monster_food() const { return sold_monster_food_; }
    void set_sold_monster_food(bool sold) { sold_monster_food_ = sold; }
    bool has_made_monster_food() const { return made_monster_food_; }
    void set_made_monster_food(bool v) { made_monster_food_ = v; }
    // 신용도가 처음으로 71 미만으로 떨어진 적 있는지 (대사/안내 변화 트리거)
    bool trust_ever_dropped() const { return trust_ever_dropped_; }
    void mark_trust_dropped() { trust_ever_dropped_ = true; }
    int get_shop_trust() const { return shop_trust_; }
    void lower_shop_trust(int amount);
    void restore_shop_trust(int amount);
    bool is_shop_restricted() const { return shop_trust_ <= 30; }
    void record_sale_income(int amount);
    std::string start_new_day_economy();
    int discounted_upgrade_cost(int base_cost) const;
    int discounted_market_buy_price(int base_price) const;
    int monster_food_sell_price(int base_price) const;
    int monster_food_success_chance() const;
    bool protects_monster_food_failure() const;
    bool should_refund_seed() const;
    bool should_skip_harvest_stamina() const;
    bool should_double_livestock_product() const;
    bool should_save_cooking_ingredient() const;
    bool should_trigger_starry_night() const;

private:
    int x_;
    int y_;
    int money_;
    int stamina_;
    int day_;
    int hoe_grade_;
    int rod_grade_;
    int pick_grade_;
    int storage_level_;
    char facing_;
    int stamina_limit_;
    bool sick_today_;
    bool wildlife_today_;
    int starry_night_days_;
    int skill_points_;
    bool str_nodes_[7];
    bool int_nodes_[9];
    bool luk_nodes_[7];
    bool sold_monster_food_;
    bool made_monster_food_;   // 처음으로 괴식을 만든 적 있는지
    bool trust_ever_dropped_;  // 신용도가 최고(71+) 에서 내려간 적 있는지
    int shop_trust_;
    int daily_sales_income_;
    int pending_dividend_;
    std::string levelup_message_;

    LevelSystem level_system_;
    Inventory<ItemType> inventory_;
    std::string name_;
};
