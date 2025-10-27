#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <string>

namespace Game
{
	// Screen dimensions
	constexpr int screenWidth = 800;
	constexpr int screenHeight = 600;

	//Resources
	std::string resourcePath = std::string(GetApplicationDirectory()) + "Src";
	Texture2D playerTxr;
	Texture2D pistolTxr;
	Texture2D sniperTxr;
	Texture2D rifleTxr;

	//Player
	Vector2 player = { screenWidth/2, screenHeight/2 };
	float playerSpd = 150;
	int maxHp = 20;
	int playerHp = 20;
	bool controllable = true;
	bool isWalkingRight = true;

	//Gun
	float gunRot = 0.0f;
	float fireCoolDown = 0.0f;
	int type = 0;
	int previousType = type;
	struct GunType
	{	//Default Values
		Texture2D txr;
		Vector2 pos{};
		float Fire_Rate = 0.25f;
		int Bullet_Speed = 450;
		float Reload_Timer = 1.5f;
		int loadedAmmo = 30;
		int clipSize = 30;
		int currentAmmo = 100;
		int maxAmmo = 100;
		int damage = 5;
	};
	GunType gunType[3];
	float gunWidth[3];
	float gunHeight[3];

	//Enemy
	constexpr int maxEnemyies = 100;
	constexpr int maxEnemyHp = 50;
	constexpr int enemySize = 15;
	constexpr float Spawn_Timer = 3.0f;
	constexpr int enemySpd = 10;

	Vector2 enemies[maxEnemyies];
	int enemyCount = 0;
	float spawnTimer = 0;
	int enemyHp[maxEnemyies];
	float damageCooldown;
	bool canDamage;

	//Ammo
	constexpr int constMaxAmmo = 300;
	float reloadTimer = gunType[type].Reload_Timer;
	bool isReloading = false;

	//Bullet
	struct Bullet
	{
		Vector2 position;
		Vector2 velocity;
		bool active;
	};
	Bullet bullets[constMaxAmmo];

	//Pickup
	struct Pickup
	{
		Rectangle rect{ 20,20,10,20 };
		bool active = false;
	}pickup[maxEnemyies];

	//Heal
	struct Heal
	{
		Rectangle rect{ 20,20,10,20 };
		bool active = false;
	}heal[maxEnemyies];

	//Shop
	bool shopOpenned = false;

	struct Abilities
	{
		int healUpgrade = 1;
		int ammoUpgrade = 10;
		int speedUpgrade = 10;
		bool autoAim = false;
		float autoAimTimer = 20.0f;
		float autoAimRange = 300.0f;
	}abilities;

	static Rectangle HP;
	static Rectangle Ammo;
	static Rectangle Speed;
	static Rectangle Aim;
	static Rectangle Range;
	static Rectangle Timer;

	bool shopButtonPressed = false;

	//Coins
	const int maxCoins = 999;
	int coins = 0;
	struct Coins
	{
		Rectangle rect{};
		bool active = false;
	};
	Coins coinsStr[maxCoins];
	int	activeCoins = 0;
}
using namespace Game;

void LoadResources()
{
	playerTxr = LoadTexture((resourcePath + "\\Player.png").c_str());
	pistolTxr = LoadTexture((resourcePath + "\\Pistol.png").c_str());
	sniperTxr = LoadTexture((resourcePath + "\\Sniper.png").c_str());
	rifleTxr = LoadTexture((resourcePath + "\\Rifle.png").c_str());

	gunWidth[0] = pistolTxr.width;
	gunWidth[1] = sniperTxr.width;
	gunWidth[2] = rifleTxr.width;
	gunHeight[0] = pistolTxr.height;
	gunHeight[1] = sniperTxr.height;
	gunHeight[2] = rifleTxr.height;

	gunType[0] = { pistolTxr, {0,0}, 0.5f, 450, 1.5f, 30, 30, 150, 150, 10 }; // Pistol
	gunType[1] = { sniperTxr, {0,0}, 0.8f, 650, 3.5f, 10, 10, 50, 50, 50 };   // Sniper
	gunType[2] = { rifleTxr, {0,0}, 0.2f, 500, 2.0f, 30, 30, 150, 150, 10 };  // Rifle
}

