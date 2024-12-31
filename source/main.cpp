#include "starter.h"
#include <iostream>
#include <vector>

const int TOP_SCREEN_WIDTH = 400;
const int BOTTOM_SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;

C3D_RenderTarget *topScreen = nullptr;
C3D_RenderTarget *bottomScreen = nullptr;

bool isGamePaused;

C2D_TextBuf scoreDynamicBuffer;
C2D_TextBuf livesDynamicBuffer;

C2D_TextBuf pauseStaticBuffer;
C2D_Text pauseTexts[1];

float textSize = 1.0f;

typedef struct
{
	Rectangle bounds;
	bool isDestroyed;
} Laser;

std::vector<Laser> playerLasers;
std::vector<Laser> alienLasers;

float lastTimePlayerShoot;
float lastTimeAliensShoot;

typedef struct
{
	Sprite sprite;
	int lives;
	int speed;
	int score;
} Player;

Player player;

typedef struct
{
	Sprite sprite;
	int points;
	int velocityX;
	bool shouldMove;
	bool isDestroyed;
} MysteryShip;

MysteryShip mysteryShip;

float lastTimeMysteryShipSpawn;

typedef struct
{
	Sprite sprite;
	int lives;
	bool isDestroyed;
} Structure;

std::vector<Structure> structures;

typedef struct
{
	Sprite sprite;
	int points;
	int velocity;
	bool isDestroyed;
} Alien;

std::vector<Alien> aliens;

bool shouldChangeVelocity;

Sprite alienSprite1;
Sprite alienSprite2;
Sprite alienSprite3;

std::vector<Alien> createAliens()
{
	alienSprite1 = loadSprite("alien_1.t3x", 0, 0, 16, 16);
	alienSprite2 = loadSprite("alien_2.t3x", 0, 0, 16, 16);
	alienSprite3 = loadSprite("alien_3.t3x", 0, 0, 16, 16);

	std::vector<Alien> aliens;

	// 5*11 Aliens
	aliens.reserve(55);

	int positionX;
	int positionY = 10;
	int alienPoints = 8;

	Sprite actualSprite;

	for (int row = 0; row < 5; row++)
	{
		positionX = 40;

		switch (row)
		{
		case 0:
			actualSprite = alienSprite3;
			break;

		case 1:
		case 2:
			actualSprite = alienSprite2;
			break;

		default:
			actualSprite = alienSprite1;
		}

		for (int columns = 0; columns < 11; columns++)
		{
			actualSprite.bounds.x = positionX;
			actualSprite.bounds.y = positionY;

			Alien actualAlien = {actualSprite, alienPoints, 1, false};

			aliens.push_back(actualAlien);
			positionX += 30;
		}

		alienPoints--;
		positionY += 25;
	}

	return aliens;
}

void aliensMovement()
{
	for (Alien &alien : aliens)
	{
		alien.sprite.bounds.x += alien.velocity;

		float alienPosition = alien.sprite.bounds.x + alien.sprite.bounds.w;

		if ((!shouldChangeVelocity && alienPosition > TOP_SCREEN_WIDTH) || alienPosition < alien.sprite.bounds.w)
		{
			shouldChangeVelocity = true;
			break;
		}
	}

	if (shouldChangeVelocity)
	{
		for (Alien &alien : aliens)
		{
			alien.velocity *= -1;
			alien.sprite.bounds.y += 5;
		}

		shouldChangeVelocity = false;
	}
}

void checkCollisionBetweenStructureAndLaser(Laser &laser)
{
	for (Structure &structure : structures)
	{
		if (!structure.isDestroyed && hasCollision(structure.sprite.bounds, laser.bounds))
		{
			laser.isDestroyed = true;

			structure.lives--;

			if (structure.lives == 0)
			{
				structure.isDestroyed = true;
			}

			// Mix_PlayChannel(-1, explosionSound, 0);

			break;
		}
	}
}

