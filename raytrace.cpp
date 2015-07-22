// raytrace.cpp : Defines the entry point for the console application.
//

#include "stdafx.h" 
#include "Ray.h"
#include "Color.h"
#include "Image.h"
#include "vector"
#include <iostream>
#include <cmath>
#include "Matrix.h"
#include "Polygon.h"
#include <thread>
#include <sstream>
#include <ctime>
#include "poly2tri/tetgen.h"

const double SCREENWIDTH = 1000;
const double SCREENHEIGHT = 1000;

using namespace std;

vector<Surface> surfaces;
vector<Property> properties;
vector<Objekt*> objekte;
vector<Light> lights;
vector<Vector> vertices;
vector<Vector> normalVertices;
vector<vector<int>> allIndices;
vector<vector<int>> allNormalIndices;
vector<int> indices;
vector<int> normalIndices;
vector<PolygonSurface*> polygonSurfaces;

static int xRes, yRes;
static Color background;
static Color ambience;
static Vector eye;
static Vector lookAt;
static double aspectRatio;
static double fovY;
static double fovX = 90;
static Vector up;

struct VertexNormal
{
	Vector v;
	Vector n;
};
struct Sphere
{
	Vector midPoint;
	float radius;
};

extern "C" {
	extern FILE *yyin;
	int yyparse();

	void add_light(char *n, double dirx, double diry, double dirz, double colr, double colg, double colb) {
		fprintf(stderr,"  adding light %f %f %f %f %f %f\n", dirx, diry, dirz, colr, colg, colb);
		lights.push_back(Light(Vector(dirx,diry,dirz).normalize(), Color(colr, colg, colb)));
	};
	void add_quadric(char *n, double a, double b, double c, double d, double e, double f, double g, double h, double j, double k) {
		fprintf(stderr,"  adding quadric %s %f %f %f %f %f %f %f %f %f %f %f %f\n", n, a,b,c,d,e,f,g,h,j,k);
		surfaces.push_back(Surface(n, a,b,c,d,e,f,g,h,j,k));
	};
	void add_property(char *n,  double ar, double ag, double ab, double r, double g, double b, double s, double m, double shininess) {
		fprintf(stderr,"  adding prop %f %f %f %f %f\n", r, g, b, s, m);
		properties.push_back(Property(n, Color(ar, ag, ab), Color(r, g, b), s, m, shininess));
	};
	void setResolution(int x, int y){
		xRes = x;
		yRes = y;
	};
	void setBackground(double r, double g, double b){
		background.r = r;
		background.g = g;
		background.b = b;
	};
	void setAmbience(double r, double g, double b){
		ambience.r = r;
		ambience.g = g;
		ambience.b = b;
	};
	void setEyePoint(double x, double y, double z){
		eye.x = x;
		eye.y = y;
		eye.z = z;
	};
	void setLookAt(double x, double y, double z){
		lookAt.x = x;
		lookAt.y = y;
		lookAt.z = z;
	};
	void setAspectRatio(double ratio){
		aspectRatio = ratio;
	};
	void setFovY(double fovy){
		fovY = fovy;
	};
	void setUp(double x, double y, double z){
		up.x = x;
		up.y = y;
		up.z = z;
	};
	void add_objekt(char *ns, char *np) {
		Surface *s = NULL;
		Property *p = NULL;
		string ss(ns);
		string sp(np);
		for (vector<Property>::iterator i = properties.begin(); i != properties.end(); ++i)
			if (i->getName() == sp) {
				p = &(*i);
				break;
			}

		for(vector<Surface>::iterator i = surfaces.begin(); i != surfaces.end(); ++i) 
			if(i->getName() == ss) {
				s = &(*i);
				objekte.push_back(new Objekt(s, p));
				break;
			}

		vector<PolygonSurface*> polys;
		for (PolygonSurface* s : polygonSurfaces)
		{
			if (s->getName() == ss)
			{
				polys.push_back(s);
			}
		}
		if (polys.size() > 0)
		{
			PolygonObjekt* po = new PolygonObjekt(polys, p);
			objekte.push_back(po);
		}

		fprintf(stderr, "  adding object: surface %s, property %s\n", ns, np);
	}
	void add_sphere(char *n, double xm, double ym, double zm, double r)
	{
		//A = E = H = 1.0; B = C = F = 0.0; D = -2Xm, G = -2Ym, J = -2Zm und
		//# K = Xm ^ 2 + Ym ^ 2 + Zm ^ 2 - Radius ^ 2
		double a, e, h;
		double b, c, f;

		a = e = h = 1.0;
		b = c = f = 0.0;

		double d = -2.0 * xm;
		double g = -2.0 * ym;
		double j = -2.0 * zm;
		double k = xm*xm + ym*ym + zm*zm - r*r;
		surfaces.push_back(Surface(n, a, b, c, d, e, f, g, h, j, k));
	}

	void MinimumBoundingSphere(Sphere& sphere, Vector& a, Vector& b, Vector& c) {
		float dotABAB = b.vsub(a).dot(b.vsub(a));
		float dotABAC = b.vsub(a).dot(c.vsub(a));
		float dotACAC = c.vsub(a).dot(c.vsub(a));
		float d = 2.0f*(dotABAB*dotACAC - dotABAC*dotABAC);
		Vector midPoint;
		Vector referencePt = a;
		if (abs(d) <= 0.00001) {
			// a, b, and c lie on a line. Circle center is center of AABB of the
			// points, and radius is distance from circle center to AABB corner
			Vector min, max;
			min.x = DBL_MAX; max.x = -DBL_MAX;
			min.y = DBL_MAX; max.y = -DBL_MAX;
			min.z = DBL_MAX; max.z = -DBL_MAX;

			//minimale und maximale Werte berechnen
			if (a.x < min.x) min.x = a.x;
			if (a.x > max.x) max.x = a.x;
			if (a.y < min.y) min.y = a.y;
			if (a.y > max.y) max.y = a.y;
			if (a.z < min.z) min.z = a.z;
			if (a.z > max.z) max.z = a.z;

			if (b.x < min.x) min.x = b.x;
			if (b.x > max.x) max.x = b.x;
			if (b.y < min.y) min.y = b.y;
			if (b.y > max.y) max.y = b.y;
			if (b.z < min.z) min.z = b.z;
			if (b.z > max.z) max.z = b.z;

			if (c.x < min.x) min.x = c.x;
			if (c.x > max.x) max.x = c.x;
			if (c.y < min.y) min.y = c.y;
			if (c.y > max.y) max.y = c.y;
			if (c.z < min.z) min.z = c.z;
			if (c.z > max.z) max.z = c.z;
			midPoint.x = (min.x + max.x) / 2;
			midPoint.y = (min.y + max.y) / 2;
			midPoint.z = (min.z + max.z) / 2;
			referencePt = min;
		}
		else {
			float s = (dotABAB*dotACAC - dotACAC*dotABAC) / d;
			float t = (dotACAC*dotABAB - dotABAB*dotABAC) / d;
			// s controls height over AC, t over AB, (1-s-t) over BC
			if (s <= 0.0f) {
				midPoint = a.vadd(c).svmpy(0.5);
			}
			else if (t <= 0.0f) {
				midPoint = a.vadd(b).svmpy(0.5);
			}
			else if (s + t >= 1.0f) {
				midPoint = b.vadd(c).svmpy(0.5);
				referencePt = b;
			}
			else
				midPoint = a.vadd(b.vsub(a).svmpy(s)).vadd(c.vsub(a).svmpy(t));
		}
		sphere.radius = sqrt(midPoint.vsub(referencePt).dot(midPoint.vsub(referencePt)));
		sphere.midPoint = midPoint;
	}

	void constructPolygon(char*n)
	{

		//for (vector<int> i : allIndices)
		//{
		//	tetgenio in ,out;
		//	in.pointlist = new double[i.size() *3];
		//	in.numberofpoints = i.size();
		//	in.firstnumber = 0;
		//	in.numberoffacets = 1;
		//	in.facetlist = new tetgenio::facet[in.numberoffacets];
		//	in.facetmarkerlist = new int[in.numberoffacets];
		//	tetgenio::facet* f = &in.facetlist[0];
		//	f->numberofpolygons = 1;
		//	f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
		//	f->numberofholes = 0;
		//	f->holelist = NULL;
		//	tetgenio::polygon *p = &f->polygonlist[0];
		//	p->numberofvertices = i.size();
		//	p->vertexlist = new int[p->numberofvertices];
		//	int count = 0;
		//	int count2 = 0;
		//	for (int j : i)
		//	{
		//		Vector v = vertices.at(j - 1);
		//		in.pointlist[count++] = v.x;
		//		in.pointlist[count++] = v.y;
		//		in.pointlist[count++] = v.z;
		//		p->vertexlist[count2] = count2++;
		//	}
		//	tetgenbehavior behaviour;
		//	behaviour.weighted = 1;
		//	tetrahedralize(&behaviour, &in, &out);
		//}

		//for (int i = 0; i < allIndices.size(); i++)
		//{
		//	vector<int> indices = allIndices.at(i);
		//	vector<VertexNormal> vert;
		//	for (int j = 0; j < indices.size(); j++)
		//	{
		//		VertexNormal vn;
		//		vn.v = vertices.at(indices.at(j) - 1);
		//		if (normalIndices.size() > j)
		//		{
		//			vn.n = normalVertices.at(normalIndices.at(j) - 1);
		//		}
		//		vert.push_back(vn);
		//	}

		//	VertexNormal u, nahe, nahe2;
		//	u = vert.at(0);
		//	double minDist = DBL_MAX;
		//	for (int j = 1; j < vert.size(); j++)
		//	{
		//		VertexNormal temp = vert.at(j);
		//		double dist = u.v.vsub(temp.v).veclength();
		//		if (dist < minDist)
		//		{
		//			minDist = dist;
		//			nahe2 = nahe;
		//			nahe = temp;
		//		}
		//	}

		//	Vector normal = u.v.vsub(nahe.v).cross(u.v.vsub(nahe2.v)).normalize();
		//	if (normal.dot(u.n)<0)
		//	{
		//		VertexNormal temp = nahe2;
		//		nahe2 = nahe;
		//		nahe = temp;
		//	}
		//	Sphere sphere;
		//	MinimumBoundingSphere(sphere, u.v, nahe.v, nahe2.v);
		//}


		for (int i = 0; i < allIndices.size(); i++)
		{
			vector<int> a = allIndices.at(i);
			Vector u, v, w;
			u = vertices.at(a.at(0) - 1);
			v = vertices.at(a.at(1) - 1);
			w = vertices.at(a.at(2) - 1);
			if (allNormalIndices.size() > 0)
			{
				vector<int> normals = allNormalIndices.at(i);
				Vector n0, n1, n2;
				n0 = normalVertices.at(normals.at(0) - 1).normalize();
				n1 = normalVertices.at(normals.at(1) - 1).normalize();
				n2 = normalVertices.at(normals.at(2) - 1).normalize();
				polygonSurfaces.push_back(new PolygonSurface(n, u, v, w, n0, n1, n2));
				if (a.size()>3)
				{
					for (int j = 3; j < a.size(); j++){
						v = vertices.at(a.at(j) - 1);
						n1 = normalVertices.at(normals.at(j) - 1);

						Vector t = v;
						v = w;
						w = t;

						t = n1;
						n1 = n2;
						n2 = t;
						polygonSurfaces.push_back(new PolygonSurface(n, u, v, w, n0,n1,n2));
					}
				}
			} else
			{
				polygonSurfaces.push_back(new PolygonSurface(n, u, v, w));
				if (a.size()>3)
				{
					for (int j = 3; j < a.size(); j++){
						v = vertices.at(a.at(j) - 1);

						Vector t = v;
						v = w;
						w = t;
						polygonSurfaces.push_back(new PolygonSurface(n, u, v, w));
					}
				}
			}
		}
		allIndices = vector<vector<int>>();
		allNormalIndices = vector<vector<int>>();
		//vertices = vector<Vector>();
	}
	void add_normalVertex(double x, double y, double z)
	{
		normalVertices.push_back(Vector(x, y, z));
	}
	void add_vertex(double x, double y, double z)
	{
		vertices.push_back(Vector(x, y, z));
	}
	void constructIndices()
	{
		allIndices.push_back(indices);
		allNormalIndices.push_back(normalIndices);
		indices = vector<int>();
		normalIndices = vector<int>();
	}
	void add_indice(int a)
	{
		indices.push_back(a);
	}
	void add_stringIndice(char* s)
	{
		stringstream ss(s);
		string result;
		getline(ss, result, '/');
		indices.push_back(atoi(result.c_str()));
	}
	void add_normal(int a)
	{
		normalIndices.push_back(a);
	}
}
struct Pixel
{
	int x, y, r, g, b;
	Pixel(int x, int y, int r, int g, int b) : x(x), y(y), r(r), g(g), b(b){}
};

