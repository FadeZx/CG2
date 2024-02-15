// DT285_Drawing.h

#ifndef DT285_DRAWING_H
#define DT285_DRAWING_H

#include <iostream>
#include <cstdlib>
#include "Affine.h"
#include "Mesh.h"
#include <vector>

using namespace std;

GLuint VBO, EBO, VAO;


vector<Point> temp_verts;

vector<float> vertices;
vector<unsigned int> indices;
vector<float> colors;


void InitBuffer() {
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Assume some data upload happens here with glBufferData

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute (adjust stride and offset as necessary)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

Matrix PerspectiveProjection(float dist)
{
    Matrix P;
    P.row[0] = Hcoords(1, 0, 0, 0);
    P.row[1] = Hcoords(0, 1, 0, 0);
    P.row[2] = Hcoords(0, 0, 0, 0);
    P.row[3] = Hcoords(0, 0, -1 / dist, 1);
    return P;
}

void AddLineSegment(const Point& start, const Point& end,const Vector& color,
    vector<float>& vertices, vector<unsigned int>& indices, vector<float>& colors) {
    int startIndex = vertices.size() / 3; // 3 components (x, y, z) per vertex

    // Add start and end points to the vertices list
    vertices.push_back(start.x);
    vertices.push_back(start.y);
    vertices.push_back(start.z);
    vertices.push_back(end.x);
    vertices.push_back(end.y);
    vertices.push_back(end.z);

    // Add color for both the start and end vertices
    for (int i = 0; i < 2; ++i) { // Repeat twice, once for start, once for end
        colors.push_back(color.x);
        colors.push_back(color.y);
        colors.push_back(color.z);
    }

    // Add indices for this line segment
    indices.push_back(startIndex);
    indices.push_back(startIndex + 1);
}

void DisplayEdges(Mesh& m, const Affine& A, const Matrix& Proj, const Vector& color) {

    vertices.clear();
    indices.clear();
    colors.clear();
   
    temp_verts.resize(m.EdgeCount());
    Matrix obj2dev = Proj * A ;


    // Transform all vertices and prepare for drawing
    for (int i = 0; i < m.VertexCount(); ++i) {
        Hcoords v = obj2dev * m.GetVertex(i);
        temp_verts[i] = (1.0f / v.w) * v;
    }

    // Prepare indices for edges
    for (int j = 0; j < m.EdgeCount(); ++j) {
        const Point& P = temp_verts[m.GetEdge(j).index1],
            Q = temp_verts[m.GetEdge(j).index2];
        AddLineSegment(P, Q,color, vertices, indices,colors);
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertices.data());
    glColorPointer(3, GL_FLOAT, 0, colors.data());

    glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, indices.data());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void DisplayFaces(Mesh& m, const Affine& A, const Matrix& Proj, const Vector& color);


#endif
