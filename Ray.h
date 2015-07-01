#pragma once
#include "Vector.h"
#include "Color.h"
#include "Light.h"
#include "Objekt.h"
#include "vector"
#include "Polygon.h"

class Ray
{
	Vector direction, origin;
	int	 depth;
	Color background;
	Color ambience;
	//Color shaded_color(Light *light, Ray &reflectedray, Vector &normal, Objekt *obj);

public:
	Ray(void) : direction(), origin(), depth (0) {};
	Ray(Vector &dir, Vector &orig, int d) : direction(dir), origin(orig), depth (d) {};
	Color shade(std::vector<Objekt*> &, std::vector<Light> &);

	Vector& getDirection() { return direction; };
	Vector& getOrigin() { return origin; };
	int getDepth() { return depth; };

	void setDirection(Vector &v) { direction=v; };
	void setOrigin(Vector &v) { origin=v; };
	void setDepth(int d) { depth=d; };
	void setBackgroundColor(Color& color){ background = color; };
	void setGlobalAmbience(Color& color){ ambience = color; };

	Ray reflect(Vector &origin, Vector &normal);
	double intersect(Objekt* obj);
	float Ray::intersectTriangle(PolygonObjekt* poly);

	/*----------------------------------------------------------------------------*/
	/* shaded_color   berechnet die Farbe im Treffpunkt, die durch die           */
	/*   Lichtquelle hervorgerufen wird.                                          */
	/* Aufrufparameter:    Lichtquelle                                            */
	/*                     Reflexionsstrahl                                       */
	/*                     Normalenvektor im Treffpunkt                           */
	/*                     aktuelles Objekt                                       */
	/* Rueckgabeparameter: errechnete Farbe                                       */
	/*----------------------------------------------------------------------------*/
	template<class T>
	Color shaded_color(Light *light, Ray &reflectedray, Vector &normal, T *obj)
	{
		Color reflected_color;
		Color specular;
		Color lambert;
		double spec;

		double ldot;
		ldot = light->getDirection().dot(normal);
		reflected_color = black;
		if (1.0 + ldot > 1.0) {
			lambert = light->getColor().scmpy(ldot);
			reflected_color = lambert.outprodc(obj->getProperty().getReflectance());
		}
		spec = reflectedray.getDirection().dot(light->getDirection());

		if (1.0 + spec > 1.0) {

			spec = spec * spec;
			spec = spec * spec;
			spec = spec * spec;
			spec *= obj->getProperty().getSpecular();
			specular = light->getColor().scmpy(spec);
			reflected_color = reflected_color.addcolor(specular);
		}

		return reflected_color;
	} /* shaded_color() */

};


