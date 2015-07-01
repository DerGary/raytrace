#pragma once
#include "Vector.h"
#include "Color.h"

class Light
{
	Vector	 direction;
	Color	 color;
public:
	Light(void): direction(), color() {};
	Light(Vector &v, Color &c): direction(v), color(c) {};

	Vector getDirection() { return direction; };
	Color getColor() { return color; };
};