void ShootBullet(Vector2 pos, Vector2 dir)
{
	for (int i = 0; i < constMaxAmmo; i++)
	{
		if (!bullets[i].active)
		{
			bullets[i].position = pos;
			bullets[i].velocity = Vector2Scale(Vector2Normalize(dir), static_cast<float>(gunType[type].Bullet_Speed));
			bullets[i].active = true;
			gunType[type].loadedAmmo--;
			break;
		}
	}
}

void UpdateBullets(float deltaTime)
{
	for (int i = 0; i < constMaxAmmo; i++)
	{
		if (bullets[i].active)
		{
			bullets[i].position = Vector2Add(bullets[i].position, Vector2Scale(bullets[i].velocity, deltaTime));
			if (bullets[i].position.x < 0 || bullets[i].position.x > screenWidth || 
			bullets[i].position.y < 0 || bullets[i].position.y > screenHeight)
			{
				bullets[i].active = false;
			}
		}
	}
}

void SpawnEnemy(float deltaTime)
{
	spawnTimer -= deltaTime;

	if (enemyCount < maxEnemyies && spawnTimer <= 0)
	{	
		Vector2 spawnPos{};
		switch (GetRandomValue(0, 3))
		{
		case 0:
			//top
			spawnPos = { static_cast<float>(GetRandomValue(0, screenWidth)), -enemySize };
			break;
		case 1:
			//down
			spawnPos = { static_cast<float>(GetRandomValue(0, screenWidth)), screenHeight };
			break;
		case 2:
			//right
			spawnPos = { screenWidth, static_cast<float>(GetRandomValue(0, screenHeight)) };
			break;
		case 3:
			//left
			spawnPos = { -enemySize, static_cast<float>(GetRandomValue(0, screenHeight)) };
			break;
		}

		enemies[enemyCount] = spawnPos;
		enemyHp[enemyCount] = maxEnemyHp;
		enemyCount++;
		spawnTimer = Spawn_Timer;
	}
}

void UpdateEnemy(float deltaTime)
{
	for (int i = 0; i < enemyCount; i++)
	{
		Vector2 enemyCenter{ enemies[i].x + enemySize / 2.0f, enemies[i].y + enemySize / 2.0f };
		Vector2 direction = Vector2Subtract({ player.x,player.y }, enemyCenter);
		direction = Vector2Normalize(direction);
		if (!CheckCollisionRecs(Rectangle{ enemies[i].x,enemies[i].y,static_cast<float>(enemySize),static_cast<float>(enemySize) }, Rectangle{ player.x - playerTxr.width / 2,player.y - playerTxr.height / 2,static_cast<float>(playerTxr.width),static_cast<float>(playerTxr.height) }))
		{
			enemies[i].x += direction.x * enemySpd * deltaTime;
			enemies[i].y += direction.y * enemySpd * deltaTime;
		}
	}
}

void SpawnCoin(Vector2 position)
{
	if (activeCoins >= maxCoins) return;

	for (int i = 0; i < maxCoins; i++)
	{
		if (!coinsStr[i].active)
		{
			coinsStr[i].rect = { position.x, position.y, 10, 10 };
			coinsStr[i].active = true;
			activeCoins++;
			break;
		}
	}
}

