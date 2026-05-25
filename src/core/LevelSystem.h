#pragma once

// 레벨 및 경험치 시스템
// 초기 스태미나 150, 레벨업마다 +20
// 9-10, 19-20, 29-30 돌파 시 +50 보너스
// Lv.30 만렙, 이후 기록 갱신용
class LevelSystem {
public:
    static constexpr int MAX_LEVEL = 30;
    static constexpr int INITIAL_STAMINA = 150;

    LevelSystem();

    // 경험치 추가, 레벨업 시 true 반환
    // 한 번에 여러 레벨업도 처리
    bool add_exp(int exp);

    // 게터
    int get_level() const { return level_; }
    int get_exp() const { return exp_; }
    int get_exp_to_next_level() const;
    int get_max_stamina() const { return max_stamina_; }

    // 다음 레벨까지 필요한 누적 EXP (게임 디자인 v2 표 기반)
    static int required_exp_for_level(int target_level);

private:
    int level_;
    int exp_;
    int max_stamina_;

    // 레벨업 시 호출됨 (max_stamina 갱신)
    void on_level_up();
};