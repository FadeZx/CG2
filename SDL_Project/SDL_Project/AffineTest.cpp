// AffineTest.cpp
// cs200 1/15

#include <iostream>
#include <iomanip>
#include <cmath>
#include "Affine.h"
using namespace std;


ostream& operator<<(ostream& s, const Hcoords& v) {
  return s << '[' << v.x << ',' << v.y << ',' << v.z << ',' << v.w << ']';
}


ostream& operator<<(ostream& s, const Matrix& A) {
  s << '{';
  for (int i=0; i < 4; ++i) {
    s << '{';
    for (int j=0; j < 4; ++j)
      s << A[i][j] << ((j < 3) ? ',' : '}');
    s << ((i < 3) ? ',' : '}');
  }
  return s;
}


int main(void) {
  const Point O;
  const float pi = 4.0f*atan(1.0f);
  cout << fixed << setprecision(3);

  Vector v(4,-5,3);
  Affine A = Trans(v) * Scale(3);
  cout << A << endl;

  Point P(9,-2,-7);
  Point Q = O + 3*(P-O) + v,
      R = A*P;
  cout << abs(R-Q) << endl;

  A = Rot(0.5f*pi,v) * Trans(Vector(7,5,1));
  cout << A << endl;

  P = Point(1,4,2);
  Affine B = Trans(P-O) * Rot(1.5f*pi,Vector(5,1,2)) * Trans(O-P);
  cout << B << endl;

  Q = Point(5,-8,6);
  Vector u = Q - P;
  cout << dot(u,B*Q-P) << endl;
  
  B = Affine(Vector(5,2,1),Vector(1,4,5),Vector(9,8,2),Point(4,4,3));
  cout << B << endl;

  u = 3*P + 2*Q - 5*R;
  cout << u << endl;
 // P = 3*P + 2*Q - 7*R; // this should fail ...

  return 0;
}

