#include "StdAfx.h"
#include "Ray.h"
#include "math.h"
#include "float.h"
#include "Polygon.h"

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

Color Ray::shade(vector<Objekt*> &objects, vector<Light> &lights)
{
	Objekt *closest = NULL;
	Color cur_color; 
	double min_t = DBL_MAX, t;

	Vector intersection_position,	normal;
	Ray lv, reflected_ray;
	bool something_intersected = false;

	for (vector<Objekt*>::iterator o = objects.begin(); o != objects.end(); ++o) {
		t = intersect(*o);
		if (0.0 < t && t < min_t) {
			min_t = t;
			closest = *o;
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
			for (vector<Objekt*>::iterator o = objects.begin(); o != objects.end(); ++o) {
				t = lv.intersect(*o);
				if (t > 0.0) {
					something_intersected = true;
					break;
				}
			}
			if (something_intersected == false) {
				PolygonObjekt* closestPoly = dynamic_cast<PolygonObjekt*> (closest);
				Color new_color;
				if (closestPoly == NULL)
					new_color = shaded_color<Objekt>(&(*li), reflected_ray, normal, closest);
				else
					new_color = shaded_color<PolygonObjekt>(&(*li), reflected_ray, normal, closestPoly);
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

double Ray::intersect(Objekt* obj)
{
	PolygonObjekt* poly = dynamic_cast<PolygonObjekt*>(obj);
	if (poly == NULL)
	{
		double a, b, c, d, e, f, g, h, j, k, t = -1.0,
			acoef, bcoef, ccoef, root, disc;

		a = obj->getSurface().a;
		b = obj->getSurface().b;
		c = obj->getSurface().c;
		d = obj->getSurface().d;
		e = obj->getSurface().e;
		f = obj->getSurface().f;
		g = obj->getSurface().g;
		h = obj->getSurface().h;
		j = obj->getSurface().j;
		k = obj->getSurface().k;

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
		return ((0.001 < t) ? t : -1.0);
	} 
	else
	{
		return intersectTriangle(poly);
	}

} /* intersect() */

//float Ray::intersectTriangle(PolygonObjekt* poly)
//{
//	Vector P = getOrigin(), D = getDirection();
//	//Vector V0, V1, V2;
//
//	//Vector E1 = V1.vsub(V0);
//	//Vector E2 = V2.vsub(V0);
//
//	//Vector H = D.cross(E2);
//
//	//float a = E1.dot(H);
//
//	////ray is parallel to the triangle plane
//	//if ((a > -0.00001) && (a < 0.00001))
//	//	return -1.0;
//
//	//float f = 1 / a;
//
//	//Vector S = P.vsub(V0);
//
//	//float u = f * S.dot(H);
//
//	////ray intersected the triangle but outside of the triangle
//	//if ((u < 0.0) || (u > 1.0))
//	//	return -1.0;
//
//	//Vector Q = S.cross(E1);
//
//	//float v = f * D.dot(Q);
//
//	///*; If the value of the V coordinate is outside the range of values inside the triangle,
//	//; then the ray has intersected the plane outside the triangle.
//	//;
//	//; U + V cannot exceed 1.0 or the point is not in the triangle.
//	//;
//	//; If you imagine the triangle as half a square this makes sense.U = 1 V = 1 would be  in the
//	//; lower left hand corner which would be in the second triangle making up the square.
//	//*/
//	//if ((v < 0.0) || ((u + v) > 1.0))
//	//	return -1.0;
//	//else 
//	//	return v;
//
//	Vector normal = poly->get_normal(Vector(1, 1, 1));
//	float r = normal.dot(getDirection().vsub(getOrigin()));
//	if (r == 0)
//	{
//		return -1.0;
//	}
//
//}
#define SMALL_NUM   0.00000001 // anything that avoids division overflow
float Ray::intersectTriangle(PolygonObjekt* poly)
{
	Vector    u, v, n;              // triangle vectors
	Vector    dir, w0, w;           // ray vectors
	float     r, a, b;              // params to calc ray-plane intersect

	// get triangle edge vectors and plane normal
	PolygonSurface& surface = poly->getSurface();
	Vector v0 = surface.vertices.at(0);
	u = surface.vertices.at(1).vsub(v0);
	v = surface.vertices.at(2).vsub(v0);
	n = u.cross(v);              // cross product
	if (n.x == 0 && n.y == 0 && n.z == 0)             // triangle is degenerate
		return -1;                  // do not deal with this case

	dir = getDirection();            // ray direction vector
	w0 = getOrigin().vsub(v0);
	a = -n.dot(w0);
	b = n.dot(dir);
	if (fabs(b) < SMALL_NUM) {     // ray is  parallel to triangle plane
		if (a == 0)                 // ray lies in triangle plane
			return -1;
		else 
			return -1;              // ray disjoint from plane
	}

	// get intersect point of ray with triangle plane
	r = a / b;
	if (r < 0.0)                    // ray goes away from triangle
		return -1;                   // => no intersect
	// for a segment, also test if (r > 1.0) => no intersect

	Vector I = dir.svmpy(r).vadd(getOrigin());            // intersect point of ray and plane

	// is I inside T?
	float    uu, uv, vv, wu, wv, D;
	uu = u.dot(u);
	uv = u.dot(v);
	vv = v.dot(v);
	w = I.vsub(v0);
	wu = w.dot(u);
	wv = w.dot(v);
	D = uv * uv - uu * vv;

	// get and test parametric coords
	float s, t;
	s = (uv * wv - vv * wu) / D;
	if (s < 0.0 || s > 1.0)         // I is outside T
		return -1;
	t = (uv * wu - uu * wv) / D;
	if (t < 0.0 || (s + t) > 1.0)  // I is outside T
		return -1;

	float distance = getOrigin().vsub(I).veclength();

	return 0.001 < distance ? distance : -1.0;                       // I is in T
}