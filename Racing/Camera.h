#ifndef CAMERA_H
#define CAMERA_H

#include "raylib.h"
#include "Player.h"

class Cam {
private:
    Camera2D camera;

public:
    Cam();
    void Update(const Player& player);
    void BeginCameraMode() const;
    void EndCameraMode();
    Camera2D GetCamera() const;
};
#endif