void HandleEnemyDeath(int enemyIndex, int wayEnemyDied)
{
	Vector2 deadEnemyPos = enemies[enemyIndex];

	if (wayEnemyDied == 0)
	{
		for (int i = 0; i < 2;i++)
		{
			SpawnCoin(deadEnemyPos);
		}

		if (GetRandomValue(1, 3) == 1)
		{
			for (int p = 0; p < maxEnemyies; p++)
			{
				if (!pickup[p].active)
				{
					pickup[p].active = true;
					pickup[p].rect.x = deadEnemyPos.x;
					pickup[p].rect.y = deadEnemyPos.y;
					break;
				}
			}
		}

		if (GetRandomValue(1, 3) == 1)
		{
			for (int h = 0; h < maxEnemyies; h++)
			{
				if (!heal[h].active)
				{
					heal[h].active = true;
					heal[h].rect.x = deadEnemyPos.x;
					heal[h].rect.y = deadEnemyPos.y;
					break;
				}
			}
		}
	}

	else if (wayEnemyDied == 1)
	{
		for (int i = 0; i < 1;i++)
		{
			SpawnCoin(deadEnemyPos);
		}

		if (GetRandomValue(1, 5) == 1)
		{
			for (int p = 0; p < maxEnemyies; p++)
			{
				if (!pickup[p].active)
				{
					pickup[p].active = true;
					pickup[p].rect.x = deadEnemyPos.x;
					pickup[p].rect.y = deadEnemyPos.y;
					break;
				}
			}
		}

		if (GetRandomValue(1, 5) == 1)
		{
			for (int h = 0; h < maxEnemyies; h++)
			{
				if (!heal[h].active)
				{
					heal[h].active = true;
					heal[h].rect.x = deadEnemyPos.x;
					heal[h].rect.y = deadEnemyPos.y;
					break;
				}
			}
		}
	}

	for (int k = enemyIndex; k < enemyCount - 1; k++)
	{
		enemies[k] = enemies[k + 1];
		enemyHp[k] = enemyHp[k + 1];
	}
	enemyCount--;
}

void Collisions()
{
	//Bullet-Enemy Collision
	for (int i = 0; i < constMaxAmmo; i++)
	{
		if (!bullets[i].active) { continue; }

		bool bulletHit = false;
		for (int j = 0; j < enemyCount; j++)
		{
			if ((CheckCollisionRecs(Rectangle{ bullets[i].position.x,bullets[i].position.y,5,5 }, Rectangle{ enemies[j].x, enemies[j].y, static_cast<float>(enemySize), static_cast<float>(enemySize) })))
			{
				enemyHp[j] -= gunType[type].damage;
				bulletHit = true;

				if ((enemyHp[j] <= 0))
				{
					HandleEnemyDeath(j, 0); // Zero means the enemy died with the gun
					j--;				
				}
				break;
			}
		}

		if (bulletHit)
		{
			bullets[i].active = false;
		}
	}

	//Enemy-Player Collision
	for (int i = 0; i < enemyCount; i++)
	{
		if ((CheckCollisionRecs(Rectangle{ enemies[i].x,enemies[i].y,static_cast<float>(enemySize),static_cast<float>(enemySize) }, Rectangle{ player.x - playerTxr.width / 2,player.y - playerTxr.height / 2,static_cast<float>(playerTxr.width),static_cast<float>(playerTxr.height) })))
		{
			playerHp--;
			HandleEnemyDeath(i, 1); //One means the enemy died with the player collision
			i--;
			break;
		}
	}

	for (int i = 0; i < maxCoins; i++)
	{
		//Coins Pickup
		if (coinsStr[i].active && CheckCollisionRecs(Rectangle{ player.x - playerTxr.width / 2,player.y - playerTxr.height / 2,static_cast<float>(playerTxr.width),static_cast<float>(playerTxr.height) }, coinsStr[i].rect))
		{
			if (coins < maxCoins)
			{
				coins++;
				coinsStr[i].active = false;
				activeCoins = 0;
			}
		}
	}

	//Pickup Collisions
	for (int i = 0; i < enemyCount; i++)
	{
		//Ammo Pickup
		if (pickup[i].active && CheckCollisionRecs(Rectangle{ player.x - playerTxr.width / 2,player.y - playerTxr.height / 2,static_cast<float>(playerTxr.width),static_cast<float>(playerTxr.height) }, pickup[i].rect))
		{
			if (gunType[type].currentAmmo < gunType[type].maxAmmo)
			{
				gunType[type].currentAmmo += gunType[type].clipSize;
				pickup[i].active = false;
			}
		}
		//Heal Pickup
		if (heal[i].active && CheckCollisionRecs(Rectangle{ player.x - playerTxr.width / 2,player.y - playerTxr.height / 2,static_cast<float>(playerTxr.width),static_cast<float>(playerTxr.height) }, heal[i].rect))
		{
			if (playerHp < maxHp)
			{
				playerHp++;
				heal[i].active = false;
			}
		}
	}
}

