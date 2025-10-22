#include "raylib.h"
#include "rlgl.h"
#include <string>
#include <iostream>
#include <fstream>
#define STORAGE_DATA_FILE "save.data"

namespace Utils
{
    // Linear interpolation between start and end values based on t
    float Lerp(float start, float end, float t)
    {
        return start + t * (end - start);
    }

    // Clamps the value between a minimum and maximum range
    float Clamp(float value, float min, float max)
    {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
}

using namespace Utils;
using namespace std;

// Screen dimensions
const int screenWidth = 600;
const int screenHeight = 800;

// Constants for pipes and gameplay
const int numPipes = 3;
const int spacing = 350;
const int gapHeight = 200;
const float maxPipeSpeed = 300.0f;
const int baseGapSize = 200;
const int maxGapSize = 300;

// Game state variables
bool pause = false;
bool resumeCountdown = false;
float countdownTime = 0;
const float countdownDuration = 3.0f;
bool muted = false;

bool collision = false;
int score = 0;
int highscore = 0;

// Physics constants
const float gravity = 500.0f;
const float jumpStrength = 300.0f;

// Color constants
Color transparentGray = { 59, 63, 59, 190 };
Color countdownCol = { 150, 250, 150, 255 };

// Storage data position
enum StorageData
{
    STORAGE_POSITION_HISCORE = 0
};

// Storage class for saving and loading game data
class Storage
{
public:
    Storage(const string& filename) : fileName(filename) {}

    bool save(unsigned int position, int value)
    {
        int dataSize = 0;
        unsigned char* fileData = LoadFileData(fileName.c_str(), &dataSize);
        unsigned char* newFileData = NULL;

        if (fileData != NULL)
        {
            if (dataSize <= position * sizeof(int))
            {
                newFileData = resizeData(fileData, position, dataSize);
                if (!newFileData) return false;
            }
            else
            {
                newFileData = fileData;
            }

            int* dataPtr = (int*)newFileData;
            dataPtr[position] = value;
        }
        else
        {
            dataSize = (static_cast<unsigned long long>(position) + 1) * sizeof(int);
            fileData = (unsigned char*)RL_MALLOC(dataSize);
            if (!fileData) return false;

            int* dataPtr = (int*)fileData;
            dataPtr[position] = value;

            newFileData = fileData;
        }

        bool success = SaveFileData(fileName.c_str(), newFileData, (static_cast<unsigned long long>(position) + 1) * sizeof(int));
        RL_FREE(newFileData);
        return success;
    }

    int load(unsigned int position)
    {
        int dataSize = 0;
        unsigned char* fileData = LoadFileData(fileName.c_str(), &dataSize);

        if (!fileData || dataSize < (int)(position * sizeof(int)))
        {
            UnloadFileData(fileData);
            return 0;
        }

        int* dataPtr = (int*)fileData;
        int value = dataPtr[position];
        UnloadFileData(fileData);
        return value;
    }

private:
    string fileName;

