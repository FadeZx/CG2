#include "RasterUtilities.h"
#include <algorithm>
#include <vector>

int IntFloor(float value) {
    return static_cast<int>(value);
}

int IntCeil(float value) {
    return static_cast<int>(value + 0.99999f);
}

Point interpolatePoint(const Point& start, const Point& end, float t) {
    return Point{
        start.x + (end.x - start.x) * t,
        start.y + (end.y - start.y) * t,
        start.z + (end.z - start.z) * t
    };
}

void FillTriangle(Raster& r, const Point& P, const Point& Q, const Point& R) {
    
    const Point* top = &P;
    const Point* mid = &Q;
    const Point* bot = &R;

    //sort vertices by y-coordinate
    if (mid->y < top->y) std::swap(top, mid);
    if (bot->y < top->y) std::swap(top, bot);
    if (bot->y < mid->y) std::swap(mid, bot);

    // Calculate slopes and step increments
    float invSlope1 = (mid->y - top->y) ? (mid->x - top->x) / (mid->y - top->y) : 0;
    float invSlope2 = (bot->y - top->y) ? (bot->x - top->x) / (bot->y - top->y) : 0;
    float invSlope3 = (bot->y - mid->y) ? (bot->x - mid->x) / (bot->y - mid->y) : 0;

    //interpolate Z value
    auto interpolateZ = [&](float zStart, float zEnd, float yStart, float yEnd, float yCurrent) {
        if (yEnd != yStart)
            return zStart + (zEnd - zStart) * (yCurrent - yStart) / (yEnd - yStart);
        return zStart;
     };

    
    for (int y = IntCeil(top->y); y <= IntFloor(bot->y); y++) {
        bool isSecondHalf = y > mid->y || mid->y == top->y;
        float segmentHeight = isSecondHalf ? bot->y - mid->y : mid->y - top->y;
        float alpha = (float)(y - top->y) / (bot->y - top->y);
        float beta = (float)(y - (isSecondHalf ? mid->y : top->y)) / segmentHeight;

        Point A = interpolatePoint(*top, *bot, alpha);
        Point B = isSecondHalf ? interpolatePoint(*mid, *bot, beta) : interpolatePoint(*top, *mid, beta);

        if (A.x > B.x) std::swap(A, B);

        r.GotoPoint(IntCeil(A.x), y); // Set initial position for the row
        for (int x = IntCeil(A.x); x <= IntFloor(B.x); x++) {
            float phi = B.x == A.x ? 1.0f : (float)(x - A.x) / (B.x - A.x);
            float zCurrent = A.z + (B.z - A.z) * phi;
            if (zCurrent < r.GetZ()) {
                r.WriteZ(zCurrent);
                r.WritePixel();
            }
            if (x < IntFloor(B.x)) r.IncrementX(); // Move to the next pixel in the row
        }
    }
}