void Reload(float deltaTime)
{
	if (previousType != type) { isReloading = false; }
	if (isReloading)
	{
		reloadTimer -= deltaTime;

		if (reloadTimer <= 0.0f)
		{
			reloadTimer = gunType[type].Reload_Timer;
			isReloading = false;

			if (gunType[type].currentAmmo >= gunType[type].loadedAmmo)
			{
				gunType[type].currentAmmo -= gunType[type].clipSize - gunType[type].loadedAmmo;
				gunType[type].loadedAmmo += gunType[type].clipSize - gunType[type].loadedAmmo;
			}
			if (gunType[type].currentAmmo <= 0)
			{
				gunType[type].loadedAmmo += gunType[type].currentAmmo;
				gunType[type].currentAmmo = 0;
			}
		}
	}
}

void UpdateGun(float deltaTime)
{
	Vector2 dir{};

	//Update the gun position
	if (type == 0)
	{
		gunType[type].pos.x = player.x;
		gunType[type].pos.y = player.y+5;
	}
	if (type == 1)
	{
		gunType[type].pos.x = player.x;
		gunType[type].pos.y = player.y+5;
	}
	if (type == 2)
	{
		gunType[type].pos.x = player.x;
		gunType[type].pos.y = player.y+5;
	}

	if (!abilities.autoAim || enemyCount == 0)
	{ 	
		//Making the Gun rotate with the mouse
		Vector2 mousePos = GetMousePosition();
		dir = Vector2Normalize(Vector2{ mousePos.x - player.x, mousePos.y - player.y });
		gunRot = atan2f(dir.y, dir.x) * RAD2DEG;

		//Shooting when the mouse left button pressed(Down)
		if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && fireCoolDown <= 0.0f && gunType[type].loadedAmmo > 0 && !isReloading)
		{
			Vector2 bulletSpawnPos = Vector2{ gunType[type].pos.x + dir.x * gunWidth[type] - 2.5f, gunType[type].pos.y + dir.y * gunWidth[type] - 2.5f };
			ShootBullet(bulletSpawnPos, dir);
			fireCoolDown = gunType[type].Fire_Rate;
		}
	}
	else
	{
		abilities.autoAimTimer -= deltaTime;
		if (abilities.autoAimTimer <= 0)
		{
			abilities.autoAim = false;
		}

		int nearestEnemy = -1;
		float minDistance = abilities.autoAimRange;
		Vector2 playerCenter{ player.x,player.y };

		for (int i = 0; i < enemyCount; i++)
		{
			Vector2 enemyCenter{ enemies[i].x + enemySize / 2,enemies[i].y + enemySize / 2 };
			float distance = Vector2Distance(playerCenter, enemyCenter);
			if (distance < minDistance)
			{
				minDistance = distance;
				nearestEnemy = i;
			}
		}
		if (nearestEnemy != -1)
		{
			dir = Vector2Normalize(Vector2Subtract(enemies[nearestEnemy], playerCenter));
			gunRot = atan2f(dir.y, dir.x) * RAD2DEG;

			if (fireCoolDown <= 0.0f && gunType[type].loadedAmmo > 0 && !isReloading)
			{
				Vector2 bulletSpawnPos = Vector2{ gunType[type].pos.x + dir.x * gunWidth[type] - 2.5f, gunType[type].pos.y + dir.y * gunWidth[type] - 2.5f };
				ShootBullet(bulletSpawnPos, dir);
				fireCoolDown = gunType[type].Fire_Rate;
			}
		}
	}
}

