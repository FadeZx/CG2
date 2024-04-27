#pragma once

#include "Mesh.h"
#include <vector>
#include "ObjFileReader.h"  // Include the ObjFileReader header

class CowMesh : public Mesh {
public:
    CowMesh(const std::string& filename) {
        ObjFileReader objReader(filename);
        vertices = objReader.getVertices();
        faces = objReader.getFaces();
        edges = objReader.getEdges();
        dimensions = objReader.calculateDimensions();
        center = objReader.calculateCenter();
        for (const auto& face : objReader.getFaces()) {
            if (face.index1 >= vertices.size() || face.index2 >= vertices.size() || face.index3 >= vertices.size()) {
                throw std::runtime_error("Face index out of range.");
            }
        }
    };
    int VertexCount(void) override { return static_cast<int>(vertices.size()); }
    Point GetVertex(int i) override { return vertices[i]; }
    Vector Dimensions(void) override { return dimensions; }
    Point Center(void) override { return center; }
    int FaceCount(void) override { return static_cast<int>(faces.size()); }
    Face GetFace(int i) override { return faces[i]; }
    int EdgeCount(void) override { return static_cast<int>(edges.size()); }
    Edge GetEdge(int i) override { return edges[i]; }

private:
    std::vector<Point> vertices;
    std::vector
    <Face> faces;
    std::vector<Edge> edges;
    Vector dimensions;
    Point center;
};