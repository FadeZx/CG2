#ifndef DT285_HALFSPACE
#define DT285_HALFSPACE

#include "Affine.h"

// computes the dot product of the half space h, which specified by
// its homogeneous coordinate representation and the point P.
float dot(const Hcoords& h, const Point& P);

// computes the dot product of the half space h, which specified by
// its homogeneous coordinate representation and the point P.
Hcoords HalfSpace(const Vector& n, const Point& C);

// computes the homogeneous coordinate representation of the half space
// with outwardly pointing surface normal vector n and whose boundary contains the point C.
Hcoords HalfSpace(const Point& A, const Point& B, const Point& C, const Point& P);

struct Interval {
	float bgn, end;
	Interval(float a = 0, float b = 1) : bgn(a), end(b) { }
	bool IsEmpty(void) const { return bgn > end; }
};


// computes the intersection interval I = [a, b] that corresponds to the intersection of the half space h
// and the line segment PQ with endpoints PQ.
// If the intersection is empty then return empty Interval (bgn > end)
Interval ClipSegment(const Hcoords& h, const Point& P, const Point& Q);


#endif

