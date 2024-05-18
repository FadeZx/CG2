#include <vector>
#include "Affine.h"  // Assuming this file provides definitions for Point, Hcoords, etc.
#include "Clip.h"

Point intersect(const Point& S, const Point& E, const Hcoords& plane) {
    Vector SE = E - S; // Direction vector of the line segment
    float denominator = dot(Vector(plane.x, plane.y, plane.z), SE); // Compute denominator using dot product

    if (denominator == 0) return S;  // Line is parallel to the plane

    float numerator = -(dot(Vector(plane.x, plane.y, plane.z), Vector(S.x, S.y, S.z)) + plane.w);
    float t = numerator / denominator;
    return S + t * SE;
}

Clip::Clip() {
    // Define planes for a unit cube [-1, 1]^3
    hspaces.push_back(Hcoords(1, 0, 0, 1));  // x >= -1
    hspaces.push_back(Hcoords(-1, 0, 0, 1)); // x <= 1
    hspaces.push_back(Hcoords(0, 1, 0, 1));  // y >= -1
    hspaces.push_back(Hcoords(0, -1, 0, 1)); // y <= 1
    hspaces.push_back(Hcoords(0, 0, 1, 1));  // z >= -1
    hspaces.push_back(Hcoords(0, 0, -1, 1)); // z <= 1
}

Clip::Clip(int n, const Hcoords* clip_to_hspaces) {
    for (int i = 0; i < n; ++i) {
        hspaces.push_back(clip_to_hspaces[i]);
    }
}

bool Clip::operator()(Point& A, Point& B) {
    float t0 = 0.0, t1 = 1.0;
    Vector direction = B - A;

    for (const Hcoords& plane : hspaces) {
        float numerator = dot(Vector(plane.x, plane.y, plane.z), Vector(A.x, A.y, A.z)) + plane.w;
        float denominator = dot(Vector(plane.x, plane.y, plane.z), direction);

        if (fabs(denominator) < 1e-6) {  // Consider an epsilon for floating point precision issues
            if (numerator > 0) return false;  // The segment is entirely outside the plane
            continue;  // The segment is parallel to the plane and lies inside
        }

        float t = -numerator / denominator;
        if (denominator > 0) {
            if (t > t1) return false;
            t0 = std::max(t0, t);
        }
        else {
            if (t < t0) return false;
            t1 = std::min(t1, t);
        }
    }

    if (t0 > t1) return false;

    A = A + t0 * direction;
    B = A + (t1 - t0) * direction;
    return true;
}


bool Clip::operator()(std::vector<Point>& verts_to_clip) {
    if (verts_to_clip.size() < 3) return false;  // Not a polygon

    std::vector<Point> input = verts_to_clip, output;

    for (const Hcoords& plane : hspaces) {
        if (input.empty()) return false;

        Point S = input.back();
        output.clear();

        for (const Point& E : input) {
            if (dot(Vector(plane.x, plane.y, plane.z), Vector(E.x, E.y, E.z)) + plane.w < 0) {
                // E is inside the half-space
                if (dot(Vector(plane.x, plane.y, plane.z), Vector(S.x, S.y, S.z)) + plane.w >= 0) {
                    output.push_back(intersect(S, E, plane));
                }
                output.push_back(E);
            }
            else if (dot(Vector(plane.x, plane.y, plane.z), Vector(S.x, S.y, S.z)) + plane.w < 0) {
                // S is inside but E is outside
                output.push_back(intersect(S, E, plane));
            }
            S = E;
        }

        input = output;
    }

    verts_to_clip = input;
    return !verts_to_clip.empty();
}


