#include <citro2d.h>
#include <iostream>
#include <vector>

// the 3ds has different screen width, but the same screen height.
const int TOP_SCREEN_WIDTH = 400;
const int BOTTOM_SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;

C3D_RenderTarget *topScreen = nullptr;
C3D_RenderTarget *bottomScreen = nullptr;

bool isGamePaused;

int collisionCounter;

C2D_TextBuf textDynamicBuffer;

C2D_TextBuf textStaticBuffer;
C2D_Text staticTexts[1];

float textSize = 1.0f;

// Create colors
const u32 WHITE = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
const u32 BLACK = C2D_Color32(0x00, 0x00, 0x00, 0x00);
const u32 GREEN = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);
const u32 RED = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
const u32 BLUE = C2D_Color32(0x00, 0x00, 0xFF, 0xFF);

typedef struct
{
	float x;
	float y;
	float z;
	float w;
	float h;
	unsigned int color;
} Rectangle;

typedef struct
{
	C2D_Image texture;
	Rectangle textureBounds;
} Sprite;

Sprite shipSprite;
Sprite playerSprite;
Sprite alienSprite1;
Sprite alienSprite2;
Sprite alienSprite3;
Sprite structureSprite;

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
	float x;
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
	float x;
	Sprite sprite;
	int points;
	int velocity;
	bool isDestroyed;
} Alien;

std::vector<Alien> aliens;

bool shouldChangeVelocity = false;

C2D_SpriteSheet sheet;

// try to load make work the filePath
// Sprite loadSprite(const char *filePath, float positionX, float positionY, float width, float height)
// {
//     Rectangle bounds = {positionX, positionY, 0, width, height};

// 	std::string completePath = "romfs:/gfx/" + filePath;

// 	C2D_SpriteSheet sheet = C2D_SpriteSheetLoad(completePath);
// 	C2D_Image image = C2D_SpriteSheetGetImage(sheet, 0);

//     Sprite sprite = {image, bounds}; 

//     return sprite;
// }

std::vector<Alien> createAliens()
{
	C2D_SpriteSheet sheet = C2D_SpriteSheetLoad("romfs:/gfx/alien_1.t3x");
	C2D_Image sprite = C2D_SpriteSheetGetImage(sheet, 0);

	C2D_SpriteSheet sheet2 = C2D_SpriteSheetLoad("romfs:/gfx/alien_2.t3x");
	C2D_Image sprite2 = C2D_SpriteSheetGetImage(sheet2, 0);

	C2D_SpriteSheet sheet3 = C2D_SpriteSheetLoad("romfs:/gfx/alien_3.t3x");
	C2D_Image sprite3 = C2D_SpriteSheetGetImage(sheet3, 0);

	Rectangle initialBounds = {0, 0, 0, 16, 16, WHITE};

	// alienSprite1 = loadSprite("alien_1.t3x", 0, 0, 16, 16);
	alienSprite1 = {sprite, initialBounds};
	alienSprite2 = {sprite2, initialBounds};
	alienSprite3 = {sprite3, initialBounds};

	std::vector<Alien> aliens;

	// 5*11 Aliens
	aliens.reserve(55);

	int positionX;
	int positionY = 25;
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
			actualSprite.textureBounds.x = positionX;
			actualSprite.textureBounds.y = positionY;

			Alien actualAlien = {(float)positionX, actualSprite, alienPoints, 1, false};

			aliens.push_back(actualAlien);
			positionX += 30;
		}

		alienPoints--;
		positionY += 25;
	}

	return aliens;
}

void aliensMovement(float deltaTime)
{
	for (Alien &alien : aliens)
	{
		alien.x += alien.velocity;
		alien.sprite.textureBounds.x = alien.x;

		float alienPosition = alien.sprite.textureBounds.x + alien.sprite.textureBounds.w;

		if ((!shouldChangeVelocity && alienPosition > TOP_SCREEN_WIDTH) || alienPosition < alien.sprite.textureBounds.w)
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
			alien.sprite.textureBounds.y += 5;
		}

		shouldChangeVelocity = false;
	}
}

