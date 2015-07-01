#pragma once
#include "Vector.h"
#include <vector>

class PolygonSurface : public Surface
{
private:
	std::string name;


public:
	std::vector<Vector> vertices;
	PolygonSurface(char* name, std::vector<Vector> vertices) : name(name), vertices(vertices){}
	std::string getName(){ return name; }
};
class PolygonObjekt : public Objekt
{
	PolygonSurface *surface;
	Property *properties;
public:
	PolygonObjekt(void) : surface(NULL), properties(NULL) {};
	PolygonObjekt(PolygonSurface *s, Property *p) : surface(s), properties(p) {};

	Property getProperty() override { return *properties; };
	PolygonSurface& getSurface() override { return *surface; };

	Vector get_normal(Vector &v) override{
		Vector a, b;
		a = surface->vertices.at(0).vsub(surface->vertices.at(1));
		b = surface->vertices.at(0).vsub(surface->vertices.at(2));
		return a.cross(b).cross(v);
	};
};