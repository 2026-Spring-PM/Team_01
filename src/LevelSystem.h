#pragma once

class LevelSystem {
public:
    static constexpr int MAX_LEVEL = 30;
    static constexpr int INITIAL_STAMINA = 10;

    LevelSystem();

    bool add_exp(int exp);

    int get_level() const { return level_; }
    int get_exp() const { return exp_; }
    int get_exp_to_next_level() const;
    int get_max_stamina() const { return max_stamina_; }

    static int required_exp_for_level(int target_level);

    // 보너스 레벨업 체크 (Lv 6,10,16,20,26,30)
    static bool is_bonus_level(int level);

private:
    int level_;
    int exp_;
    int max_stamina_;

    void on_level_up();
};
