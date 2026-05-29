#pragma once

#include <map>
#include <vector>
#include <utility>

template <typename T>
class Inventory {
public:
    explicit Inventory(int max_slots) : max_slots_(max_slots) {}

    bool add(const T& item, int amount = 1) {
        if (amount <= 0) return false;
        // v9: inventory capacity is based on total item count, not unique item types.
        if (used_slots() + amount > max_slots_) return false;
        items_[item] += amount;
        return true;
    }

    bool remove(const T& item, int amount = 1) {
        if (amount <= 0) return false;
        auto it = items_.find(item);
        if (it == items_.end() || it->second < amount) return false;
        it->second -= amount;
        if (it->second == 0) items_.erase(it);
        return true;
    }

    int count(const T& item) const {
        auto it = items_.find(item);
        return it == items_.end() ? 0 : it->second;
    }

    int used_slots() const {
        int total = 0;
        for (const auto& pair : items_) total += pair.second;
        return total;
    }
    int unique_item_count() const { return static_cast<int>(items_.size()); }
    int max_slots() const { return max_slots_; }
    int free_slots() const { return max_slots_ - used_slots(); }
    bool can_add(int amount = 1) const { return amount > 0 && used_slots() + amount <= max_slots_; }
    bool is_full() const { return used_slots() >= max_slots_; }
    void expand(int additional) { if (additional > 0) max_slots_ += additional; }

    std::vector<std::pair<T, int>> get_all() const {
        std::vector<std::pair<T, int>> result;
        for (const auto& pair : items_) result.push_back(pair);
        return result;
    }

private:
    std::map<T, int> items_;
    int max_slots_;
};
