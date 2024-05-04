#include <iostream>
#include <iomanip>
#include "HalfSpace.h"
#include "Clip.h"
using namespace std;


ostream& operator<<(ostream& s, const Point& P) {
  s << '(' << P.x << ',' << P.y << ',' << P.z << ')';
  return s;
}


int main(void) {
  cout << boolalpha;

  {
    Clip clip;

    Point P(-1,0.5,2),
          Q(1,1.5,-1);
    cout << clip(P,Q) << ' '
         << P << ' ' << Q << endl;

    vector<Point> verts;
    verts.push_back(Point(0,0,0));
    verts.push_back(Point(4,1,0));
    verts.push_back(Point(0,-1,2));
    cout << clip(verts) << ' ';
    unsigned istart = 0;
    for (unsigned i=1; i < verts.size(); ++i)
      istart = (verts[i].y < verts[istart].y) ? i : istart;
    for (unsigned i=0; i < verts.size(); ++i)
      cout << verts[(istart+i)%verts.size()] << ' ';
    cout << endl;
  }

  {
    Point V[4] = { Point(0,0,0), Point(1,0,0), Point(0,1,0), Point(0,0,1) };
    Hcoords h[4] = { HalfSpace(V[1],V[2],V[3],V[0]),
                     HalfSpace(V[0],V[2],V[3],V[1]),
                     HalfSpace(V[0],V[1],V[3],V[2]),
                     HalfSpace(V[0],V[1],V[2],V[3]) };
    Clip clip(4,h);

    Point P(0,-1,2),
          Q(1,1,-1);
    cout << clip(P,Q) << ' '
         << P << ' ' << Q << endl;

    vector<Point> verts;
    verts.push_back(Point(-1,1,0));
    verts.push_back(Point(1,1,1));
    verts.push_back(Point(1,-1,1));
    verts.push_back(Point(-1,-1,0));
    cout << clip(verts) << ' ';
    unsigned istart = 0;
    for (unsigned i=1; i < verts.size(); ++i)
      istart = (verts[i].y < verts[istart].y) ? i : istart;
    for (unsigned i=0; i < verts.size(); ++i)
      cout << verts[(istart+i)%verts.size()] << ' ';
    cout << endl;
  }

  return 0;
}

