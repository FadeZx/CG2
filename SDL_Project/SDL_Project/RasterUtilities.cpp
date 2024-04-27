#include "RasterUtilities.h"

void FillRect(Raster& r, int x, int y, int w, int h) {

    r.GotoPoint(x, y);
    for (int i = 0; i < w; ++i) {
        for (int j = 0; j < h; ++j) {
            r.WritePixel();
            r.WriteZ(1);
            r.IncrementX();

        }
        for (int j = 0; j < h; ++j) {
            r.DecrementX();
        }
        r.IncrementY();
    }

}

void FillRectZ(Raster& r, int x, int y, int w, int h, float z) {

    r.GotoPoint(x, y);
    for (int i = 0; i < w; ++i) {
        for (int j = 0; j < h; ++j) {
            r.WritePixel();
            r.WriteZ(z);
            r.IncrementX();

        }
        for (int j = 0; j < h; ++j) {
            r.DecrementX();
        }
        r.IncrementY();
    }

}

void DrawLine(Raster& r, const Point& P, const Point& Q) {

    float dx = Q.x - P.x;
    float dy = Q.y - P.y;
    float dz = Q.z - P.z;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

    float xIncrement = dx / steps;
    float yIncrement = dy / steps;
    float zIncrement = dz / steps;
    float x = P.x;
    float y = P.y;
    float z = P.z;

    for (int i = 0; i <= steps; ++i) {
        int px = round(x);
        int py = round(y);

        r.GotoPoint(px, py);

         if (z <= r.GetZ()) {
             r.WriteZ(z);
             r.WritePixel();
         }
    
        x += xIncrement;
        y += yIncrement;
        z += zIncrement;
    }
}