    unsigned char* resizeData(unsigned char* data, unsigned int position, int dataSize)
    {
        unsigned char* newData = (unsigned char*)RL_REALLOC(data, (position + 1) * sizeof(int));
        if (!newData)
        {
            RL_FREE(data);
            return NULL;
        }

        int* dataPtr = (int*)newData;
        for (int i = dataSize / sizeof(int); i <= (int)position; i++)
        {
            dataPtr[i] = 0;
        }

        return newData;
    }
};

// Player structure
struct Player
{
    int x = screenWidth / 8;
    int y = 300;
    int previousY = 300;
    int width = 25;
    int height = 25;
    float speedY = 0.0f;
    float rotation = 0.0f;
};

// Pipe structure
struct Pipe
{
    int x = screenWidth * 2;
    int y = 0;
    int width = 100;
    int height = 325;
    float pipeSpeed = 200.0f;
    bool scored = false;
    int currentGapSize = baseGapSize;
};

Player player;
Pipe pipe[numPipes];
Texture2D background;
Texture2D pipeTexture;
Texture2D buttonONTexture;
Texture2D buttonOFFTexture;
Rectangle button;
Storage storage(STORAGE_DATA_FILE);
Sound jump;


// Adjusts the gap size of the pipe
void PipeGap(Pipe& pipe)
{
    pipe.height = GetRandomValue(150, screenHeight - pipe.currentGapSize - 150);
}

void InitButton()
{
    button.x = static_cast<float>(screenWidth - buttonONTexture.width - 10);
    button.y = static_cast<float>(screenHeight - buttonONTexture.height - 10);
    button.width = static_cast<float>(buttonONTexture.width);
    button.height = static_cast<float>(buttonONTexture.height);
}

void LoadAssets()
{
    string folderPath = string(GetApplicationDirectory()) + "\\assets";
    string bgPath = folderPath + "\\BG.png";
    Image bg = LoadImage(bgPath.c_str());
    ImageResize(&bg, screenWidth, screenHeight);
    background = LoadTextureFromImage(bg);
    UnloadImage(bg);

    string pipePath = folderPath + "\\Pipe.png";
    Image pipeImg = LoadImage(pipePath.c_str());
    pipeTexture = LoadTextureFromImage(pipeImg);
    UnloadImage(pipeImg);

    string buttonONPath = folderPath + "\\Sound ON.png";
    Image buttonONImg = LoadImage(buttonONPath.c_str());
    ImageResize(&buttonONImg, static_cast<int>(buttonONImg.width * 2.5f), static_cast<int>(buttonONImg.height * 2.5f));
    buttonONTexture = LoadTextureFromImage(buttonONImg);
    UnloadImage(buttonONImg);

    string buttonOFFPath = folderPath + "\\Sound OFF.png";
    Image buttonOFFImg = LoadImage(buttonOFFPath.c_str());
    ImageResize(&buttonOFFImg, static_cast<int>(buttonOFFImg.width * 2.5f), static_cast<int>(buttonOFFImg.height * 2.5f));
    buttonOFFTexture = LoadTextureFromImage(buttonOFFImg);
    UnloadImage(buttonOFFImg);

    string jumpPath = folderPath + "\\Jump.wav";
    jump = LoadSound(jumpPath.c_str());
}

// Resets the game state
void ResetGame()
{
    player.y = static_cast<int>(Lerp(static_cast<float>(player.y), 300.0f, 0.2f));
    player.speedY = 0.0f;

    for (int i = 0; i < numPipes; i++)
    {
        pipe[i].x = screenWidth + (i * spacing);
        pipe[i].pipeSpeed = 200.0f;
        pipe[i].scored = false;
        PipeGap(pipe[i]);
    }

    score = 0;
    collision = false;
    pause = false;
}

// Initializes the game
void InitGame()
{
    InitWindow(screenWidth, screenHeight, "Flappy Shakhaa");
    SetTargetFPS(60);
    highscore = storage.load(STORAGE_POSITION_HISCORE);

    LoadAssets();
    InitButton();
    ResetGame();

    while (true)
    {
        if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
        {
            // Prevent game start if the mute button is clicked
            if (!(CheckCollisionPointRec(GetMousePosition(), button) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
            {
                PlaySound(jump);
                break; // Exit the menu loop to start the game
            }
        }

        BeginDrawing();
        ClearBackground(WHITE);
        DrawTexture(background, 0, 0, WHITE);
        DrawRectangle(75, 300, player.width, player.height, GRAY);
        DrawText("Press Space, Up Arrow, or Mouse Button to Start", screenWidth / 14, screenHeight / 8, 20, BLACK);
        DrawText("Press E to Pause/Unpause, Escape to Exit the Game", screenWidth / 18, screenHeight / 5, 20, BLACK);
        DrawTexture(muted ? buttonOFFTexture : buttonONTexture, static_cast<int>(button.x), static_cast<int>(button.y), WHITE);
        EndDrawing();


        if (CheckCollisionPointRec(GetMousePosition(), button) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            muted = !muted;
            SetSoundVolume(jump, muted ? 0.0f : 1.0f);
            continue;
        }

        // Exit condition
        if (IsKeyPressed(KEY_ESCAPE) || WindowShouldClose())
        {
            CloseWindow();
            return;
        }
    }

    for (int i = 0; i < numPipes; i++)
    {
        pipe[i].x = screenWidth + (i * spacing);
        PipeGap(pipe[i]);
    }
}

void UpdatePlayer(float deltaTime)
{
    player.speedY += gravity * deltaTime;
    player.y += static_cast<int>(player.speedY * deltaTime);

    if ((IsKeyPressed(KEY_SPACE)|| IsKeyPressed(KEY_UP) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) && player.y + player.height < screenHeight)
    {
        if (!(CheckCollisionPointRec(GetMousePosition(), button) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
        {  
            PlaySound(jump);
            player.speedY = -jumpStrength;
            player.rotation = 0.0f;
        }
    }
    if (player.speedY > 0)
    {
        // Player is falling, rotate down
        player.rotation = Lerp(player.rotation, 270.0f, 0.1f);  // Gradual tilt downwards
    }
    else if (player.speedY < 0)
    {
        // Player is rising, rotate up
        player.rotation = Lerp(player.rotation, 135.0f, 0.1f);  // Gradual tilt upwards
    }
    else
    {
        // Player is idle (not moving vertically), reset rotation to neutral position
        player.rotation = Lerp(player.rotation, 0.0f, 0.1f);  // Smoothly reset to 0 degrees
    }

    if (player.y + player.height >= screenHeight)
    {
        player.y = screenHeight - player.height;
        player.speedY = 0;
        collision = true;
        if (score > highscore) highscore = score;
    }
    else if (player.y <= 0)
    {
        player.y = 0;
    }
}
void UpdatePipes(float deltaTime)
{
    for (int i = 0; i < numPipes; i++)
    {
        pipe[i].pipeSpeed = 200.0f + static_cast<float>(score) / 5.0f;
        pipe[i].x -= static_cast<int>(pipe[i].pipeSpeed * deltaTime);
        pipe[i].pipeSpeed = Clamp(pipe[i].pipeSpeed + 0.01f, 200.0f, maxPipeSpeed);

        int targetGapSize = baseGapSize + static_cast<int>((pipe[i].pipeSpeed - 200.0f) * 0.5f);
        targetGapSize = static_cast<int>(Clamp(static_cast<float>(targetGapSize), baseGapSize, maxGapSize));
        pipe[i].currentGapSize = static_cast<int>(Lerp(static_cast<float>(pipe[i].currentGapSize), static_cast<float>(targetGapSize), 0.1f * deltaTime));

        if (pipe[i].x + pipe[i].width <= 0)
        {
            pipe[i].x = screenWidth + spacing;
            pipe[i].scored = false;
            PipeGap(pipe[i]);
        }

        if (!pipe[i].scored && player.x > pipe[i].x + pipe[i].width)
        {
            score++;
            pipe[i].scored = true;
        }
    }
}
void CheckCollisions()
{
    for (int i = 0; i < numPipes; i++)
    {
        Rectangle playerRect =
        { 
            static_cast<float>(player.x),
            static_cast<float>(player.y),
            static_cast<float>(player.width),
            static_cast<float>(player.height)
        };
        Rectangle upperPipeRect =
        { 
            static_cast<float>(pipe[i].x),
            0,
            static_cast<float>(pipe[i].width),
            static_cast<float>(pipe[i].height)
        };
        Rectangle lowerPipeRect = 
        {
            static_cast<float>(pipe[i].x),
            static_cast<float>(pipe[i].height + gapHeight),
            static_cast<float>(pipe[i].width),
            static_cast<float>(screenHeight - (pipe[i].height + gapHeight))
        };

        if (CheckCollisionRecs(playerRect, upperPipeRect) || CheckCollisionRecs(playerRect, lowerPipeRect))
        {
            collision = true;
            if (score > highscore) highscore = score;
            storage.save(STORAGE_POSITION_HISCORE, highscore);
            break;
        }
    }
}
void HandlePauseAndCountdown(float deltaTime)
{
    if (IsKeyPressed(KEY_E) && !resumeCountdown && !collision)
    {
        pause = !pause;

        if (!pause)
        {
            resumeCountdown = true;
            countdownTime = countdownDuration;
        }
    }

    if (resumeCountdown)
    {
        countdownTime -= deltaTime;

        if (countdownTime <= 0)
        {
            countdownTime = 0;
            resumeCountdown = false;
            pause = false;
        }
        else
        {
            pause = true;
        }
    }
}

void HandleMuteButton()
{
    if (CheckCollisionPointRec(GetMousePosition(), button) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        muted = !muted;
        SetSoundVolume(jump, muted ? 0.0f : 1.0f);
    }
}

// Updates the game state
void UpdateGame(float deltaTime)
{
    if (collision && IsKeyPressed(KEY_R))
    {
        ResetGame();
    }

    if (!pause && !collision)
    {
        // Update player position based on physics and controls
        player.previousY = player.y; // Store previous Y position for movement calculation
        UpdatePlayer(deltaTime); // Update player movement

        // Update pipes movement and check for scoring
        UpdatePipes(deltaTime);

        // Check for collisions
        CheckCollisions();

        HandleMuteButton();
    }

    // Handle pause/resume and countdown
    HandlePauseAndCountdown(deltaTime);
    
}

// Renders the game
void RenderGame()
{
    BeginDrawing();
    ClearBackground(WHITE);

    DrawTexture(background, 0, 0, WHITE);

    for (int i = 0; i < numPipes; i++)
    {
        Rectangle sourceRectUpper = { 0.0f, (float)pipeTexture.height, (float)pipeTexture.width, -(float)pipeTexture.height };
        Rectangle sourceRectLower = { 0.0f, 0.0f, (float)pipeTexture.width, (float)pipeTexture.height };
        Rectangle destUpper = { (float)pipe[i].x, 0.0f, (float)pipe[i].width, (float)pipe[i].height };
        Vector2 pipeOrigin = { 0.0f, 0.0f };

        DrawTexturePro(pipeTexture, sourceRectUpper, destUpper, pipeOrigin, 0.0f, WHITE);

        Rectangle destLower = {
            (float)pipe[i].x,
            (float)(pipe[i].height + pipe[i].currentGapSize),
            (float)pipe[i].width,
            (float)(screenHeight - (pipe[i].height + pipe[i].currentGapSize))
        };

        DrawTexturePro(pipeTexture, sourceRectLower, destLower, pipeOrigin,0.0f, WHITE);
    }
    Vector2 playerOrigin = { static_cast<float>(player.width / 2),static_cast<float>(player.height / 2) };
    Rectangle playerRect = { (float)player.x, (float)player.y, (float)player.width, (float)player.height };
    DrawRectanglePro(playerRect, playerOrigin, player.rotation, GRAY);
    DrawText(("Score: " + to_string(score)).c_str(), 50, 50, 25, WHITE);
    DrawText(("HighScore: " + to_string(highscore)).c_str(), screenWidth - 200, 50, 25, WHITE);

    if (collision)
    {
        DrawText("Game Over! Press R to Restart", screenWidth / 4, screenHeight / 2, 20, RED);
    }

    if (pause && !resumeCountdown)
    {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(transparentGray, 0.75f));
        DrawText("Paused", screenWidth / 2 - 75, screenHeight / 2 - 75, 40, BLUE);
    }

    if (resumeCountdown)
    {
        DrawText(("Resuming in: " + to_string(static_cast<int>(ceil(countdownTime)))).c_str(), screenWidth / 2 - 100, screenHeight / 2-50, 32, countdownCol);
    }

    DrawTexture(muted ? buttonOFFTexture : buttonONTexture, static_cast<int>(button.x), static_cast<int>(button.y), WHITE);
    EndDrawing();
}

// Closes the game
void CloseGame()
{
    UnloadTexture(background);
    UnloadTexture(pipeTexture);
    UnloadTexture(buttonONTexture);
    UnloadTexture(buttonOFFTexture);

    UnloadSound(jump);
    CloseAudioDevice();

    storage.save(STORAGE_POSITION_HISCORE, highscore);

    CloseWindow();
}

int main()
{
    InitAudioDevice();
    InitGame();
    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();
        UpdateGame(deltaTime);
        RenderGame();
    }

    CloseGame();
    return 0;
}