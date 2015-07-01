#include "StdAfx.h"
#include "Ray.h"
#include "math.h"
#include "float.h"

using namespace std;

/*----------------------------------------------------------------------------*/
/* shade   arbeitet den folgenden Algorithmus ab:                             */
/*   Schneide den Sichtstrahl mit jedem Objekt. Merke, welches Objekt den     */
/*   kleinsten Parameterwert fuer den Schnittpunkt liefert. Wurde kein Objekt */
/*   geschnitten, dann setze den Farbwert auf Hintergrund. Von dem kleinsten  */
/*   Schnittpunkt aus, bestimme Strahl zu jeder Lichtquelle. Trifft dieser    */
/*   Strahl ein Objekt, dann traegt diese Lichtquelle nicht zur Beleuchtung   */
/*   bei. Bestimme den Farbwert nach der Lichtgleichung.                      */
/*   Dann berechne den Reflektionsstrahl und fuehre Beleuchtungsberechnung    */
/*   auch fuer diesen Strahl durch und addiere dessen Farbwerte zu den        */
/*   bereits bestimmten Farbwerten.                                           */
/* Rueckgabeparameter: Farbe, die auf diesem Strahl zu sehen ist              */
/*----------------------------------------------------------------------------*/

Color Ray::shade(vector<Objekt> &objects, vector<Light> &lights)
{
	Objekt *closest = NULL;
	Color cur_color; 
	double min_t = DBL_MAX, t;

	Vector intersection_position,	normal;
	Ray lv, reflected_ray;
	bool something_intersected = false;

	for (vector<Objekt>::iterator o = objects.begin(); o != objects.end(); ++o) {

		t = intersect(*o);
		if (0.0 < t && t < min_t) {
			min_t = t;
			closest = &(*o);
		}
	}

	if (closest == NULL) {
		if (depth == 0)
			cur_color = Color(background.r,background.g,background.b); //background_color;
		else
			cur_color = black;
	} else {
		intersection_position = origin.vadd(direction.svmpy(min_t));
		normal = closest->get_normal(intersection_position);
		reflected_ray = reflect(intersection_position, normal);
		cur_color = closest->getProperty().getAmbient().outprodc(Color(ambience.r, ambience.g, ambience.b));  // black statt Globales Ambient

		for (vector<Light>::iterator li = lights.begin(); li != lights.end(); ++li) {
			lv.setDirection(li->getDirection());
			lv.setOrigin(intersection_position);
			something_intersected = false;
			for (vector<Objekt>::iterator o = objects.begin(); o != objects.end(); ++o) {
				t = lv.intersect(*o);
				if (t > 0.0) {
					something_intersected = true;
					break;
				}
			}
			if (something_intersected == false) {
				Color new_color = shaded_color(&(*li), reflected_ray, normal, closest);
				cur_color = cur_color.addcolor(new_color);
			} 
		}

		if (depth < 5) {
			Color mirror_color = reflected_ray.shade(objects, lights);
			mirror_color = mirror_color.scmpy(closest->getProperty().getMirror());
			cur_color = mirror_color.addcolor(cur_color);
		}
	}
	return(cur_color);
} /* shade() */


/*----------------------------------------------------------------------------*/
/* shaded_color   berechnet die Farbe im Treffpunkt, die durch die           */
/*   Lichtquelle hervorgerufen wird.                                          */
/* Aufrufparameter:    Lichtquelle                                            */
/*                     Reflexionsstrahl                                       */
/*                     Normalenvektor im Treffpunkt                           */
/*                     aktuelles Objekt                                       */
/* Rueckgabeparameter: errechnete Farbe                                       */
/*----------------------------------------------------------------------------*/

Color Ray::shaded_color(Light *light, Ray &reflectedray, Vector &normal, Objekt *obj)
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
		specular =  light->getColor().scmpy(spec);
		reflected_color = reflected_color.addcolor(specular);
	}

	return reflected_color;
} /* shaded_color() */

/*----------------------------------------------------------------------------*/
/* reflect   berechnet einen Reflektionsstrahl aus dem einfallenden Strahl an */
/*    dem Reflektionspunkt mit Hilfe des Normalenvektors an dieser Stelle.    */
/* Aufrufparameter:    einfallender Strahl                                    */
/*                     Reflektionspunkt                                       */
/*                     Normalenvektor                                         */
/* Rueckgabeparameter: Reflektionsstrahl                                      */
/*----------------------------------------------------------------------------*/

Ray Ray::reflect(Vector &origin, Vector &normal)
{
	Ray	 reflection;
	double	 incdot;

	incdot = normal.dot(direction);
	reflection.origin = origin;
	reflection.direction = normal.svmpy(2.0*incdot);
	reflection.direction = direction.vsub(reflection.direction).normalize();

	reflection.depth = depth + 1;
	return(reflection);
} /* reflect() */

double Ray::intersect(Objekt &obj)
{
	double a, b, c, d, e, f, g, h, j, k, t = -1.0,
		acoef, bcoef, ccoef, root, disc;

	a = obj.getSurface().a; 
	b = obj.getSurface().b; 
	c = obj.getSurface().c;
	d = obj.getSurface().d; 
	e = obj.getSurface().e; 
	f = obj.getSurface().f;
	g = obj.getSurface().g; 
	h = obj.getSurface().h; 
	j = obj.getSurface().j;
	k = obj.getSurface().k;

	acoef = Vector(direction.dot(Vector(a, b, c)),
		e*direction.y + f*direction.z,
		h*direction.z).dot(direction);

	bcoef = 
		Vector(d, g, j).dot(direction) + 
		origin.dot( Vector( 
		direction.dot(Vector(a+a , b, c)), 
		direction.dot(Vector(b, e+e, f)), 
		direction.dot(Vector(c, f, h+h))));

	ccoef = origin.dot( 
		Vector(Vector(a, b, c).dot(origin) + d,
		e*origin.y + f*origin.z + g,
		h*origin.z + j)) + k;

	if (1.0 + acoef != 1.0) {
		disc = bcoef * bcoef - 4 * acoef * ccoef;
		if (disc > -DBL_EPSILON) {
			root = sqrt( disc );
			t = ( -bcoef - root ) / ( acoef + acoef );
			if (t < 0.0) {
				t = ( -bcoef + root ) / ( acoef + acoef );
			}
		}
	}
	return ((0.001 < t) ? t : -1.0);
} /* intersect() */