void removeDestroyedElements()
{
	for (auto iterator = aliens.begin(); iterator != aliens.end();)
	{
		if (iterator->isDestroyed)
		{
			aliens.erase(iterator);
		}
		else
		{
			iterator++;
		}
	}

	for (auto iterator = playerLasers.begin(); iterator != playerLasers.end();)
	{
		if (iterator->isDestroyed)
		{
			playerLasers.erase(iterator);
		}
		else
		{
			iterator++;
		}
	}

	for (auto iterator = alienLasers.begin(); iterator != alienLasers.end();)
	{
		if (iterator->isDestroyed)
		{
			alienLasers.erase(iterator);
		}
		else
		{
			iterator++;
		}
	}
}

void update()
{
	int keyHeld = hidKeysHeld();

	if (keyHeld & KEY_LEFT && player.sprite.bounds.x > 0)
	{
		player.sprite.bounds.x -= player.speed;
	}

	else if (keyHeld & KEY_RIGHT && player.sprite.bounds.x < TOP_SCREEN_WIDTH - player.sprite.bounds.w)
	{
		player.sprite.bounds.x += player.speed;
	}

	if (!mysteryShip.shouldMove)
	{
		lastTimeMysteryShipSpawn++;

		if (lastTimeMysteryShipSpawn >= 300)
		{
			lastTimeMysteryShipSpawn = 0;

			mysteryShip.shouldMove = true;
		}
	}

	if (mysteryShip.shouldMove)
	{
		if (mysteryShip.sprite.bounds.x > TOP_SCREEN_WIDTH + mysteryShip.sprite.bounds.w || mysteryShip.sprite.bounds.x < -80)
		{
			mysteryShip.velocityX *= -1;
			mysteryShip.shouldMove = false;
		}

		mysteryShip.sprite.bounds.x += mysteryShip.velocityX;
	}

	if (keyHeld & KEY_A)
	{
		lastTimePlayerShoot++;

		if (lastTimePlayerShoot >= 15)
		{
			Rectangle laserBounds = {player.sprite.bounds.x + 10, player.sprite.bounds.y - player.sprite.bounds.h, 0, 2, 8, YELLOW};

			playerLasers.push_back({laserBounds, false});

			lastTimePlayerShoot = 0;

			// Mix_PlayChannel(-1, laserSound, 0);
		}
	}

	for (Laser &laser : playerLasers)
	{
		laser.bounds.y -= 3;

		if (laser.bounds.y < 0)
			laser.isDestroyed = true;

		if (!mysteryShip.isDestroyed && hasCollision(mysteryShip.sprite.bounds, laser.bounds))
		{
			laser.isDestroyed = true;
			mysteryShip.isDestroyed = true;

			player.score += mysteryShip.points;

			// Mix_PlayChannel(-1, explosionSound, 0);

			break;
		}

		for (Alien &alien : aliens)
		{
			if (!alien.isDestroyed && hasCollision(alien.sprite.bounds, laser.bounds))
			{
				alien.isDestroyed = true;
				laser.isDestroyed = true;

				player.score += alien.points;

				// Mix_PlayChannel(-1, explosionSound, 0);

				break;
			}
		}

		checkCollisionBetweenStructureAndLaser(laser);
	}

	lastTimeAliensShoot++;

	if (aliens.size() > 0 && lastTimeAliensShoot >= 30)
	{
		int randomAlienIndex = rand() % aliens.size();

		Alien alienShooter = aliens[randomAlienIndex];

		Rectangle laserBounds = {alienShooter.sprite.bounds.x + 10, alienShooter.sprite.bounds.y + alienShooter.sprite.bounds.h, 0, 2, 8, YELLOW};

		alienLasers.push_back({laserBounds, false});

		lastTimeAliensShoot = 0;

		// Mix_PlayChannel(-1, laserSound, 0);
	}

	for (Laser &laser : alienLasers)
	{
		laser.bounds.y += 3;

		if (laser.bounds.y > SCREEN_HEIGHT)
			laser.isDestroyed = true;

		if (player.lives > 0 && hasCollision(player.sprite.bounds, laser.bounds))
		{
			laser.isDestroyed = true;

			player.lives--;

			// Mix_PlayChannel(-1, explosionSound, 0);

			break;
		}

		checkCollisionBetweenStructureAndLaser(laser);
	}

	aliensMovement();

	removeDestroyedElements();
}

