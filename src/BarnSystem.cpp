#include "BarnSystem.h"
#include "LivestockSystem.h"

#include <algorithm>
#include <cstdlib>
#include <cmath>

bool BarnSystem::is_inside(int x, int y) const {
    return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT;
}

bool BarnSystem::has_animal_at(int x, int y) const {
    return animal_at(x, y) != nullptr;
}


BarnAnimal* BarnSystem::animal_at(int x, int y) {
    for (auto& a : animals_) {
        if (a.x == x && a.y == y) return &a;
    }
    return nullptr;
}

const BarnAnimal* BarnSystem::animal_at(int x, int y) const {
    for (const auto& a : animals_) {
        if (a.x == x && a.y == y) return &a;
    }
    return nullptr;
}

bool BarnSystem::is_blocking_object(int x, int y) const {
    // Barn objects are solid. The layout intentionally leaves a clear walking route:
    // EXIT(1,10) -> (2,10) -> (2,9) -> central open pen x=3..8, y=4..7.
    // Keep this table in sync with GuiRenderer::draw_barn().
    static const int blocked[][2] = {
        // feed / water troughs near the top wall
        {3,2},{4,2},{5,2},
        {6,2},{7,2},{8,2},

        // side hay props
        {1,2},{1,3},
        {10,2},{10,3},

        // side and lower pen fences
        {2,5},{2,6},
        {9,5},{9,6},
        {4,8},{5,8},{6,8},{7,8},

        // lower storage props, kept away from the exit route
        {10,8},
        {9,9},{7,9},{8,9},{10,9}
    };
    for (const auto& p : blocked) {
        if (p[0] == x && p[1] == y) return true;
    }
    return false;
}

bool BarnSystem::is_walkable(int x, int y, const Player& player) const {
    if (!is_inside(x, y)) return false;
    if (x == 0 || y == 0 || x == WIDTH - 1 || y == HEIGHT - 1) {
        return x == EXIT_X && y == EXIT_Y;
    }
    if (is_blocking_object(x, y)) return false;
    if (has_animal_at(x, y)) return false;
    (void)player;
    return true;
}

int BarnSystem::max_animals() const {
    return 8;
}

bool BarnSystem::find_empty_spawn(int& x, int& y) const {
    const int starts[][2] = {{5,5},{6,5},{5,6},{6,6},{4,5},{7,5},{4,6},{7,6},{3,4},{8,4},{3,7},{8,7}};
    for (const auto& p : starts) {
        if (!has_animal_at(p[0], p[1]) && !is_blocking_object(p[0], p[1]) && !(p[0] == EXIT_X && p[1] == EXIT_Y)) { x = p[0]; y = p[1]; return true; }
    }
    for (int yy = 1; yy < HEIGHT - 1; ++yy) {
        for (int xx = 1; xx < WIDTH - 1; ++xx) {
            if (!has_animal_at(xx, yy) && !is_blocking_object(xx, yy) && !(xx == EXIT_X && yy == EXIT_Y)) { x = xx; y = yy; return true; }
        }
    }
    return false;
}

bool BarnSystem::place_animal(Player& player, int animal_type, std::string& message) {
    if (static_cast<int>(animals_.size()) >= max_animals()) {
        message = "축사가 가득 찼습니다.";
        return false;
    }
    int sx = 0, sy = 0;
    if (!find_empty_spawn(sx, sy)) {
        message = "가축을 배치할 빈 공간이 없습니다.";
        return false;
    }

    Tile tmp;
    tmp.type = TileType::LIVESTOCK;
    if (!LivestockSystem::place_animal(player, tmp, animal_type, message)) return false;

    BarnAnimal a;
    a.x = sx;
    a.y = sy;
    a.visual_x = sx;
    a.visual_y = sy;
    a.facing = 'f';
    a.tile = tmp;
    animals_.push_back(a);
    message += " 축사 내부에 배치되었습니다.";
    return true;
}

