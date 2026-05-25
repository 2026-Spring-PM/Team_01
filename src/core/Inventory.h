#pragma once

#include <map>
#include <vector>
#include <utility>

// 인벤토리 템플릿 클래스 (C++ 필수요소: Template)
// 아이템 종류(T)별로 수량을 관리
// 최대 슬롯 제한, 레벨업 시 확장 가능
//
// 사용 예시:
//   Inventory<int> inv(20);              // 슬롯 20개
//   inv.add(101, 5);                     // 아이템 101을 5개 추가
//   int count = inv.count(101);          // 보유 수량 확인
//   inv.remove(101, 2);                  // 2개 차감
template <typename T>
class Inventory {
public:
    // 생성자: 최대 슬롯 수 지정
    explicit Inventory(int max_slots) : max_slots_(max_slots) {}

    // 아이템 추가
    // 반환값: 성공 여부 (가득 차서 실패하면 false)
    bool add(const T& item, int amount = 1) {
        // 이미 보유 중이면 그냥 수량 증가
        auto it = items_.find(item);
        if (it != items_.end()) {
            it->second += amount;
            return true;
        }

        // 새 슬롯 필요, 슬롯 부족하면 실패
        if (static_cast<int>(items_.size()) >= max_slots_) {
            return false;
        }

        items_[item] = amount;
        return true;
    }

    // 아이템 차감
    // 반환값: 성공 여부 (보유량 부족하면 false)
    bool remove(const T& item, int amount = 1) {
        auto it = items_.find(item);
        if (it == items_.end() || it->second < amount) {
            return false;
        }

        it->second -= amount;
        if (it->second == 0) {
            items_.erase(it);  // 0개면 슬롯에서 제거
        }
        return true;
    }

    // 보유 수량 확인
    int count(const T& item) const {
        auto it = items_.find(item);
        return (it != items_.end()) ? it->second : 0;
    }

    // 사용된 슬롯 수
    int used_slots() const { return static_cast<int>(items_.size()); }

    // 최대 슬롯 수
    int max_slots() const { return max_slots_; }

    // 가득 찼는지
    bool is_full() const { return used_slots() >= max_slots_; }

    // 슬롯 확장 (레벨업 시 사용)
    void expand(int additional) { max_slots_ += additional; }

    // 전체 아이템 목록 (화면 출력용)
    // 반환값: (아이템, 수량) 쌍의 벡터
    std::vector<std::pair<T, int>> get_all() const {
        std::vector<std::pair<T, int>> result;
        for (const auto& [item, amount] : items_) {
            result.emplace_back(item, amount);
        }
        return result;
    }

private:
    std::map<T, int> items_;  // 아이템 → 수량
    int max_slots_;           // 최대 슬롯 수
};