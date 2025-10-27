#include "raylib.h"
#include "raymath.h"
#include <iostream>

float windowPosX = 0;
float windowPosY = 0;
int windowSize = 64;

float thinkingTimer = 5;
int runningSpd = 175;
int walkingSpd = 75;

int targetX;
int targetY;

enum Modes
{
    thinking = 0,
    running = 1,
    walking = 2,
    chasing = 3
} modes;

void InitPet()
{
    SetConfigFlags(FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_TOPMOST | FLAG_WINDOW_UNDECORATED);
    InitWindow(64, 64, "");
    SetTargetFPS(60);

    Vector2 pos = GetWindowPosition();
    windowPosX = pos.x;
    windowPosY = pos.y;

    modes = thinking;
    thinkingTimer = 1;
}

void Thinking()
{
    thinkingTimer -= GetFrameTime();
    if (thinkingTimer <= 0)
    {
        int random = GetRandomValue(1, 2);
        int monitor = GetCurrentMonitor();
        targetX = GetRandomValue(0, GetMonitorWidth(monitor) - windowSize);
        targetY = GetRandomValue(0, GetMonitorHeight(monitor) - windowSize);
        if (random == 1) { modes = running; }
        if (random == 2) { modes = walking; }
    }
}

void Moving(int speed, float deltaTime)
{
    int monitor = GetCurrentMonitor();
    int maxX = GetMonitorWidth(monitor) - windowSize;
    int maxY = GetMonitorHeight(monitor) - windowSize;

    // Calculate direction vector
    Vector2 direction = { targetX - windowPosX, targetY - windowPosY };
    float distance = Vector2Length(direction);
    
    if (distance > 0)
    {
        // Normalize direction and calculate movement
        direction = Vector2Normalize(direction);
        float moveDistance = speed * deltaTime;
        
        // Don't overshoot the target
        if (moveDistance > distance)
        {
            moveDistance = distance;
        }
        
        windowPosX += direction.x * moveDistance;
        windowPosY += direction.y * moveDistance;
    }

    // Clamp position to monitor bounds
    windowPosX = Clamp(windowPosX, 0, maxX);
    windowPosY = Clamp(windowPosY, 0, maxY);

    // Check if we've reached the target (with some tolerance)
    if (Vector2Distance({windowPosX, windowPosY}, {float(targetX), float(targetY)}) < 1.0f)
    {
        windowPosX = targetX;
        windowPosY = targetY;
        modes = thinking;
        thinkingTimer = 5;
    }

    SetWindowPosition(int(round(windowPosX)), int(round(windowPosY)));
}

void UpdatePet(float deltaTime)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){ modes = chasing; } 

    switch (modes)
    {
    case thinking:
        Thinking();
        break;
    case running:
        Moving(runningSpd, deltaTime);
        break;
    case walking:
        Moving(walkingSpd, deltaTime);
        break;
    case chasing:
        Vector2 dir = Vector2Normalize(Vector2Subtract(GetMousePosition(), {windowPosX,windowPosY}));
        windowPosX += dir.x * runningSpd * deltaTime;
        windowPosY += dir.y * runningSpd * deltaTime;

        if (CheckCollisionPointRec(GetMousePosition(),{windowPosX,windowPosY,(float)windowSize,(float)windowSize}))
        {
            SetMousePosition(windowPosX + windowSize / 2, windowPosY + windowSize / 2);
            
            Moving(runningSpd,deltaTime);
            if (modes == thinking)
            {SetMousePosition(GetMousePosition().x,GetMousePosition().y);}
        }
        break;
    }
}

void DrawPet()
{
    BeginDrawing();
    ClearBackground({230,22,3,255});
    EndDrawing();
}

int main(void)
{
    InitPet();
    
    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();
        UpdatePet(deltaTime);
        DrawPet();
    }
    CloseWindow();

    return 0;
}