bool BarnSystem::hatch_egg(Player& player, std::string& message) {
    if (static_cast<int>(animals_.size()) >= max_animals()) {
        message = "축사가 가득 차서 부화할 수 없습니다.";
        return false;
    }
    int before = player.count_item(ItemType::BABY_CHICKEN);
    if (!LivestockSystem::hatch_egg(player, message)) return false;
    int after = player.count_item(ItemType::BABY_CHICKEN);
    if (after > before) {
        // 부화 성공으로 생긴 새끼 닭을 바로 축사에 배치한다. 배치 스태미나가 중복 차감되지 않도록 직접 생성한다.
        player.remove_item(ItemType::BABY_CHICKEN, 1);
        int sx = 0, sy = 0;
        if (!find_empty_spawn(sx, sy)) return true;
        Tile t;
        t.type = TileType::LIVESTOCK;
        t.is_occupied = true;
        t.animal_placed = true;
        t.animal_type = 1;
        const LivestockInfo* info = LivestockSystem::get_info(1);
        t.animal_adult_age = info ? info->adult_age : 3;
        BarnAnimal a;
        a.x = sx;
        a.y = sy;
        a.visual_x = sx;
        a.visual_y = sy;
        a.facing = 'f';
        a.tile = t;
        animals_.push_back(a);
        message += " 축사 안에 바로 배치되었습니다.";
    }
    return true;
}

BarnAnimal* BarnSystem::nearest_adjacent_animal(const Player& player) {
    BarnAnimal* best = nullptr;
    int best_dist = 999;
    for (auto& a : animals_) {
        int dist = std::abs(a.x - player.get_x()) + std::abs(a.y - player.get_y());
        if (dist <= 1 && dist < best_dist) {
            best = &a;
            best_dist = dist;
        }
    }
    return best;
}


static void front_delta_from_facing(char facing, int& dx, int& dy) {
    dx = 0;
    dy = 0;
    if (facing == 'b') dy = -1;
    else if (facing == 'f') dy = 1;
    else if (facing == 'l') dx = -1;
    else if (facing == 'r') dx = 1;
    else dy = 1;
}

BarnAnimal* BarnSystem::front_animal(const Player& player) {
    int dx = 0, dy = 0;
    front_delta_from_facing(player.get_facing(), dx, dy);
    return animal_at(player.get_x() + dx, player.get_y() + dy);
}

const BarnAnimal* BarnSystem::front_animal(const Player& player) const {
    int dx = 0, dy = 0;
    front_delta_from_facing(player.get_facing(), dx, dy);
    return animal_at(player.get_x() + dx, player.get_y() + dy);
}

std::string BarnSystem::describe_animal(const BarnAnimal& animal) const {
    const LivestockInfo* info = LivestockSystem::get_info(animal.tile.animal_type);
    const std::string name = info ? info->name : "가축";
    const bool adult = info && animal.tile.animal_age >= info->adult_age;
    std::string status = name + " 상태\n";
    status += "위치: (" + std::to_string(animal.x + 1) + ", " + std::to_string(animal.y + 1) + ")\n";
    if (info) {
        status += "나이: " + std::to_string(animal.tile.animal_age) + "일 / 성체 기준 " + std::to_string(info->adult_age) + "일\n";
        status += std::string("성장 상태: ") + (adult ? "성체" : "새끼") + "\n";
        if (adult) {
            status += "생산물: ";
            status += item_name(info->product_item);
            status += " " + std::to_string(info->product_amount) + "개\n";
            status += std::string("오늘 생산: ") + (animal.tile.animal_produced_today ? "완료" : "가능") + "\n";
        } else {
            int remain = info->adult_age - animal.tile.animal_age;
            if (remain < 0) remain = 0;
            status += "성체까지 남은 일수: " + std::to_string(remain) + "일\n";
        }
    }
    return status;
}

bool BarnSystem::collect_nearby_product(Player& player, std::string& message) {
    BarnAnimal* a = nearest_adjacent_animal(player);
    if (!a) {
        message = "가축 바로 옆에서만 상호작용할 수 있습니다.";
        return false;
    }
    const int ax = a->x;
    const int ay = a->y;
    bool ok = LivestockSystem::collect_product(player, a->tile, message);
    if (ok && !a->tile.has_animal()) {
        animals_.erase(std::remove_if(animals_.begin(), animals_.end(), [&](const BarnAnimal& other) { return other.x == ax && other.y == ay; }), animals_.end());
    }
    return ok;
}

