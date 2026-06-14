#pragma once

class GameObject {
public:
    virtual ~GameObject() = default;
    
    virtual void update() = 0;       
};
