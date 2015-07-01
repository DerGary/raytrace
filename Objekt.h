#pragma once
#include "Surface.h"
#include "Property.h"
#include "Vector.h"
#include "vector"


class Objekt
{
	Surface *surface;
	Property *properties;
public:
	Objekt(void) : surface(NULL), properties(NULL) {};
	Objekt(Surface *s, Property *p) : surface(s), properties(p) {};

	virtual Property getProperty()  { return *properties; };
	virtual Surface& getSurface()  { return *surface; };

	virtual Vector get_normal(Vector &v);
};

