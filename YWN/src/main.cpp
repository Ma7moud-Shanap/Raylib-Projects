#include "raylib.h"
#include "raymath.h"
#include "string"

using namespace std;

Camera3D camera = { 0 };
Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
float hue = 0;
double lastTime = 0;

void CloneSelf(int argc, char *argv[])
{
    if (argc > 0)
    {
        if (GetTime() - lastTime > 5.0f)
        {
            OpenURL(argv[0]);
            lastTime = GetTime();
        }
    }
}

void InitProgram()
{
    const int screenWidth = 350;
    const int screenHeight = 175;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "YWN");


    camera.position = { 0.0f, 10.0f, 10.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    SetTargetFPS(30);
    lastTime = GetTime();
}

Color ColorModifier()
{
    hue = fmodf(hue + 0.5f, 360.0f);
    return ColorFromHSV(hue, 1.0f, 1.0f);
}

int main(int argc,char *argv[])
{
    InitProgram();

    while (!WindowShouldClose()) 
    {
        CloneSelf(argc, argv);
        UpdateCamera(&camera, CAMERA_ORBITAL);
        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        
        DrawCubeV(cubePosition, { 3.0f, 3.0f, 3.0f }, ColorModifier());
        DrawCubeWiresV(cubePosition, { 3.0f, 3.0f, 3.0f }, BLACK);

        EndMode3D();

        Image img = GenImageGradientSquare(1024, 1024, 0, ColorModifier(), ColorModifier());
        Texture2D textureLeak1 = LoadTextureFromImage(img);
        Texture2D textureLeak2 = LoadTextureFromImage(img);
        Texture2D textureLeak3 = LoadTextureFromImage(img);
        Texture2D textureLeak4 = LoadTextureFromImage(img);

        EndDrawing();
    }
    CloseWindow();    

    return 0;
}