void fooNeu(int process, int processCount, Image& bild)
{

	int Xresolution = xRes;
	int Yresolution = yRes;

	Vector camZVektor = lookAt.vsub(eye);
	Vector camXVektor = up.cross(camZVektor).normalize().svmpy(-1);
	Vector camYVektor = camZVektor.cross(camXVektor).normalize();

	double fovYValue = tan((0.5*fovY)*(M_PI / 180))*camZVektor.veclength();

	double planeWidth = fovYValue*aspectRatio;
	double planeHeight = fovYValue;

	Vector planeStart = camXVektor.svmpy(-(planeWidth)).vadd(camYVektor.svmpy(-(planeHeight))).vadd(lookAt);

	Vector incrementX = camXVektor.svmpy(planeWidth * 2 / xRes);
	Vector incrementY = camYVektor.svmpy(planeHeight * 2 / yRes);

	Vector laufY = planeStart.vadd(incrementY.svmpy(process));
	Vector laufX = laufY;

	incrementY = incrementY.svmpy(processCount);

	Ray	ray(lookAt, eye, 0);
	ray.setBackgroundColor(background);
	ray.setGlobalAmbience(ambience);


	std::time_t timeEnd = std::time(nullptr);
	int finish = (Yresolution / processCount);
	if (Yresolution % processCount != 0)
	{
		finish += 1;
	}
	for (int scanline = 0; scanline < finish; scanline++) {
	std:time_t currentTime = std::time(nullptr);
		printf("%4d Zeit bis ende: %4lld minuten\n\r", finish - scanline, (currentTime - timeEnd)*(finish - scanline) / 60);
		timeEnd = currentTime;
		laufX = laufY;
		for (int sx = 0; sx < Xresolution; sx++) {
			ray.setDirection(laufX.vsub(ray.getOrigin()).normalize());
			laufX = laufX.vadd(incrementX);
			Color color = ray.shade(objekte, lights);

			//bild->push_back(Pixel(sx, process + (scanline*processCount),
			//	color.r > 1.0 ? 255 : int(255 * color.r),
			//	color.g > 1.0 ? 255 : int(255 * color.g),
			//	color.b > 1.0 ? 255 : int(255 * color.b)));
			
			bild.set(sx, process + (scanline*processCount),
			color.r > 1.0 ? 255 : int(255 * color.r),
			color.g > 1.0 ? 255 : int(255 * color.g),
			color.b > 1.0 ? 255 : int(255 * color.b));
		}
		laufY = laufY.vadd(incrementY);
	}
	printf("Thread finished\n");
}



