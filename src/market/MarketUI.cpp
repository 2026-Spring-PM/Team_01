#include "MarketUI.h"

// ==========================================
// 1. CONSTRUCTOR: Setup the game world
// ==========================================
MarketUI::MarketUI() : mainPlayer(500) { 
    InitWindow(800, 600, "Farm Village - Custom Fonts Enabled");
    SetTargetFPS(60);

    // --- LOAD THE FONT ---
    // Make sure "retro_font.ttf" is in the same folder as this file!
    // If it is missing, Raylib will safely fallback to the default font.
    myCustomFont = LoadFontEx("hellokittyfont.ttf", 64, 0, 250);

    // Setup Game Variables
    currentDay = 1;
    currentScreen = OUTSIDE_MAP; 
    playerPos = { 400.0f, 300.0f };
    speed = 5.0f;
    playerRadius = 25; 
    mapSize = 1200.0f;
    selectedSlot = 0;

    // Setup Camera
    camera = { 0 };
    camera.offset = { 800 / 2.0f, 600 / 2.0f };
    camera.rotation = 0.0f; 
    camera.zoom = 1.0f;

    // Setup Interactive Hitboxes
    pierreDoor = { 200, 150, 100, 100 }; 
    lewisDoor = { 600, 150, 100, 100 };  
    buyPotatoBtn = { 200, 200, 400, 60 };
    sellPotatoBtn = { 200, 200, 400, 60 };
    cheatGrowBtn = { 200, 300, 400, 60 };
}

// ==========================================
// 2. DESTRUCTOR: Clean up when closed
// ==========================================
MarketUI::~MarketUI() {
    UnloadFont(myCustomFont); // Clean up font memory!
    CloseWindow();
}

