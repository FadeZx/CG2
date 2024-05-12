#include <iostream>
#include <iomanip>
#include "HalfSpace.h"
using namespace std;


ostream& operator<<(ostream& s, const Hcoords v) {
    s << '[' << v.x << ',' << v.y << ',' << v.z << ',' << v.w << ']';
    return s;
}


bool near(float x, float y) {
    return Hcoords::Near(x, y);
}


int main(void) {
    cout << boolalpha;

    {
        Hcoords h(5, -2, 1, 3);
        Point P(9, -6, -8);
        cout << dot(h, P) << endl;
    }

    {
        Vector n(3, -5, 2);
        Point C(7, 8, -1);
        Hcoords h = HalfSpace(n, C);
        cout << near(dot(h, C), 0) << ' '
            << (dot(h, C + n) > 0) << ' '
            << (dot(h, C - n) < 0) << endl;
    }

    {
        Point V1(1, 1, 0),
            V2(2, 1, 1),
            V3(-1, 0, 2),
            V4(0, -1, -1);
        Hcoords h = HalfSpace(V1, V2, V3, V4);
        cout << near(dot(h, V1), 0) << ' '
            << near(dot(h, V2), 0) << ' '
            << near(dot(h, V3), 0) << ' '
            << (dot(h, V4) < 0) << endl;
    }

    {
        Hcoords h(1, 1, -2, 3);
        Point P(7, 1, -2),
            Q(17, -24, 3);
        Interval inter = ClipSegment(h, P, Q);
        cout << '[' << inter.bgn << ',' << inter.end << ']' << endl;
    }

    return 0;
}