int main(int argc, _TCHAR* argv[])
{
	/* parse the input file */
	yyin = fopen("data/lamboSceneAll.data","r");
	if(yyin == NULL) {
		fprintf(stderr, "Error: Konnte Datei nicht öffnen\n");
		return 1;
	}
	if (yyparse()){
		fprintf(stderr, "Error: Konnte Datei nicht einlesen\n");
		std::cin.get();
		return 1;
	}
	fclose (yyin);


	

	up = up.normalize();


	int Xresolution = xRes;
	int Yresolution = yRes;

	Vector camZVektor = lookAt.vsub(eye);
	Vector camXVektor = up.cross(camZVektor).normalize().svmpy(-1);
	Vector camYVektor = camZVektor.cross(camXVektor).normalize();

	double fovYValue = tan((0.5*fovY)*(M_PI / 180))*camZVektor.veclength();

	double planeWidth = fovYValue*aspectRatio;
	double planeHeight = fovYValue;

	Vector planeStart = camXVektor.svmpy(-(planeWidth)).vadd(camYVektor.svmpy(-(planeHeight))).vadd(lookAt);

	Vector incrementX = camXVektor.svmpy(planeWidth*2 / xRes);
	Vector incrementY = camYVektor.svmpy(planeHeight*2 / yRes);

	Vector laufY = planeStart;
	Vector laufX = planeStart;

	Image bild(Xresolution, Yresolution);

	Ray	ray(lookAt, eye, 0);
	ray.setBackgroundColor(background);
	ray.setGlobalAmbience(ambience);

	int processCount = 4;
	vector<thread*> pool;
	//vector<vector<Pixel>*> pixelPool;
	for (int i = 0; i < processCount; i++)
	{
		//vector<Pixel>* pix = new vector<Pixel>();
		//std::thread* t = new thread(foo, (int)(Yresolution / processCount)*(i), (int)(Yresolution / processCount)*(i + 1), pix);
		std::thread* t = new thread(fooNeu, i, processCount, bild);
		//pixelPool.push_back(pix);
		pool.push_back(t);
		
	}
	for (int i = 0; i < processCount; i++)
	{
		pool.at(i)->join();
	}

	//for (int i = 0; i < pixelPool.size(); i++)
	//{
	//	vector<Pixel>* pix = pixelPool.at(i);
	//	for (int j = 0; j < pix->size(); j++)
	//	{
	//		Pixel pixel = pix->at(j);
	//		bild.set(pixel.x, pixel.y, pixel.r, pixel.g, pixel.b);
	//	}
	//}

	char *name = "raytrace-bild.ppm";
	bild.save(name);
	std::cin.get();
	return 0;
}
