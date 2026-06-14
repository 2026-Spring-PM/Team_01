#pragma once

#include "Position.h"
#include <string>

// Abstract base class for objects that have a tile position and can be updated.
// Player and moving barn animals both override these virtual functions.
class GameObject {
public:
    virtual ~GameObject() = default;

    virtual void update() = 0;
    virtual Position get_position() const = 0;
    virtual void set_position(const Position& position) = 0;
    virtual std::string debug_name() const = 0;
};
