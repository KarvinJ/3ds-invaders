#pragma once

#include <citro2d.h>

typedef struct
{
	float x;
	float y;
	float w;
	float h;
	unsigned int color;
} Rectangle;

void drawRectangle(Rectangle &rectangle);

bool hasCollision1(Rectangle &bounds, Rectangle &ball);

void initSubSprites();