void PlayerMovement(float deltaTime)
{
	//If the Player dies the game stops
	if (playerHp <= 0) { controllable = false; }

	fireCoolDown -= deltaTime;
	if (fireCoolDown < 0.0f) { fireCoolDown = 0.0f; }

	//Player Movement
	Vector2 movement = { 0.0f,0.0f };
	if (IsKeyDown(KEY_LEFT)) { movement.x -= 1; isWalkingRight = false; }
	if (IsKeyDown(KEY_RIGHT)) { movement.x += 1; isWalkingRight = true; }
	if (IsKeyDown(KEY_UP)) { movement.y -= 1; }
	if (IsKeyDown(KEY_DOWN)) { movement.y += 1; }

	Vector2 normalizedMovement = Vector2Normalize(movement);
	player.x += normalizedMovement.x * playerSpd * deltaTime;
	player.y += normalizedMovement.y * playerSpd * deltaTime;

	//Weapon Switch
	previousType = type;
	if (IsKeyPressed(KEY_ONE)) { type = 0; isReloading = false; }
	if (IsKeyPressed(KEY_TWO)) { type = 1; isReloading = false; }
	if (IsKeyPressed(KEY_THREE)) { type = 2; isReloading = false; }

	//Reload
	if ((IsKeyPressed(KEY_RIGHT_CONTROL) || gunType[type].loadedAmmo <= 0) && !isReloading)
	{
		isReloading = true;
		reloadTimer = gunType[type].Reload_Timer;
	}
}

