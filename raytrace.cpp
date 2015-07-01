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

const double SCREENWIDTH = 1000;
const double SCREENHEIGHT = 1000;

using namespace std;

vector<Surface> surfaces;
vector<Property> properties;
vector<Objekt> objekte;
vector<Light> lights;
vector<Vector> vertices;
vector<vector<int>> allIndices;
vector<int> indices;
vector<PolygonSurface> polygonSurfaces;

static int xRes, yRes;
static Color background;
static Color ambience;
static Vector eye;
static Vector lookAt;
static double aspectRatio;
static double fovY;
static double fovX = 90;
static Vector up;

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
	void add_property(char *n,  double ar, double ag, double ab, double r, double g, double b, double s, double m) {
		fprintf(stderr,"  adding prop %f %f %f %f %f\n", r, g, b, s, m);
		properties.push_back(Property(n, Color(ar, ag, ab), Color(r, g, b), s, m));
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
				objekte.push_back(Objekt(s, p));
				break;
			}
		for (vector<PolygonSurface>::iterator i = polygonSurfaces.begin(); i != polygonSurfaces.end(); ++i)
			if (i->getName() == ss)
			{
				s = &(*i);
				//objekte.push_back(PolygonObjekt(&*i, p));
			}

		//if(s == NULL) {
		//	fprintf(stderr, "Surface not found: %s\n", ns);
		//	exit(1);
		//}
		//if(p == NULL) {
		//	fprintf(stderr, "Property not found: %s\n", np);
		//	exit(1);
		//}
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

	void constructPolygon(char*n)
	{
		
		for (auto&& a : allIndices)
		{
			vector<Vector> temp;
			for (auto&& b : a)
			{
				temp.push_back(vertices.at(b - 1));
			}
			polygonSurfaces.push_back(PolygonSurface(n,temp));
		}
		allIndices = vector<vector<int>>();
		vertices = vector<Vector>();
	}
	void add_vertex(double x, double y, double z)
	{
		vertices.push_back(Vector(x, y, z));
	}
	void constructIndices()
	{
		allIndices.push_back(indices);
		indices = vector<int>();
	}
	void add_indice(int a)
	{
		indices.push_back(a);
	}
}



