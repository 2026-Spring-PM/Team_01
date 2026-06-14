#pragma once

#include <string>
#include <vector>

class Player;

class Tutorial {
public:
    Tutorial();

    // 튜토리얼이 아직 완료되지 않았으면 true
    bool is_active() const { return !completed_; }

    // 튜토리얼이 이미 끝났으면 true
    bool is_completed() const { return completed_; }

    // 현재 단계의 대사를 반환 (터미널용)
    std::string current_dialogue() const;

    // 현재 단계의 제목 (GUI 팝업 타이틀용)
    std::string current_title() const;

    // 다음 단계로 진행, 마지막이면 completed_를 true로
    void advance();

    // 총 단계 수
    int total_steps() const;

    // 현재 단계 번호 (0-based)
    int current_step() const { return step_; }

    // NPC 이름
    static const char* npc_name();

private:
    struct DialogueLine {
        std::string title;
        std::string text;
    };

    int step_;
    bool completed_;
    std::vector<DialogueLine> dialogues_;

    void build_dialogues();
};
