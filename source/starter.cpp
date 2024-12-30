#include "starter.h"

void drawRectangle(Rectangle &rectangle)
{
	// glBoxFilled(rectangle.x, rectangle.y, rectangle.x + rectangle.w, rectangle.y + rectangle.h, rectangle.color);
}

bool hasCollision1(Rectangle &bounds, Rectangle &ball)
{
	return bounds.x < ball.x + ball.w && bounds.x + bounds.w > ball.x &&
		   bounds.y < ball.y + ball.h && bounds.y + bounds.h > ball.y;
}