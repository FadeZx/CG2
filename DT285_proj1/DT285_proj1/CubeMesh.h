// CubeMesh.h
// -- unit cube centered at the origin
// cs250 12/14

#include "Mesh.h"


class CubeMesh : public Mesh {
  public:
    int VertexCount(void) { return 8; }
    Point GetVertex(int i) { return vertices[i]; }
    Vector Dimensions(void) { return Vector(1,1,1); }
    Point Center(void) { return Point(); }
    int FaceCount(void) { return 12; }
    Face GetFace(int i) { return faces[i]; }
    int EdgeCount(void) { return 12; }
    Edge GetEdge(int i) { return edges[i]; }
  private:
    static const Point vertices[8];
    static const Face faces[12];
    static const Edge edges[12];
};

