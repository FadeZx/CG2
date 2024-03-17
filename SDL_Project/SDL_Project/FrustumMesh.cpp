#include "FrustumMesh.h"
#include <cmath>

// Define the edges and faces arrays
const Mesh::Edge FrustumMesh::edges[16] = {
    Edge(0, 1), Edge(0, 2), Edge(0, 3), Edge(0, 4), // Edges from center of projection to near plane
    Edge(1, 2), Edge(2, 3), Edge(3, 4), Edge(4, 1), // Edges of the near plane
    Edge(5, 6), Edge(6, 7), Edge(7, 8), Edge(8, 5), // Edges of the far plane
    Edge(1, 5), Edge(2, 6), Edge(3, 7), Edge(4, 8)  // Edges connecting near and far planes
};

const Mesh::Face FrustumMesh::faces[12] = {
    Face(1, 2, 3), Face(1, 3, 4), // Near face
    Face(1, 5, 6), Face(1, 6, 2), // Bottom face
    Face(2, 6, 7), Face(2, 7, 3), // Right face
    Face(3, 7, 8), Face(3, 8, 4), // Top face
    Face(4, 8, 5), Face(4, 5, 1), // Left face
    Face(5, 8, 7), Face(5, 7, 6)  // Far face
};



FrustumMesh::FrustumMesh(float fov, float aspect, float near, float far) {
    // Calculate the dimensions of the near and far planes
    float nearHeight = 1.0f * tan(fov / 2.0f) * near;
    float nearWidth = nearHeight * aspect;
    float farHeight = 1.0f * tan(fov / 2.0f) * far;
    float farWidth = farHeight * aspect;

    // Define the vertices
    vertices[0] = Point(0, 0, 0); // Center of projection
    vertices[1] = Point(-nearWidth / 2, -nearHeight / 2, -near); // Near bottom left
    vertices[2] = Point(nearWidth / 2, -nearHeight / 2, -near);  // Near bottom right
    vertices[3] = Point(nearWidth / 2, nearHeight / 2, -near);   // Near top right
    vertices[4] = Point(-nearWidth / 2, nearHeight / 2, -near);  // Near top left
    vertices[5] = Point(-farWidth / 2, -farHeight / 2, -far);    // Far bottom left
    vertices[6] = Point(farWidth / 2, -farHeight / 2, -far);     // Far bottom right
    vertices[7] = Point(farWidth / 2, farHeight / 2, -far);      // Far top right
    vertices[8] = Point(-farWidth / 2, farHeight / 2, -far);     // Far top left

    // Center and dimensions of the frustum
    center = Point(0, 0, -(near + far) / 2);
    dimensions = Vector(farWidth, farHeight, far - near);
}

int FrustumMesh::VertexCount(void) {
    return 9;
}

Point FrustumMesh::GetVertex(int i) {
    return vertices[i];
}

Vector FrustumMesh::Dimensions(void) {
    return dimensions;
}

Point FrustumMesh::Center(void) {
    return center;
}

int FrustumMesh::FaceCount(void) {
    return 12;
}

Mesh::Face FrustumMesh::GetFace(int i) {
    return faces[i];
}

int FrustumMesh::EdgeCount(void) {
    return 16;
}

Mesh::Edge FrustumMesh::GetEdge(int i) {
    return edges[i];
}
