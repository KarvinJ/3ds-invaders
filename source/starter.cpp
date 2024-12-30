#include "starter.h"

void drawRectangle(Rectangle &rectangle)
{
	C2D_DrawRectSolid(rectangle.x, rectangle.y, rectangle.z, rectangle.w, rectangle.h, rectangle.color);
}

bool hasCollision(Rectangle &rectangle, Rectangle &rectangle2)
{
	return rectangle.x < rectangle2.x + rectangle2.w && rectangle.x + rectangle.w > rectangle2.x &&
		   rectangle.y < rectangle2.y + rectangle2.h && rectangle.y + rectangle.h > rectangle2.y;
}

Sprite loadSprite(const char *filePath, float positionX, float positionY, float width, float height)
{
	Rectangle bounds = {positionX, positionY, 0, width, height};

	C2D_SpriteSheet sheet = C2D_SpriteSheetLoad(filePath);
	C2D_Image image = C2D_SpriteSheetGetImage(sheet, 0);

	Sprite sprite = {image, bounds};

	return sprite;
}