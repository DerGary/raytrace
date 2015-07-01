#pragma once

#define black Color()

class Color
{
public:
	double r, g, b;

	Color(void) : r(0.0), g(0.0), b(0.0) {};
	Color(double _r, double _g, double _b) : r(_r), g(_g), b(_b) {};

	Color addcolor(Color &c);
	Color outprodc(Color &c);
	Color scmpy(double s);

};

