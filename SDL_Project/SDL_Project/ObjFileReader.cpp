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

    while (std::getline(file, line)) {
        // Handle comments
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);

        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string type;
        ss >> type;
        if (type == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            vertices.emplace_back(x, y, z);
        }
        else if (type == "f") {
            std::vector<int> indices;
            std::string vertexData;
            while (ss >> vertexData) {
                std::replace(vertexData.begin(), vertexData.end(), '/', ' ');
                std::stringstream vertexStream(vertexData);
                int vertexIndex, textureIndex, normalIndex;
                vertexStream >> vertexIndex; // Parse vertex index

                // Handle relative indices
                if (vertexIndex < 0) vertexIndex = vertices.size() + vertexIndex + 1;
                if (vertexIndex > 0) indices.push_back(vertexIndex - 1); // Convert to 0-based index
            }
            // Add faces and edges
            for (size_t i = 2; i < indices.size(); ++i) {
                faces.emplace_back(indices[0], indices[i - 1], indices[i]);
                std::pair<int, int> edges[] = {
                    {std::min(indices[0], indices[i - 1]), std::max(indices[0], indices[i - 1])},
                    {std::min(indices[i - 1], indices[i]), std::max(indices[i - 1], indices[i])},
                    {std::min(indices[i], indices[0]), std::max(indices[i], indices[0])}
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
