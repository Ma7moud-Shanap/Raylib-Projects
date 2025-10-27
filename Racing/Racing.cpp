#include "raylib.h"
#include "Player.h"
#include "Camera.h"

const int screenWidth = 800;
const int screenHeight = 600;

void InitGame()
{
    InitWindow(screenWidth, screenHeight, "Racing Game");
    SetTargetFPS(60);
}

void CloseGame(RenderTexture& screenCamera1, RenderTexture& screenCamera2) {
    UnloadRenderTexture(screenCamera1);
    UnloadRenderTexture(screenCamera2);
    CloseWindow();
}

int main()
{
    InitGame();

    // Player 1 uses W, S, A, D
    Player player1(screenWidth / 2, screenHeight / 2, KEY_W, KEY_S, KEY_A, KEY_D);

    // Player 2 uses Up, Down, Left, Right Arrows
    Player player2(screenWidth / 2 + 50, screenHeight / 2, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT);

    Cam camera1;
    Cam camera2;
    Cam map1;
    Cam map2;

    RenderTexture screenCamera1 = LoadRenderTexture(screenWidth / 2, screenHeight);
    RenderTexture screenCamera2 = LoadRenderTexture(screenWidth / 2, screenHeight);

    Rectangle splitScreenRect = { 0.0f, 0.0f, (float)screenCamera1.texture.width, -(float)screenCamera1.texture.height };

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        // Update
        player1.Update(deltaTime);
        player2.Update(deltaTime);

        camera1.Update(player1);
        camera2.Update(player2);

        // Draw
        BeginTextureMode(screenCamera1);
        ClearBackground(RAYWHITE);
        camera1.BeginCameraMode();
        player1.Draw(RED);
        player2.Draw(BLUE);
        camera1.EndCameraMode();
        EndTextureMode();

        BeginTextureMode(screenCamera2);
        ClearBackground(RAYWHITE);
        camera2.BeginCameraMode();
        player1.Draw(RED);
        player2.Draw(BLUE);
        camera2.EndCameraMode();
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);

        DrawTextureRec(screenCamera1.texture, splitScreenRect, Vector2 { 0, 0 }, WHITE);
        DrawTextureRec(screenCamera2.texture, splitScreenRect, Vector2 { screenWidth / 2.0f, 0 }, WHITE);

        DrawRectangle(screenWidth / 2 - 2, 0, 4, screenHeight, LIGHTGRAY);

        EndDrawing();
    }

    CloseGame(screenCamera1,screenCamera2);
    return 0;
}