// ==========================================
// 3. MAIN GAME LOOP: Where the magic happens
// ==========================================
void MarketUI::run() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    while (!WindowShouldClose()) {
        
        // --- UPDATE PHASE (Logic & Inputs) ---
        Vector2 mousePoint = GetMousePosition();
        bool isClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        if (currentScreen == OUTSIDE_MAP) {
            // Player Movement & Boundaries
            if (IsKeyDown(KEY_RIGHT)) playerPos.x += speed;
            if (IsKeyDown(KEY_LEFT)) playerPos.x -= speed;
            if (IsKeyDown(KEY_DOWN)) playerPos.y += speed;
            if (IsKeyDown(KEY_UP)) playerPos.y -= speed;

            if (playerPos.x < playerRadius) playerPos.x = playerRadius;
            if (playerPos.y < playerRadius) playerPos.y = playerRadius;
            if (playerPos.x > mapSize - playerRadius) playerPos.x = mapSize - playerRadius;
            if (playerPos.y > mapSize - playerRadius) playerPos.y = mapSize - playerRadius;

            camera.target = playerPos;

            // Hotbar scrolling
            float wheel = GetMouseWheelMove();
            if (wheel != 0) {
                selectedSlot -= (int)wheel;
                if (selectedSlot < 0) selectedSlot = 8;
                if (selectedSlot > 8) selectedSlot = 0;
            }

            // Enter Shops
            if (CheckCollisionCircleRec(playerPos, playerRadius, pierreDoor) && IsKeyPressed(KEY_SPACE)) {
                currentScreen = PIERRE_SHOP_UI; 
            }
            if (CheckCollisionCircleRec(playerPos, playerRadius, lewisDoor) && IsKeyPressed(KEY_SPACE)) {
                currentScreen = LEWIS_SHOP_UI; 
            }
        } 
        else if (currentScreen == PIERRE_SHOP_UI) {
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_SPACE)) currentScreen = OUTSIDE_MAP; 
            
            if (CheckCollisionPointRec(mousePoint, buyPotatoBtn) && isClicked) {
                pierre.buyItem(mainPlayer, 0, 1); 
            }
        }
        else if (currentScreen == LEWIS_SHOP_UI) {
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_SPACE)) currentScreen = OUTSIDE_MAP; 
            
            if (CheckCollisionPointRec(mousePoint, sellPotatoBtn) && isClicked) {
                lewis.sellItem(mainPlayer, ITEM_POTATO, 1); 
            }

            if (CheckCollisionPointRec(mousePoint, cheatGrowBtn) && isClicked) {
                int seedsWeHave = mainPlayer.inventory.getQuantity(ITEM_POTATO_SEED);
                if (seedsWeHave > 0) {
                    mainPlayer.inventory.removeItem(ITEM_POTATO_SEED, seedsWeHave);
                    mainPlayer.inventory.addItem(ITEM_POTATO, seedsWeHave * 3); 
                }
            }
        }

        // --- DRAW PHASE (Rendering) ---
        BeginDrawing();
        ClearBackground(BLACK); 

        // 1. OUTSIDE MAP
        if (currentScreen == OUTSIDE_MAP) {
            BeginMode2D(camera);
                DrawRectangle(0, 0, mapSize, mapSize, { 143, 196, 96, 255 }); 
                
                for (int i = 0; i < mapSize; i += 100) {
                    DrawLine(i, 0, i, mapSize, { 120, 170, 80, 255 }); 
                    DrawLine(0, i, mapSize, i, { 120, 170, 80, 255 }); 
                }

                DrawRectangleRec(pierreDoor, BLUE);
                DrawTextEx(myCustomFont, "PIERRE", { pierreDoor.x + 15.0f, pierreDoor.y + 40.0f }, 20, 1.0f, WHITE);
                
                DrawRectangleRec(lewisDoor, ORANGE);
                DrawTextEx(myCustomFont, "LEWIS", { lewisDoor.x + 15.0f, lewisDoor.y + 40.0f }, 20, 1.0f, BLACK);

                DrawCircle(playerPos.x, playerPos.y, playerRadius, RED);
            EndMode2D(); 
        } 
        // 2. PIERRE'S SHOP
        else if (currentScreen == PIERRE_SHOP_UI) {
            ClearBackground(DARKBLUE);
            DrawTextEx(myCustomFont, "PIERRE'S SEED SHOP", { 200.0f, 80.0f }, 40, 1.0f, WHITE);
            DrawTextEx(myCustomFont, "Press ESC to exit", { 300.0f, 130.0f }, 20, 1.0f, LIGHTGRAY);
            
            DrawRectangleRec(buyPotatoBtn, LIGHTGRAY);
            if (CheckCollisionPointRec(mousePoint, buyPotatoBtn)) DrawRectangleLinesEx(buyPotatoBtn, 3, GOLD); 
            DrawTextEx(myCustomFont, "Buy Potato Seed (5G)", { buyPotatoBtn.x + 70.0f, buyPotatoBtn.y + 15.0f }, 24, 1.0f, BLACK);
        }
        // 3. LEWIS'S SHOP
        else if (currentScreen == LEWIS_SHOP_UI) {
            ClearBackground(ORANGE);
            DrawTextEx(myCustomFont, "LEWIS'S PRODUCE EXCHANGE", { 120.0f, 80.0f }, 40, 1.0f, BLACK);
            DrawTextEx(myCustomFont, "Press ESC to exit", { 320.0f, 130.0f }, 20, 1.0f, DARKGRAY);
            
            DrawRectangleRec(sellPotatoBtn, LIGHTGRAY);
            if (CheckCollisionPointRec(mousePoint, sellPotatoBtn)) DrawRectangleLinesEx(sellPotatoBtn, 3, RED);
            DrawTextEx(myCustomFont, "Sell Potato (+5G)", { sellPotatoBtn.x + 90.0f, sellPotatoBtn.y + 15.0f }, 24, 1.0f, BLACK);

            DrawRectangleRec(cheatGrowBtn, DARKGREEN);
            if (CheckCollisionPointRec(mousePoint, cheatGrowBtn)) DrawRectangleLinesEx(cheatGrowBtn, 3, GOLD);
            DrawTextEx(myCustomFont, "[TEST] Grow All Seeds into Potatoes", { cheatGrowBtn.x + 20.0f, cheatGrowBtn.y + 15.0f }, 20, 1.0f, WHITE);
        }

        // --- GLOBAL HUD ---
        float hudX = screenWidth - 170.0f;
        float hudY = 20.0f;
        DrawRectangle((int)hudX, (int)hudY, 150, 75, Fade(BROWN, 0.9f));
        DrawRectangleLinesEx({hudX, hudY, 150.0f, 75.0f}, 4, DARKBROWN);
        
        DrawTextEx(myCustomFont, TextFormat("Day %d", currentDay), { hudX + 15.0f, hudY + 15.0f }, 20, 1.0f, RAYWHITE);
        DrawTextEx(myCustomFont, TextFormat("%d G", mainPlayer.getGold()), { hudX + 15.0f, hudY + 40.0f }, 24, 1.0f, GOLD);

        DrawTextEx(myCustomFont, TextFormat("Seeds: %d", mainPlayer.inventory.getQuantity(ITEM_POTATO_SEED)), { 20.0f, 20.0f }, 20, 1.0f, WHITE);
        DrawTextEx(myCustomFont, TextFormat("Crops: %d", mainPlayer.inventory.getQuantity(ITEM_POTATO)), { 20.0f, 50.0f }, 20, 1.0f, WHITE);

        EndDrawing();
    }
}