#include "CowMesh.h"
#include <sstream>
#include <fstream>
#include <stdexcept>

CowMesh::CowMesh(const std::string& filename) {
    LoadFromObj(filename);
}

void CowMesh::LoadFromObj(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    std::string line;
    while (getline(file, line)) {
        std::stringstream ss(line);
        std::string type;
        ss >> type;
        if (type == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            vertices.emplace_back(x, y, z);
        }
        else if (type == "f") {
            int idx1, idx2, idx3;
            char ignore; // Used to ignore the slashes and potential other indices (like texture or normal indices)
            std::string vertex1, vertex2, vertex3;
            if (ss >> vertex1 >> vertex2 >> vertex3) {
                std::stringstream vertexStream1(vertex1);
                std::stringstream vertexStream2(vertex2);
                std::stringstream vertexStream3(vertex3);

                vertexStream1 >> idx1 >> ignore >> ignore >> ignore;
                vertexStream2 >> idx2 >> ignore >> ignore >> ignore;
                vertexStream3 >> idx3 >> ignore >> ignore >> ignore;

                faces.emplace_back(idx1 - 1, idx2 - 1, idx3 - 1); // Adjust from 1-based to 0-based index
            }
        }
    }
}

Vector CowMesh::CalculateDimensions() {
    // Implementation for calculating dimensions
    Vector dimensions;
    return dimensions;
}

Point CowMesh::CalculateCenter() {
    // Implementation for calculating center
    Point center;
    return center;
}

Vector CowMesh::Dimensions() {
    if (vertices.empty())
        return Vector(0, 0, 0);

    float minX = vertices[0].x, maxX = vertices[0].x;
    float minY = vertices[0].y, maxY = vertices[0].y;
    float minZ = vertices[0].z, maxZ = vertices[0].z;

    for (const auto& v : vertices) {
        if (v.x < minX) minX = v.x;
        if (v.x > maxX) maxX = v.x;
        if (v.y < minY) minY = v.y;
        if (v.y > maxY) maxY = v.y;
        if (v.z < minZ) minZ = v.z;
        if (v.z > maxZ) maxZ = v.z;
    }

    return Vector(maxX - minX, maxY - minY, maxZ - minZ);
}

Point CowMesh::Center() {
    if (vertices.empty())
        return Point(0, 0, 0);

    float sumX = 0, sumY = 0, sumZ = 0;
    for (const auto& v : vertices) {
        sumX += v.x;
        sumY += v.y;
        sumZ += v.z;
    }

    float numVertices = static_cast<float>(vertices.size());
    return Point(sumX / numVertices, sumY / numVertices, sumZ / numVertices);
}

