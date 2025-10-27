#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

class Player
{
public:
    Rectangle rect;
    Vector2 origin;
    float speed;
    float maxSpeed;
    float rotation;
    int direction;
    int speedUp;
    int slowDown;
    float rotationSpeed;
    int moveUpKey, moveDownKey, moveLeftKey, moveRightKey;

    Player(float startX, float startY, int upKey, int downKey, int leftKey, int rightKey);

    void Update(float deltaTime);
    void Draw(Color color) const;
};

#endif