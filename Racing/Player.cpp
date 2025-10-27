#include "Player.h"
#include "raymath.h"

Player::Player(float startX, float startY, int upKey, int downKey, int leftKey, int rightKey)
    : rect({ startX, startY, 20.0f, 10.0f }),
    origin({ rect.width / 2, rect.height / 2 }),
    speed(0),
    maxSpeed(300),
    rotation(270.0f),
    direction(-1),
    speedUp(125),
    slowDown(125),
    rotationSpeed(0.5f),
    moveUpKey(upKey),
    moveDownKey(downKey),
    moveLeftKey(leftKey),
    moveRightKey(rightKey) {}


void Player::Update(float deltaTime)
{
    if (rotation >= 360) rotation = 0;

    float radians = PI * rotation / 180;
    float xMove = speed * cosf(radians) * deltaTime;
    float yMove = speed * sinf(radians) * deltaTime;

    if (IsKeyDown(moveUpKey))
    {
        speed += speedUp * deltaTime;
        direction = -1;
        if (speed > maxSpeed) speed = maxSpeed;
    }
    else if (IsKeyDown(moveDownKey))
    {
        speed -= speedUp * deltaTime;
        direction = 1;
        if (speed < -maxSpeed) speed = -maxSpeed;
    }
    else
    {
        speed += slowDown * direction * deltaTime;
        if ((direction == -1 && speed <= 0) || (direction == 1 && speed >= 0))
            speed = 0;
    }

    if (IsKeyDown(moveRightKey))
        rotation += rotationSpeed * float(fabs(speed)) * deltaTime;

    if (IsKeyDown(moveLeftKey))
        rotation -= rotationSpeed * float(fabs(speed)) * deltaTime;

    rect.x += xMove;
    rect.y += yMove;
}

void Player::Draw(Color color) const
{
    DrawRectanglePro(rect, origin, rotation, color);
}