void renderTopScreen()
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(topScreen, BROWN);
	C2D_SceneBegin(topScreen);

	if (!mysteryShip.isDestroyed)
	{
		renderSprite(mysteryShip.sprite);
	}

	for (Alien &alien : aliens)
	{
		if (!alien.isDestroyed)
		{
			renderSprite(alien.sprite);
		}
	}

	for (Laser &laser : alienLasers)
	{
		if (!laser.isDestroyed)
		{
			drawRectangle(laser.bounds);
		}
	}

	for (Laser &laser : playerLasers)
	{
		if (!laser.isDestroyed)
		{
			drawRectangle(laser.bounds);
		}
	}

	for (Structure &structure : structures)
	{
		if (!structure.isDestroyed)
		{
			renderSprite(structure.sprite);
		}
	}

	renderSprite(player.sprite);

	C3D_FrameEnd(0);
}

void renderBottomScreen()
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(bottomScreen, BLACK);
	C2D_SceneBegin(bottomScreen);

	drawDynamicText("score: %d", player.score, scoreDynamicBuffer, 90, 20, textSize);
	drawDynamicText("lives: %d", player.lives, livesDynamicBuffer, 250, 20, textSize);

	if (isGamePaused)
	{
		C2D_DrawText(&pauseTexts[0], C2D_AtBaseline | C2D_WithColor, 80, 100, 0, textSize, textSize, WHITE);
	}

	C3D_FrameEnd(0);
}

int main(int argc, char *argv[])
{
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	topScreen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	bottomScreen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	pauseStaticBuffer = C2D_TextBufNew(1024);
	C2D_TextParse(&pauseTexts[0], pauseStaticBuffer, "Game Paused");
	C2D_TextOptimize(&pauseTexts[0]);

	scoreDynamicBuffer = C2D_TextBufNew(4096);
	livesDynamicBuffer = C2D_TextBufNew(4096);

	aliens = createAliens();

	Sprite shipSprite = loadSprite("mystery.t3x", TOP_SCREEN_WIDTH, 20, 22, 14);
	mysteryShip = {shipSprite, 50, -3, false, false};

	Sprite playerSprite = loadSprite("spaceship.t3x", TOP_SCREEN_WIDTH / 2, SCREEN_HEIGHT - 20, 22, 14);
	player = {playerSprite, 2, 10, 0};

	Sprite structureSprite = loadSprite("structure.t3x", 50, SCREEN_HEIGHT - 50, 28, 17);

	Rectangle structureBounds2 = {150, SCREEN_HEIGHT - 50, 0, 28, 17, WHITE};
	Rectangle structureBounds3 = {250, SCREEN_HEIGHT - 50, 0, 28, 17, WHITE};
	Rectangle structureBounds4 = {330, SCREEN_HEIGHT - 50, 0, 28, 17, WHITE};

	structures.push_back({structureSprite, 5, false});
	structures.push_back({{structureSprite.texture, structureBounds2}, 5, false});
	structures.push_back({{structureSprite.texture, structureBounds3}, 5, false});
	structures.push_back({{structureSprite.texture, structureBounds4}, 5, false});

	while (aptMainLoop())
	{
		hidScanInput();

		int keyDown = hidKeysDown();

		if (keyDown & KEY_START)
		{
			isGamePaused = !isGamePaused;
		}

		if (!isGamePaused)
		{
			update();
		}

		renderTopScreen();

		renderBottomScreen();
	}

	// removing text.
	C2D_TextBufDelete(scoreDynamicBuffer);
	C2D_TextBufDelete(livesDynamicBuffer);
	C2D_TextBufDelete(pauseStaticBuffer);

	// removing loaded sheets.
	C2D_SpriteSheetFree(mysteryShip.sprite.sheet);
	C2D_SpriteSheetFree(alienSprite1.sheet);
	C2D_SpriteSheetFree(alienSprite2.sheet);
	C2D_SpriteSheetFree(alienSprite3.sheet);
	C2D_SpriteSheetFree(structureSprite.sheet);
	C2D_SpriteSheetFree(player.sprite.sheet);

	// closing library.
	C2D_Fini();
	C3D_Fini();
	gfxExit();
}