int main(int argc, _TCHAR* argv[])
{
	/* parse the input file */
	yyin = fopen("data/scene.data","r");
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
	
	Vector defaultUp(0, 1, 0);
	up = up.normalize();
	double rcos = up.dot(defaultUp);
	Vector crossProduct = up.cross(defaultUp);
	double rsin = crossProduct.veclength();

	double u = crossProduct.x, v = crossProduct.y, w = crossProduct.z;
	//double matrix[3][3];
	//Matrix matrix = Matrix();
	//matrix.M11 = rcos + u*u*(1 - rcos);
	//matrix.M21 = w * rsin + v*u*(1 - rcos);
	//matrix.M31 = -v * rsin + w*u*(1 - rcos);
	//matrix.M12 = -w * rsin + u*v*(1 - rcos);
	//matrix.M22 = rcos + v*v*(1 - rcos);
	//matrix.M32 = u * rsin + w*v*(1 - rcos);
	//matrix.M13 = v * rsin + u*w*(1 - rcos);
	//matrix.M23 = -u * rsin + v*w*(1 - rcos);
	//matrix.M33 = rcos + w*w*(1 - rcos);
	/*matrix[0][0] = rcos + u*u*(1 - rcos);
	matrix[1][0] = w * rsin + v*u*(1 - rcos);
	matrix[2][0] = -v * rsin + w*u*(1 - rcos);
	matrix[0][1] = -w * rsin + u*v*(1 - rcos);
	matrix[1][1] = rcos + v*v*(1 - rcos);
	matrix[2][1] = u * rsin + w*v*(1 - rcos);
	matrix[0][2] = v * rsin + u*w*(1 - rcos);
	matrix[1][2] = -u * rsin + v*w*(1 - rcos);
	matrix[2][2] = rcos + w*w*(1 - rcos);
*/
	int Xresolution = xRes;
	int Yresolution = yRes;
	
	

	Vector camZVektor = lookAt.vsub(eye);
	
	Vector camXVektor = up.cross(camZVektor).normalize().svmpy(-1);

	Vector camYVektor = camZVektor.cross(camXVektor).normalize();

	double fovYValue = tan((0.5*fovY)*(M_PI / 180))*camZVektor.veclength();

	double planeWidth = fovYValue*aspectRatio;
	double planeHeight = fovYValue;

	//Vector planeXStart = camXVektor.svmpy(-(SCREENWIDTH / 2)).vadd(lookAt);
	Vector planeStart = camXVektor.svmpy(-(planeWidth)).vadd(camYVektor.svmpy(-(planeHeight))).vadd(lookAt);
	//Vector planeYStart = camYVektor.svmpy(-(SCREENHEIGHT / 2)).vadd(lookAt);

	Vector incrementX = camXVektor.svmpy(planeWidth*2 / xRes);
	Vector incrementY = camYVektor.svmpy(planeHeight*2 / yRes);

	Vector laufY = planeStart;
	Vector laufX = planeStart;

	Image bild(Xresolution, Yresolution);

	Ray	ray(lookAt, eye, 0);
	ray.setBackgroundColor(background);
	ray.setGlobalAmbience(ambience);

	for (int scanline = 0; scanline < Yresolution; scanline++) {

		printf("%4d\r", Yresolution - scanline);
		//y += dy;
		//Vector laufX = laufY;
		//double x = -0.5 * SCREENWIDTH;
		laufX = laufY;
		for (int sx = 0; sx < Xresolution; sx++) {
			ray.setDirection(laufX.vsub(ray.getOrigin()).normalize());
			//x += dx;
			laufX = laufX.vadd(incrementX);
			Color color = ray.shade(objekte, lights);

			bild.set(sx, scanline,
				color.r > 1.0 ? 255 : int(255 * color.r),
				color.g > 1.0 ? 255 : int(255 * color.g),
				color.b > 1.0 ? 255 : int(255 * color.b));
		}
		laufY = laufY.vadd(incrementY);
	}




	//double dx = SCREENWIDTH*aspectRatio / (double)Xresolution;
	//double dy = SCREENHEIGHT / (double)Yresolution;
	//double fovYValue=tan((0.5*fovY)*(M_PI / 180));
	//dx = dx*fovYValue;
	//dy = dy*fovYValue;
	////double dx = tan(0.5*fovX)*aspectRatio / (double)SCREENWIDTH;
	////double dy = tan(0.5*fovY) / (double)SCREENHEIGHT;
	//double y = (-0.5 * SCREENHEIGHT)*fovYValue;
	////Vector eye(0, 0, SCREENHEIGHT * 8.0);
	//Ray	ray(lookAt, eye,0);
	//ray.setBackgroundColor(background);
	//ray.setGlobalAmbience(ambience);


	//Image bild(Xresolution, Yresolution);

	//for (int scanline=0; scanline < Yresolution; scanline++) {

	//	printf("%4d\r", Yresolution-scanline);
	//	y += dy;
	//	double x = (-0.5 * SCREENWIDTH)*fovYValue;

	//	for (int sx=0; sx < Xresolution; sx++) {
	//		ray.setDirection((Vector(x+lookAt.x, y+lookAt.y, 0.0+lookAt.z).vsub(ray.getOrigin()).normalize()));
	//		x += dx;
	//		Color color = ray.shade(objekte, lights);

	//		bild.set(sx, scanline, 
	//			color.r > 1.0 ? 255 : int(255 * color.r),
	//			color.g > 1.0 ? 255 : int(255 * color.g),
	//			color.b > 1.0 ? 255 : int(255 * color.b));
	//	}
	//}

	char *name = "raytrace-bild.ppm";
	bild.save(name);
	std::cin.get();
	return 0;
}

