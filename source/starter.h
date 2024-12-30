#pragma once

#include <citro2d.h>

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
	Rectangle bounds;
} Sprite;

void drawRectangle(Rectangle &rectangle);

bool hasCollision(Rectangle &rectangle, Rectangle &rectangle2);

Sprite loadSprite(const char *filePath, float positionX, float positionY, float width, float height);

void renderSprite(Sprite &sprite);