bool hasCollision(Rectangle &bounds, Rectangle &ball)
{
	return bounds.x < ball.x + ball.w && bounds.x + bounds.w > ball.x &&
		   bounds.y < ball.y + ball.h && bounds.y + bounds.h > ball.y;
}

void checkCollisionBetweenStructureAndLaser(Laser &laser)
{
	for (Structure &structure : structures)
	{
		if (!structure.isDestroyed && hasCollision(structure.sprite.textureBounds, laser.bounds))
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

	if (keyHeld & KEY_LEFT && player.sprite.textureBounds.x > 0)
	{
		player.sprite.textureBounds.x -= player.speed /* deltaTime*/;
	}

	else if (keyHeld & KEY_RIGHT && player.sprite.textureBounds.x < TOP_SCREEN_WIDTH - player.sprite.textureBounds.w)
	{
		player.sprite.textureBounds.x += player.speed /* deltaTime*/;
	}

	if (!mysteryShip.shouldMove)
	{
		// lastTimeMysteryShipSpawn += deltaTime;

		if (lastTimeMysteryShipSpawn >= 10)
		{
			lastTimeMysteryShipSpawn = 0;

			mysteryShip.shouldMove = true;
		}
	}

	if (mysteryShip.shouldMove)
	{
		if (mysteryShip.sprite.textureBounds.x > TOP_SCREEN_WIDTH + mysteryShip.sprite.textureBounds.w || mysteryShip.sprite.textureBounds.x < -80)
		{
			mysteryShip.velocityX *= -1;
			mysteryShip.shouldMove = false;
		}

		// mysteryShip.x += mysteryShip.velocityX * deltaTime;
		mysteryShip.sprite.textureBounds.x = mysteryShip.x;
	}

	// if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A))
	// {
	//     lastTimePlayerShoot += deltaTime;

	//     if (lastTimePlayerShoot >= 0.35)
	//     {
	//         SDL_Rect laserBounds = {player.sprite.textureBounds.x + 10, player.sprite.textureBounds.y - player.sprite.textureBounds.h, 2, 8};

	//         playerLasers.push_back({laserBounds, false});

	//         lastTimePlayerShoot = 0;

	//         Mix_PlayChannel(-1, laserSound, 0);
	//     }
	// }

	for (Laser &laser : playerLasers)
	{
		// laser.bounds.y -= 200 * deltaTime;

		if (laser.bounds.y < 0)
			laser.isDestroyed = true;

		if (!mysteryShip.isDestroyed && hasCollision(mysteryShip.sprite.textureBounds, laser.bounds))
		{
			laser.isDestroyed = true;
			mysteryShip.isDestroyed = true;

			player.score += mysteryShip.points;

			// std::string scoreString = "score: " + std::to_string(player.score);

			// updateTextureText(scoreTexture, scoreString.c_str(), fontSquare, renderer);

			// Mix_PlayChannel(-1, explosionSound, 0);

			break;
		}

		for (Alien &alien : aliens)
		{
			if (!alien.isDestroyed && hasCollision(alien.sprite.textureBounds, laser.bounds))
			{
				alien.isDestroyed = true;
				laser.isDestroyed = true;

				player.score += alien.points;

				// std::string scoreString = "score: " + std::to_string(player.score);

				// updateTextureText(scoreTexture, scoreString.c_str(), fontSquare, renderer);

				// Mix_PlayChannel(-1, explosionSound, 0);

				break;
			}
		}

		checkCollisionBetweenStructureAndLaser(laser);
	}

	// lastTimeAliensShoot += deltaTime;

	if (aliens.size() > 0 && lastTimeAliensShoot >= 0.6)
	{
		int randomAlienIndex = rand() % aliens.size();

		Alien alienShooter = aliens[randomAlienIndex];

		Rectangle laserBounds = {alienShooter.sprite.textureBounds.x + 10, alienShooter.sprite.textureBounds.y + alienShooter.sprite.textureBounds.h, 0, 2, 8, WHITE};

		alienLasers.push_back({laserBounds, false});

		lastTimeAliensShoot = 0;

		// Mix_PlayChannel(-1, laserSound, 0);
	}

	for (Laser &laser : alienLasers)
	{
		// laser.bounds.y += 200 * deltaTime;

		if (laser.bounds.y > SCREEN_HEIGHT)
			laser.isDestroyed = true;

		if (player.lives > 0 && hasCollision(player.sprite.textureBounds, laser.bounds))
		{
			laser.isDestroyed = true;

			player.lives--;

			// std::string liveString = "lives: " + std::to_string(player.lives);

			// updateTextureText(liveTexture, liveString.c_str(), fontSquare, renderer);

			// Mix_PlayChannel(-1, explosionSound, 0);

			break;
		}

		checkCollisionBetweenStructureAndLaser(laser);
	}

	aliensMovement(0);

	removeDestroyedElements();
}

void renderSprite(Sprite &sprite)
{
	C2D_DrawImageAt(sprite.texture, sprite.textureBounds.x, sprite.textureBounds.y, 0, NULL, 1, 1);
}

void renderTopScreen()
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(topScreen, BLACK);
	C2D_SceneBegin(topScreen);

	for (Alien &alien : aliens)
	{
		if (!alien.isDestroyed)
		{
			renderSprite(alien.sprite);
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

	// C2D_DrawRectSolid(bottomBounds.x, bottomBounds.y, bottomBounds.z, bottomBounds.w, bottomBounds.h, bottomBounds.color);

	C2D_TextBufClear(textDynamicBuffer);

	// Generate and draw dynamic text
	char buf[160];
	C2D_Text dynamicText;
	snprintf(buf, sizeof(buf), "Total collisions: %d", collisionCounter);
	C2D_TextParse(&dynamicText, textDynamicBuffer, buf);
	C2D_TextOptimize(&dynamicText);
	C2D_DrawText(&dynamicText, C2D_AlignCenter | C2D_WithColor, 150, 175, 0, textSize, textSize, WHITE);

	if (isGamePaused)
	{
		C2D_DrawText(&staticTexts[0], C2D_AtBaseline | C2D_WithColor, 110, 60, 0, textSize, textSize, WHITE);
	}

	C3D_FrameEnd(0);
}

int main(int argc, char *argv[])
{
	// Init libs
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	// Create top and bottom screens
	topScreen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	bottomScreen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	// Create two text buffers: one for static text, and another one for
	// dynamic text - the latter will be cleared at each frame.
	textStaticBuffer = C2D_TextBufNew(1024); // support up to 4096 glyphs in the buffer
	textDynamicBuffer = C2D_TextBufNew(4096);

	// Parse the static text strings
	C2D_TextParse(&staticTexts[0], textStaticBuffer, "Game Paused");

	// Optimize the static text strings
	C2D_TextOptimize(&staticTexts[0]);

	aliens = createAliens();

	C2D_SpriteSheet playerSheet = C2D_SpriteSheetLoad("romfs:/gfx/spaceship.t3x");

	C2D_Image playerSprite = C2D_SpriteSheetGetImage(playerSheet, 0);

	Rectangle playerBounds = {100, SCREEN_HEIGHT - 20, 0, 22, 14, WHITE};

	Sprite sprite = {playerSprite, playerBounds};
	player = {sprite, 2, 10, 0};

	touchPosition touch;

	// Main loop
	while (aptMainLoop())
	{
		hidScanInput();

		// Read the touch screen coordinates
		hidTouchRead(&touch);

		// touch.px
		// touch.py

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

	// Delete the text buffers
	C2D_TextBufDelete(textDynamicBuffer);
	C2D_TextBufDelete(textStaticBuffer);

	// Deinit libs
	C2D_Fini();
	C3D_Fini();
	gfxExit();
}