void Shop()
{
	if (!shopOpenned) { return; }

	static Rectangle HP = { 50,250,100,100 };
	static Rectangle Ammo = { 170,250,100,100 };
	static Rectangle Speed = { 290,250,100,100 };
	static Rectangle Aim = { 410,250,100,100 };
	static Rectangle Range = { 530,250,100,100 };
	static Rectangle Timer = { 650,250,100,100 };
	 
	DrawRectangleRec(HP, GREEN);
	DrawText("HP",		int(HP.x + 40), int(HP.y + HP.height / 2 - 20), 20, BLACK);
	DrawText("Upgrade", int(HP.x + 8),  int(HP.y + HP.height / 2),		20, BLACK);


	DrawRectangleRec(Ammo, GRAY);
	DrawText("Ammo",	int(Ammo.x + 27), int(Ammo.y + Ammo.height / 2 - 20), 20, BLACK);
	DrawText("Upgrade", int(Ammo.x + 8),  int(Ammo.y + Ammo.height / 2),	  20, BLACK);

	DrawRectangleRec(Speed, SKYBLUE);
	DrawText("Speed",   int(Speed.x + 20), int(Speed.y + Speed.height / 2 - 20), 20, BLACK);
	DrawText("Upgrade", int(Speed.x + 8),  int(Speed.y + Speed.height / 2),		 20, BLACK);

	DrawRectangleRec(Aim, GOLD);
	DrawText("Auto", int(Aim.x + 30), int(Aim.y + Aim.height / 2 - 20), 20, BLACK);
	DrawText("Aim",  int(Aim.x + 38), int(Aim.y + Aim.height / 2),		20, BLACK);

	DrawRectangleRec(Range, ORANGE);
	DrawText("Auto",  int(Range.x + 30), int(Range.y + Range.height / 2 - 25), 20, BLACK);
	DrawText("Aim",   int(Range.x + 38), int(Range.y + Range.height / 2 - 5),  20, BLACK);
	DrawText("Range", int(Range.x + 23), int(Range.y + Range.height / 2 + 15), 20, BLACK);
	
	DrawRectangleRec(Timer, VIOLET);
	DrawText("Auto",  int(Timer.x + 30), int(Timer.y + Timer.height / 2 - 25), 20, BLACK);
	DrawText("Aim",   int(Timer.x + 38), int(Timer.y + Timer.height / 2 - 5),  20, BLACK);
	DrawText("Timer", int(Timer.x + 23), int(Timer.y + Timer.height / 2 + 15), 20, BLACK);

	DrawText(TextFormat("Coins: %d",coins), 20, screenHeight - 120, 20, WHITE);

	Vector2 mousePos = GetMousePosition();
		
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		if (!shopButtonPressed)
		{
			shopButtonPressed = true;

			if (CheckCollisionPointRec(mousePos, HP) && coins >= 20)
			{
				coins -= 20;
				maxHp += abilities.healUpgrade;
				playerHp++;
				if (playerHp > maxHp) { playerHp = maxHp; }
			}
			else if (CheckCollisionPointRec(mousePos, Ammo) && coins >= 50)
			{
				coins -= 50;
				for (int i = 0; i < 3; i++)
				{
					gunType[i].clipSize += abilities.ammoUpgrade;
					gunType[i].loadedAmmo = gunType[i].clipSize;
				}
			}
			else if (CheckCollisionPointRec(mousePos, Speed) && coins >= 30)
			{
				coins -= 30;
				playerSpd += abilities.speedUpgrade;
			}
			else if (CheckCollisionPointRec(mousePos, Aim) && coins >= 100)
			{
				coins -= 100;
				abilities.autoAim = true;
			}
			else if (CheckCollisionPointRec(mousePos, Range) && coins >= 80)
			{
				coins -= 80;
				abilities.autoAimRange += 50;
			}
			else if (CheckCollisionPointRec(mousePos, Timer) && coins >= 60)
			{
				coins -= 60;
				abilities.autoAimTimer += 5;
			}
		}
	}
	else
	{
		shopButtonPressed = false;
	}
}

void InitGame()
{
	InitWindow(screenWidth, screenHeight, "");
	LoadResources();
	SetTargetFPS(60);
}

void UpdateGame(float deltaTime)
{
	if (controllable)
	{
		PlayerMovement(deltaTime);
		UpdateBullets(deltaTime);
		SpawnEnemy(deltaTime);
		UpdateEnemy(deltaTime);
		Collisions();
		Reload(deltaTime);
		UpdateGun(deltaTime);
	}
	if (IsKeyPressed(KEY_SPACE)) { controllable = !controllable; shopOpenned = !shopOpenned; }
	Shop();
}

