#pragma once

#include <string>
#include <vector>

class Tutorial {
public:
    Tutorial();

    bool is_active() const { return !completed_; }
    bool is_completed() const { return completed_; }
    std::string current_title() const;
    std::string current_dialogue() const;
    void advance();
    int total_steps() const;
    int current_step() const { return step_; }

    static const char* npc_name();

private:
    struct DialogueLine {
        std::string title;
        std::string text;
    };

    int step_ = 0;
    bool completed_ = false;
    std::vector<DialogueLine> dialogues_;

    void build_dialogues();
};
