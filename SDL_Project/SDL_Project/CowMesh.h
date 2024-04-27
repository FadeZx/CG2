#pragma once

#include "Mesh.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

class CowMesh : public Mesh {
public:
    CowMesh(const std::string& filename);
    int VertexCount(void) override { return static_cast<int>(vertices.size()); }
    Point GetVertex(int i) override { return vertices[i]; }
    Vector Dimensions(void) override;
    Point Center(void) override;
    int FaceCount(void) override { return static_cast<int>(faces.size()); }
    Face GetFace(int i) override { return faces[i]; }
    int EdgeCount(void) override { return static_cast<int>(edges.size()); }
    Edge GetEdge(int i) override { return edges[i]; }

private:
    std::vector<Point> vertices;
    std::vector<Face> faces;
    std::vector<Edge> edges;

    void LoadFromObj(const std::string& filename);
    Vector CalculateDimensions();
    Point CalculateCenter();
};
