#ifndef WORKSHOP_HPP
#define WORKSHOP_HPP

class Player;

class Workshop {
private:
    int workshopLevel;

public:
    Workshop() : workshopLevel(0) {}
    
    // J키를 눌렀을 때 실행될 상호작용 UI
    void interact(Player& player);
};

#endif // WORKSHOP_HPP