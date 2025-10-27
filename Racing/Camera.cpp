#include "Camera.h"

Cam::Cam() {
    camera.target = { 0.0f, 0.0f };
    camera.offset = { 200.0f, 300.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}
void Cam::Update(const Player& player)
{
    camera.target = { player.rect.x + player.rect.width / 2, player.rect.y + player.rect.height / 2 };
}

void Cam::BeginCameraMode() const { BeginMode2D(camera); }

void Cam::EndCameraMode() { EndMode2D(); }

Camera2D Cam::GetCamera() const { return camera; }
