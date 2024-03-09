// CubeMesh.h
// -- axis-aligned cube
// cs250 12/14

#include "Mesh.h"


class CubeMesh : public Mesh {
  public:
    int VertexCount(void) override { return 8; }
    Point GetVertex(int i) override { return vertices[i]; }
    Vector Dimensions(void) override { return Vector(2, 2, 2); }
    Point Center(void) override { return Point(0, 0, 0); }
    int FaceCount(void) override { return 12; }
    Face GetFace(int i) override { return faces[i]; }
    int EdgeCount(void) override { return 12; }
    Edge GetEdge(int i) override { return edges[i]; }

private:
    static const Point vertices[8];
    static const Face faces[12];
    static const Edge edges[12];
};

