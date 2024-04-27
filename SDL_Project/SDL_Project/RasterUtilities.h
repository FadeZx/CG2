#ifndef DT285_RASTERUTILITIES_H
#define DT285_RASTERUTILITIES_H

#include "Raster.h"
#include "Affine.h"


void FillRect(Raster& r, int x, int y, int w, int h);

void FillRectZ(Raster& r, int x, int y, int w, int h, float z = 1);

void DrawLine(Raster& r, const Point& P, const Point& Q);

// DO NOT implement this function in assignment #11:
void FillTriangle(Raster& r, const Point& P, const Point& Q, const Point& R);


#endif

