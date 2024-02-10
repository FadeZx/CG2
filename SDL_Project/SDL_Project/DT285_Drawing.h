// DT285_Drawing.h

#ifndef DT285_DRAWING_H
#define DT285_DRAWING_H

#include <cstdlib>
#include "Affine.h"
#include "Mesh.h"

GLuint VBO, EBO, VAO;

void InitBuffer() {
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Upload vertex data to VBO

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // Upload index data to EBO

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    // Set up vertex attribute pointers
}

Matrix PerspectiveProjection(float dist)
{
    Matrix P;
    P.row[0] = Hcoords(1, 0, 0, 0);
    P.row[1] = Hcoords(0, 1, 0, 0);
    P.row[2] = Hcoords(0, 0, 1, 0);
    P.row[3] = Hcoords(0, 0, -1 / dist, 0);
    return P;
}

void DisplayEdges(Mesh& m, const Affine& A, const Matrix& Proj, const Vector& color) {
    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // Transform all vertices and prepare for drawing
    for (int i = 0; i < m.VertexCount(); ++i) {
        Point transformedVertex = Proj * A * m.GetVertex(i);
        vertices.push_back(transformedVertex.x / transformedVertex.w); // Assuming perspective division
        vertices.push_back(transformedVertex.y / transformedVertex.w);
        vertices.push_back(transformedVertex.z / transformedVertex.w);
    }

    // Prepare indices for edges
    for (int i = 0; i < m.EdgeCount(); ++i) {
        Mesh::Edge edge = m.GetEdge(i);
        indices.push_back(edge.index1);
        indices.push_back(edge.index2);
    }

    // Upload vertex data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Upload index data to EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Set up vertex attribute pointers
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Draw
    glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);
}

void DisplayFaces(Mesh& m, const Affine& A, const Matrix& Proj, const Vector& color);


#endif
