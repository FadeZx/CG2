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
    float t0 = 0.0f, t1 = 1.0f;
    Vector D = B - A;  // Direction vector of the line segment

    for (const auto& plane : hspaces) {
        float numerator = dot(Vector(plane.x, plane.y, plane.z), Vector(A.x, A.y, A.z)) + plane.w;
        float denominator = dot(Vector(plane.x, plane.y, plane.z), D);

        if (denominator == 0) {
            if (numerator > 0) return false;  // Line is parallel and outside
        }
        else {
            float t = -numerator / denominator;
            if (denominator > 0) {
                if (t > t1) return false;  // No intersection
                t0 = std::max(t0, t);
            }
            else {
                if (t < t0) return false;  // No intersection
                t1 = std::min(t1, t);
            }
        }
    }

    if (t0 > t1) return false;  // No intersection

    A = A + t0 * D;  // Calculate new A
    B = A + (t1 - t0) * D;
    return true;
}

bool Clip::operator()(std::vector<Point>& verts_to_clip) {
    std::vector<Point> input = verts_to_clip;
    std::vector<Point> output;

    for (const Hcoords& clipPlane : hspaces) {
        if (input.empty()) return false;

        Point S = input.back();

        output.clear();
        for (const Point& E : input) {
            float planeDotE = dot(Vector(clipPlane.x, clipPlane.y, clipPlane.z), Vector(E.x, E.y, E.z)) + clipPlane.w;
            float planeDotS = dot(Vector(clipPlane.x, clipPlane.y, clipPlane.z), Vector(S.x, S.y, S.z)) + clipPlane.w;

            if (planeDotE < 0) {  // E is inside
                if (planeDotS >= 0) {
                    Point I = intersect(S, E, clipPlane);
                    output.push_back(I);
                }
                output.push_back(E);
            }
            else if (planeDotS < 0) {  // S is inside but E is outside
                Point I = intersect(S, E, clipPlane);
                output.push_back(I);
            }
            S = E;
        }
        input = output;
    }

    if (output.empty()) return false;

    verts_to_clip = output;
    return true;
}


