#pragma once
#include "Color.h"
#include "string"

class Property
{
	Color reflectance;
	Color ambient;
	double specular, mirror;
	std::string name;
public:
	Property(void) : reflectance(), specular(0.0), mirror(0.0) {};
	Property(char *n, Color &a, Color &r, double s, double m) : name(n), ambient(a), reflectance(r), specular(s), mirror(m) {};

	Color getAmbient() { return ambient; };
	Color getReflectance() { return reflectance; };
	double getSpecular() { return specular; };
	double getMirror() { return mirror; };
	std::string getName() { return name; };
};

