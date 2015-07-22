#pragma once
#include "Vector.h"
#include <vector>
#include <cmath>
#include <algorithm>

#define SMALL_NUM   0.00001 // anything that avoids division overflow

class PolygonSurface : public Surface
{
private:
	std::string name;


public:
	Vector v0;
	Vector v1;
	Vector v2;
	Vector normal;
	Vector normal0;
	Vector normal1;
	Vector normal2;
	bool normalVectors = true;
	PolygonSurface(char* name, Vector v0, Vector v1, Vector v2) 
		: PolygonSurface(name, v0, v1, v2, Vector(0, 0, 0), Vector(0, 0, 0), Vector(0, 0, 0))
	{
		normalVectors = false;
	};
	PolygonSurface(char* name, Vector v0, Vector v1, Vector v2, Vector normal0, Vector normal1, Vector normal2)
		: name(name), v0(v0), v1(v1), v2(v2), normal0(normal0), normal1(normal1), normal2(normal2)
	{
		normal = v0.vsub(v1).cross(v0.vsub(v2)).normalize();
	};
	std::string getName()
	{
		return name;
	};
	Vector barycentric(Vector& point)
	{
		Vector p0 = v1.vsub(v0);
		Vector p1 = v2.vsub(v0);
		Vector p2 = point.vsub(v0);
		double d00 = p0.dot(p0);
		double d01 = p0.dot(p1);
		double d11 = p1.dot(p1);
		double d20 = p2.dot(p0);
		double d21 = p2.dot(p1);
		double denom = d00 * d11 - d01 * d01;
		double v = (d11 * d20 - d01 * d21) / denom;
		double w = (d00 * d21 - d01 * d20) / denom;
		double u = 1.0f - v - w;

		return Vector(u, v, w);
	}
	Vector get_normal(Vector &v) override
	{
		if (normalVectors)
		{
			Vector bar = barycentric(v);
			return normal0.svmpy(bar.x).vadd(normal1.svmpy(bar.y)).vadd(normal2.svmpy(bar.z)).normalize();
		}
		return normal;
	};

	float intersect(Vector &direction, Vector& origin)
	{
		Vector    u, v, n;              // triangle vectors
		Vector     w0, w;           // ray vectors
		float     r, a, b;              // params to calc ray-plane intersect

		// get triangle edge vectors and plane normal
		u = v1.vsub(v0);
		v = v2.vsub(v0);
		n = u.cross(v);              // cross product
		if (n.x == 0 && n.y == 0 && n.z == 0)             // triangle is degenerate
			return -1;                  // do not deal with this case


		w0 = origin.vsub(v0);
		a = -n.dot(w0);
		b = n.dot(direction);
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

		Vector I = direction.svmpy(r).vadd(origin);            // intersect point of ray and plane

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

		float distance = origin.vsub(I).veclength();

		return 0.0001 < distance ? distance : -1.0;                       // I is in T
	};
};
class PolygonObjekt : public Objekt
{
	std::vector<PolygonSurface*> surfaces;
	float radius;
	Vector midPoint;

public:
	PolygonObjekt(void) : Objekt(), surfaces(NULL) {};
	PolygonObjekt(std::vector<PolygonSurface*> s, Property *p) : Objekt(NULL,p), surfaces(s)
	{
		calculateBoundingSphere();
		fprintf(stderr, "bounding box midpoint %f,%f,%f radius: %f",midPoint.x, midPoint.y, midPoint.z, radius);

	};
	void calculateBoundingSphere()
	{
		//minimale Werte möglichst hoch setzen
		//maximale Werte möglichst niedrig setzen
		Vector min, max;
		min.x = DBL_MAX; max.x = -DBL_MAX;
		min.y = DBL_MAX; max.y = -DBL_MAX;
		min.z = DBL_MAX; max.z = -DBL_MAX;

		//minimale und maximale Werte berechnen
		for (PolygonSurface* s : surfaces)
		{
			if (s->v0.x < min.x) min.x = s->v0.x;
			if (s->v0.x > max.x) max.x = s->v0.x;
			if (s->v0.y < min.y) min.y = s->v0.y;
			if (s->v0.y > max.y) max.y = s->v0.y;
			if (s->v0.z < min.z) min.z = s->v0.z;
			if (s->v0.z > max.z) max.z = s->v0.z;

			if (s->v1.x < min.x) min.x = s->v1.x;
			if (s->v1.x > max.x) max.x = s->v1.x;
			if (s->v1.y < min.y) min.y = s->v1.y;
			if (s->v1.y > max.y) max.y = s->v1.y;
			if (s->v1.z < min.z) min.z = s->v1.z;
			if (s->v1.z > max.z) max.z = s->v1.z;

			if (s->v2.x < min.x) min.x = s->v2.x;
			if (s->v2.x > max.x) max.x = s->v2.x;
			if (s->v2.y < min.y) min.y = s->v2.y;
			if (s->v2.y > max.y) max.y = s->v2.y;
			if (s->v2.z < min.z) min.z = s->v2.z;
			if (s->v2.z > max.z) max.z = s->v2.z;
		}

		midPoint.x = (min.x + max.x) / 2;
		midPoint.y = (min.y + max.y) / 2;
		midPoint.z = (min.z + max.z) / 2;


		radius = 0;
		for (PolygonSurface* s : surfaces)
		{
			Vector d(s->v0.x - midPoint.x, s->v0.y - midPoint.y, s->v0.z - midPoint.z);
			double le = d.veclength();
			if (radius < le) radius = le;
			d = Vector(s->v1.x - midPoint.x, s->v1.y - midPoint.y, s->v1.z - midPoint.z);
			le = d.veclength();
			if (radius < le) radius = le;
			d = Vector(s->v2.x - midPoint.x, s->v2.y - midPoint.y, s->v2.z - midPoint.z);
			le = d.veclength();
			if (radius < le) radius = le;
		}
	}


	float intersect(Vector &direction, Vector& origin, Surface** s) override
	{
		float intersect = midPoint.vsub(origin).cross(direction).veclength();
		if (intersect < radius)
		{
			float minValue = DBL_MAX;
			bool intersected = false;
			for (PolygonSurface* surf : surfaces)
			{
				float t = surf->intersect(direction, origin);
				if (t > 0.0 && t < minValue)
				{
					minValue = t;
					*s = surf;
					intersected = true;
				}
			}
			if (intersected)
				return minValue;
		}
		return -1.0;
	};


};