void DrawGame()
{
	if (!shopOpenned)
	{
		BeginDrawing();

		//Background
		ClearBackground(Color{ 224,187,99,255 });

		//Player
		{
			Rectangle dest{ player.x,player.y ,static_cast<float>(playerTxr.width),static_cast<float>(playerTxr.height) };
			Vector2 origin{ static_cast<float>(playerTxr.width) / 2,static_cast<float>(playerTxr.height) / 2 };
			if (isWalkingRight)
			{
				Rectangle source{ 0,0,static_cast<float>(playerTxr.width),static_cast<float>(playerTxr.height) };
				DrawTexturePro(playerTxr,
					source,
					dest,
					origin, 0, WHITE);
			}
			else if (!isWalkingRight)
			{
				Rectangle source{ 0,0,static_cast<float>(-playerTxr.width),static_cast<float>(playerTxr.height) };
				DrawTexturePro(playerTxr,
					source,
					dest,
					origin, 0, WHITE);
			}
		}

		//Gun
		{
			if (type == 0)
			{
				Rectangle source{ 0,0,static_cast<float>(pistolTxr.width),static_cast<float>(pistolTxr.height) };
				Rectangle dest{ gunType[type].pos.x,gunType[type].pos.y,static_cast<float>(pistolTxr.width),static_cast<float>(pistolTxr.height) };
				Vector2 origin{ 0,pistolTxr.height / 2.0f };
				DrawTexturePro(pistolTxr, source, dest, origin, gunRot, WHITE);
			}
			if (type == 1)
			{
				Rectangle source{ 0,0,static_cast<float>(sniperTxr.width),static_cast<float>(sniperTxr.height) };
				Rectangle dest{ gunType[type].pos.x,gunType[type].pos.y,static_cast<float>(sniperTxr.width),static_cast<float>(sniperTxr.height) };
				Vector2 origin{ 0,sniperTxr.height / 2.0f };
				DrawTexturePro(sniperTxr, source, dest, origin, gunRot, WHITE);
			}
			if (type == 2)
			{
				Rectangle source{ 0,0,static_cast<float>(rifleTxr.width),static_cast<float>(rifleTxr.height) };
				Rectangle dest{ gunType[type].pos.x,gunType[type].pos.y,static_cast<float>(rifleTxr.width),static_cast<float>(rifleTxr.height) };
				Vector2 origin{ 0,rifleTxr.height / 2.0f };
				DrawTexturePro(rifleTxr, source, dest, origin, gunRot, WHITE);
			}
		}

		//Enemy
		for (int i = 0; i < enemyCount; i++)
		{
			DrawRectangleV(enemies[i], Vector2{ static_cast<float>(enemySize), static_cast<float>(enemySize) }, RED);
			float hpWidth = enemyHp[i] / static_cast<float>(maxEnemyHp);
			DrawRectangleV({ enemies[i].x, enemies[i].y - 7 }, { hpWidth * enemySize, enemySize / 3.75f }, GREEN);
		}

		//Bullets
		for (int i = 0; i < gunType[type].maxAmmo; i++)
		{
			if (bullets[i].active) 
			{
				DrawRectangleV(bullets[i].position, Vector2{ 5,5 }, RED);
			}
		}

		//PickUps
		for (int i = 0;i < maxEnemyies;i++)
		{
			if (pickup[i].active)
			{
				DrawRectangleV({ pickup[i].rect.x,pickup[i].rect.y }, { pickup[i].rect.width,pickup[i].rect.height }, BLACK);
			}

			if (heal[i].active)
			{
				DrawRectangleV({ heal[i].rect.x,heal[i].rect.y }, { heal[i].rect.width,heal[i].rect.height }, GREEN);
			}
		}

		//Coins
		for (int i = 0; i < maxCoins; i++)
		{
			if (coinsStr[i].active)
			{
				DrawRectangleRec(coinsStr[i].rect, GOLD);
			}
		}

		//GUI
		for (int i = 0; i < playerHp; i++)
		{
			DrawRectangle(18 + i * 9, screenHeight - 42, 12, 16, BLACK);
			DrawRectangle(20 + i * 9, screenHeight - 40, 8, 12, GREEN);
		}

		DrawText(TextFormat("Ammo: %d/%d", gunType[type].loadedAmmo, gunType[type].currentAmmo, maxHp), 20, screenHeight - 80, 20, BLACK);
		
		DrawText(TextFormat("Coins: %d", coins), 20, screenHeight - 120, 20, BLACK);

		EndDrawing();
	}
	else
	{
		BeginDrawing();
		ClearBackground(BLACK);
		Shop();
		EndDrawing();
	}
}

int main(void)
{
	InitGame();

	while (!WindowShouldClose())
	{
		float deltaTime = GetFrameTime();
		UpdateGame(deltaTime);
		DrawGame();
	}

	UnloadTexture(playerTxr);
	UnloadTexture(pistolTxr);
	UnloadTexture(sniperTxr);
	UnloadTexture(rifleTxr);
	CloseWindow();

	return 0;
}