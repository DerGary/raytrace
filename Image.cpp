#include "StdAfx.h"
#include "Image.h"

int sgn(int x){
  return (x > 0) ? 1 : (x < 0) ? -1 : 0;
}

Image::Image(int x, int y) : xSize(x), ySize(y)
{ 
	alloc();
};

void Image::set(int x, int y, int r, int g, int b)
{
    if(x < 0 || x >= xSize || y < 0 || y >= ySize)
        return;
    setR(x, y, r);
    setG(x, y, g);
    setB(x, y, b);
}

void Image::print(int *m, char *s)
{
    fprintf(stdout,"Channel %s  (%d x %d)\n", s, xSize, ySize);
    for(int y = 0; y < ySize; y++) {
        for(int x = 0; x < xSize; x++) 
            fprintf(stdout,"%d ", m[pos(x,y)]);
        fprintf(stdout,"\n");
    }
}

void Image::print() {
    print(R, "Red");
    print(G, "Green");
    print(B, "Blue");
}

Image::Image(char *ppm)
{
    char line[200];
    FILE *f = fopen(ppm, "ra");
    if(f == NULL) {
        fprintf(stderr,"Could not open image file '%s'\n", ppm);
        return;
    }
    int mode = 0;
    // Read header
    fgets(line, 200, f);
    if(strncmp(line, "P3", 2) == 0) {
        mode = 3;  // ppm
    } else if(strncmp(line, "P2", 2) == 0) {
        mode = 2;  // pgm
    } else {
        fprintf(stderr,"Wrong header '%s'\n", line);
        return;
    }
    fprintf(stdout,"Mode: %d\n", mode);
    // Skip comment and read image size
    do {
        fgets(line, 200, f);
    } while(line[0] == '#');
    int maxval;
    sscanf(line, "%d %d", &xSize, &ySize);
    fprintf(stderr,"Reading image of size %d x %d\n", xSize, ySize);
    fscanf(f, "%d", &maxval);
    if(maxval > 255) {
        fprintf(stderr,"Not an 8-bit image\n");
        fclose(f);
        return;
    }
    
	alloc();

    for(int y = 0; y < ySize; y++) 
        for(int x = 0; x < xSize; x++) {
            int r, g, b;
            switch(mode) {
                case 2:
                    if(fscanf(f,"%d", &r) != 1) {
                        fprintf(stderr,"Early pgm-EOF reached\n");
                        return;
                    }
                    g = r;
                    b = r;
                    break;
                case 3:
                    if(fscanf(f,"%d %d %d", &r, &g, &b) != 3) {
                        fprintf(stderr,"Early ppm-EOF reached\n");
                        return;
                    }
                    break;
            };
            setR(x,y,r);
            setG(x,y,g);
            setB(x,y,b);
        }

    fclose(f);
    return;
}

void Image::save(const char *ppm)
{
    FILE *f = fopen(ppm, "w");
    if(f == NULL) {
        fprintf(stderr,"Could not open image file '%s'\n", ppm);
        return;
    }
    int mode = 0;
    if(strstr(ppm, ".pgm") != NULL) {
        mode = 2;  // pgm
        fprintf(f, "P2\n");
    } else {
        mode = 3;  // ppm
        fprintf(f, "P3\n");
    }
    fprintf(stderr,"mode = %d   %s\n", mode, ppm);
    fprintf(f,"%d %d\n255\n", xSize, ySize);
    
    for(int y = 0; y < ySize; y++) {
        for(int x = 0; x < xSize; x++) {
            switch(mode) {
                case 3:
                    fprintf(f,"%d %d %d ", getR(x,y), getG(x,y), getB(x,y));
                    break;
                case 2:
                    fprintf(f,"%d ", int(0.299*double(getR(x,y)) + 0.587*double(getG(x,y)) + 0.114*double(getB(x,y)) + 0.5));
                    break;
            }
        }
        fprintf(f,"\n");
    }
    
    fclose(f);
    return;
}

