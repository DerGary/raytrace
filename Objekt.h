#pragma once
#include "Surface.h"
#include "Property.h"
#include "Vector.h"
#include "vector"


class Objekt
{
	Surface *surface;
protected:
	Property *properties;
public:
	Objekt(void) : surface(NULL), properties(NULL) {};
	Objekt(Surface *s, Property *p) : surface(s), properties(p) {};

	virtual Property getProperty()  { return *properties; };

	virtual float intersect(Vector &direction, Vector& origin, Surface** s)
	{
		double a, b, c, d, e, f, g, h, j, k, t = -1.0,
			acoef, bcoef, ccoef, root, disc;

		a = surface->a;
		b = surface->b;
		c = surface->c;
		d = surface->d;
		e = surface->e;
		f = surface->f;
		g = surface->g;
		h = surface->h;
		j = surface->j;
		k = surface->k;

		acoef = Vector(direction.dot(Vector(a, b, c)),
			e*direction.y + f*direction.z,
			h*direction.z).dot(direction);

		bcoef =
			Vector(d, g, j).dot(direction) +
			origin.dot(Vector(
			direction.dot(Vector(a + a, b, c)),
			direction.dot(Vector(b, e + e, f)),
			direction.dot(Vector(c, f, h + h))));

		ccoef = origin.dot(
			Vector(Vector(a, b, c).dot(origin) + d,
			e*origin.y + f*origin.z + g,
			h*origin.z + j)) + k;

		if (1.0 + acoef != 1.0) {
			disc = bcoef * bcoef - 4 * acoef * ccoef;
			if (disc > -DBL_EPSILON) {
				root = sqrt(disc);
				t = (-bcoef - root) / (acoef + acoef);
				if (t < 0.0) {
					t = (-bcoef + root) / (acoef + acoef);
				}
			}
		}
		if (t > 0.001)
		{
			*s = surface;
			return t;
		}
		return -1.0;
	};
	
};

