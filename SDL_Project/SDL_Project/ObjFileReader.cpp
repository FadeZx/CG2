#include "ObjFileReader.h"
#include <algorithm> // for std::min and std::max

ObjFileReader::ObjFileReader(const std::string& filename) {
    loadFromObj(filename);
}

void ObjFileReader::loadFromObj(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    std::string line;
    std::set<std::pair<int, int>> edgeSet; // To store unique edges
    while (getline(file, line)) {
        std::stringstream ss(line);
        std::string type;
        ss >> type;
        if (type == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            vertices.emplace_back(x, y, z);
        } else if (type == "f") {
            std::string vertex1, vertex2, vertex3;
            int idx1, idx2, idx3;
            char ignore; 
            if (ss >> vertex1 >> vertex2 >> vertex3) {
                std::stringstream(vertex1) >> idx1 >> ignore >> ignore >> ignore;
                std::stringstream(vertex2) >> idx2 >> ignore >> ignore >> ignore;
                std::stringstream(vertex3) >> idx3 >> ignore >> ignore >> ignore;

                faces.emplace_back(idx1 - 1, idx2 - 1, idx3 - 1);

                std::pair<int, int> edges[3] = {
                    {std::min(idx1, idx2) - 1, std::max(idx1, idx2) - 1},
                    {std::min(idx2, idx3) - 1, std::max(idx2, idx3) - 1},
                    {std::min(idx3, idx1) - 1, std::max(idx3, idx1) - 1}
                };
                for (auto& edge : edges) {
                    edgeSet.insert(edge);
                }
            }
        }
    }
    for (auto& edge : edgeSet) {
        edges.emplace_back(edge.first, edge.second);
    }
}

const std::vector<Point>& ObjFileReader::getVertices() const {
    return vertices;
}

const std::vector<Mesh::Face>& ObjFileReader::getFaces() const {
    return faces;
}

const std::vector<Mesh::Edge>& ObjFileReader::getEdges() const {
    return edges;
}

Vector ObjFileReader::calculateDimensions() const {
    if (vertices.empty())
        return Vector(0, 0, 0);

    float minX = vertices[0].x, maxX = vertices[0].x;
    float minY = vertices[0].y, maxY = vertices[0].y;
    float minZ = vertices[0].z, maxZ = vertices[0].z;

    for (const auto& vertex : vertices) {
        minX = std::min(minX, vertex.x);
        maxX = std::max(maxX, vertex.x);
        minY = std::min(minY, vertex.y);
        maxY = std::max(maxY, vertex.y);
        minZ = std::min(minZ, vertex.z);
        maxZ = std::max(maxZ, vertex.z);
    }

    return Vector(maxX - minX, maxY - minY, maxZ - minZ);
}

Point ObjFileReader::calculateCenter() const {
    if (vertices.empty())
        return Point(0, 0, 0);

    float sumX = 0, sumY = 0, sumZ = 0;
    for (const auto& vertex : vertices) {
        sumX += vertex.x;
        sumY += vertex.y;
        sumZ += vertex.z;
    }
    float numVertices = static_cast<float>(vertices.size());
    return Point(sumX / numVertices, sumY / numVertices, sumZ / numVertices);
}
