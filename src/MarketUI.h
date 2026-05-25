#pragma once
#include "raylib.h"
#include "MarketSystem.h"

enum GameScreen { OUTSIDE_MAP, PIERRE_SHOP_UI, LEWIS_SHOP_UI };

class MarketUI {
private:
    // Backend Data
    Font myCustomFont;
    Player mainPlayer;
    PierreShop pierre;
    LewisShop lewis;
    int currentDay;

    // Visual/Camera Data
    Camera2D camera;
    Vector2 playerPos;
    GameScreen currentScreen;
    
    // Settings
    float speed;
    int playerRadius;
    float mapSize;
    int selectedSlot;
    const int totalSlots = 9;

    // Hitboxes
    Rectangle pierreDoor;
    Rectangle lewisDoor;
    Rectangle buyPotatoBtn;
    Rectangle sellPotatoBtn;
    Rectangle cheatGrowBtn;

public:
    MarketUI();
    ~MarketUI();
    void run();
};