bool BarnSystem::remove_nearby_animal(Player& player, std::string& message) {
    BarnAnimal* a = nearest_adjacent_animal(player);
    if (!a) {
        message = "제거할 가축 옆에 서야 합니다.";
        return false;
    }
    const int ax = a->x;
    const int ay = a->y;
    LivestockSystem::remove_animal(a->tile, message);
    animals_.erase(std::remove_if(animals_.begin(), animals_.end(), [&](const BarnAnimal& other) { return other.x == ax && other.y == ay; }), animals_.end());
    return true;
}


bool BarnSystem::collect_front_product(Player& player, std::string& message) {
    BarnAnimal* a = front_animal(player);
    if (!a) {
        message = "바라보는 방향 1칸 앞에 가축이 없습니다.";
        return false;
    }
    const int ax = a->x;
    const int ay = a->y;
    bool ok = LivestockSystem::collect_product(player, a->tile, message);
    if (ok && !a->tile.has_animal()) {
        animals_.erase(std::remove_if(animals_.begin(), animals_.end(), [&](const BarnAnimal& other) { return other.x == ax && other.y == ay; }), animals_.end());
    }
    return ok;
}

bool BarnSystem::remove_front_animal(Player& player, std::string& message) {
    BarnAnimal* a = front_animal(player);
    if (!a) {
        message = "바라보는 방향 1칸 앞에 제거할 가축이 없습니다.";
        return false;
    }
    const int ax = a->x;
    const int ay = a->y;
    LivestockSystem::remove_animal(a->tile, message);
    animals_.erase(std::remove_if(animals_.begin(), animals_.end(), [&](const BarnAnimal& other) { return other.x == ax && other.y == ay; }), animals_.end());
    return true;
}

void BarnSystem::advance_day() {
    for (auto& a : animals_) LivestockSystem::grow_animal(a.tile);
}

void BarnSystem::update_smooth_positions() {
    const double step = 0.045;
    for (auto& a : animals_) {
        auto move_towards = [step](double current, double target) {
            double diff = target - current;
            if (std::abs(diff) <= step) return target;
            return current + (diff > 0.0 ? step : -step);
        };
        a.visual_x = move_towards(a.visual_x, static_cast<double>(a.x));
        a.visual_y = move_towards(a.visual_y, static_cast<double>(a.y));
    }
}

void BarnSystem::update_random_movement(const Player& player) {
    update_smooth_positions();

    ++movement_tick_;
    if (movement_tick_ < 180) return;
    movement_tick_ = 0;
    if (animals_.empty()) return;

    int idx = std::rand() % static_cast<int>(animals_.size());
    BarnAnimal& a = animals_[idx];

    // If an animal is still visually walking toward its target, do not retarget it yet.
    if (std::abs(a.visual_x - a.x) > 0.05 || std::abs(a.visual_y - a.y) > 0.05) return;
    if (std::abs(a.x - player.get_x()) + std::abs(a.y - player.get_y()) <= 1) return;

    const int dirs[4][3] = {{1,0,'r'},{-1,0,'l'},{0,1,'f'},{0,-1,'b'}};
    int start = std::rand() % 4;
    for (int i = 0; i < 4; ++i) {
        int d = (start + i) % 4;
        int nx = a.x + dirs[d][0];
        int ny = a.y + dirs[d][1];
        if (!is_inside(nx, ny)) continue;
        // Keep animals in the open pen floor, not on wall/exit/decorative outer tiles.
        if (nx <= 1 || ny <= 1 || nx >= WIDTH - 2 || ny >= HEIGHT - 2) continue;
        if (is_blocking_object(nx, ny)) continue;
        if (nx == player.get_x() && ny == player.get_y()) continue;
        if (has_animal_at(nx, ny)) continue;
        a.x = nx;
        a.y = ny;
        a.facing = static_cast<char>(dirs[d][2]);
        return;
    }
}
