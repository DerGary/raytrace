#pragma once

#include <string.h>

class Image {
  private:
    int xSize;
    int ySize;
    // RGB image
    int *R;
    int *G;
    int *B;
    
    Image() {};
	void alloc() { int n=xSize*ySize; R = new int[n]; G = new int[n]; B = new int[n]; };

    void set(int *, int);
	int pos(int x, int y)  { return y*xSize + x; };
	void setR(int x, int y, int v) { R[pos(x,y)] = v; };
	void setG(int x, int y, int v) { G[pos(x,y)] = v; };
	void setB(int x, int y, int v) { B[pos(x,y)] = v; };
	int getR(int x, int y) { return R[pos(x,y)]; };
	int getG(int x, int y) { return G[pos(x,y)]; };
	int getB(int x, int y) { return B[pos(x,y)]; };
    void print(int *, char *s);
    
  public:
    Image(int x, int y);
    Image(char *ppm);

	// Save to ppm file
    void save(const char *ppm);

	// Set single RGB pixel value
    void set(int x, int y, int r, int g, int b);

	// Set single gray value pixel
	void set(int x, int y, int v) {
        set(x, y, v, v, v);
    };

	void print();
};
