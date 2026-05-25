#ifndef KITCHEN_HPP
#define KITCHEN_HPP

class Player;

class Kitchen {
private:
    int kitchenLevel;

public:
    Kitchen() : kitchenLevel(1) {}
    
    void interact(Player& player);
};

#endif // KITCHEN_HPP