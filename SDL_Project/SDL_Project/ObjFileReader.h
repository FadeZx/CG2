#pragma once
#include "Mesh.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <utility>
#include <set>

class ObjFileReader {
public:
    ObjFileReader(const std::string& filename);
    const std::vector<Point>& getVertices() const;
    const std::vector<Mesh::Face>& getFaces() const;
    const std::vector<Mesh::Edge>& getEdges() const;
    Vector calculateDimensions() const;
    Point calculateCenter() const;
    Vector calculateFaceNormal(const Point& p1, const Point& p2, const Point& p3);

private:
    std::vector<Point> vertices;
    std::vector<Mesh::Face> faces;
    std::vector<Mesh::Edge> edges;

    void loadFromObj(const std::string& filename);
};
