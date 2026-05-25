#ifndef LIVESTOCK_HPP
#define LIVESTOCK_HPP

#include <string>
#include <utility>

class Player;

enum class ProductType {
    None,
    Egg,
    Milk,
    Pork
};

class Livestock {
protected:
    std::string name;
    int age;
    int adultAge;
    bool isSlaughtered;
    bool hasProducedToday; // 🌟 추가됨: 오늘 상호작용을 이미 했는지 확인

public:
    Livestock(std::string n, int aAge);
    virtual ~Livestock();

    std::string getName() const;
    bool getIsAdult() const;
    bool getIsSlaughtered() const;
    
    // 🌟 추가됨: 상호작용 상태 확인용 Getter/Setter
    bool getHasProducedToday() const;     
    void setHasProducedToday(bool state); 

    virtual void grow();
    virtual std::pair<ProductType, int> produce(Player& player) = 0;
};

class Chicken : public Livestock {
public:
    Chicken();
    std::pair<ProductType, int> produce(Player& player) override;
};

class Pig : public Livestock {
public:
    Pig();
    std::pair<ProductType, int> produce(Player& player) override;
};

class Cow : public Livestock {
public:
    Cow();
    std::pair<ProductType, int> produce(Player& player) override;
};

#endif // LIVESTOCK_HPP