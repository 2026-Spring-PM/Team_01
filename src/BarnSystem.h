#pragma once

#include "Player.h"
#include "Tile.h"
#include <string>
#include <vector>

struct BarnAnimal {
    // x/y are logical target tile coordinates. visual_x/visual_y are used for smooth GUI movement.
    int x = 0;
    int y = 0;
    double visual_x = 0.0;
    double visual_y = 0.0;
    char facing = 'd'; // f: front/down, b: back/up, l, r
    Tile tile;
};

class BarnSystem {
public:
    static constexpr int WIDTH = 12;
    static constexpr int HEIGHT = 12;
    static constexpr int EXIT_X = 1;
    static constexpr int EXIT_Y = 10;

    bool is_inside(int x, int y) const;
    bool is_walkable(int x, int y, const Player& player) const;
    bool has_animal_at(int x, int y) const;
    BarnAnimal* animal_at(int x, int y);
    const BarnAnimal* animal_at(int x, int y) const;
    BarnAnimal* nearest_adjacent_animal(const Player& player);
    BarnAnimal* front_animal(const Player& player);
    const BarnAnimal* front_animal(const Player& player) const;
    std::string describe_animal(const BarnAnimal& animal) const;
    const std::vector<BarnAnimal>& animals() const { return animals_; }

    int max_animals() const;
    bool place_animal(Player& player, int animal_type, std::string& message);
    bool hatch_egg(Player& player, std::string& message);
    bool collect_nearby_product(Player& player, std::string& message);
    bool remove_nearby_animal(Player& player, std::string& message);
    bool collect_front_product(Player& player, std::string& message);
    bool remove_front_animal(Player& player, std::string& message);
    void advance_day();
    void update_random_movement(const Player& player);
    void update_smooth_positions();

private:
    std::vector<BarnAnimal> animals_;
    int movement_tick_ = 0;

    bool find_empty_spawn(int& x, int& y) const;
    bool is_blocking_object(int x, int y) const;
};
