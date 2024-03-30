// DT285_Drawing.h

#ifndef DT285_DRAWING_H
#define DT285_DRAWING_H

#include <iostream>
#include <cstdlib>
#include "Affine.h"
#include "Mesh.h"
#include <vector>
#include <GL/glew.h> 

using namespace std;

GLuint VBO, EBO, VAO;
GLuint shaderProgram;


vector<Point> temp_verts;
vector<Point> transformed_verts;
vector<Point> face_indices;

vector<float> vertices;
vector<unsigned int> indices;
vector<unsigned int> faces;

vector<float> colors;


void InitBuffer() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
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

void AddLineSegment(const Point& start, const Point& end, const Vector& color,
    vector<float>& vertices, vector<unsigned int>& indices, vector<float>& colors) {


    int startIndex = static_cast<unsigned int>(vertices.size() / 3);

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


void FillFace(const Point& p1, const Point& p2, const Point& p3, const Vector& color,
    vector<float>& vertices, vector<unsigned int>& indices, vector<float>& colors) {


    unsigned int baseIndex = static_cast<unsigned int>(vertices.size() / 3);
    // Add vertices to the vertex data array
    vertices.push_back(p1.x);
    vertices.push_back(p1.y);
    vertices.push_back(p1.z);

    vertices.push_back(p2.x);
    vertices.push_back(p2.y);
    vertices.push_back(p2.z);

    vertices.push_back(p3.x);
    vertices.push_back(p3.y);
    vertices.push_back(p3.z);

    // Add color data for each vertex
    for (int k = 0; k < 3; ++k) { // Each face has 3 vertices
        colors.push_back(color.x);
        colors.push_back(color.y);
        colors.push_back(color.z);
    }

    // Add indices for this face

    indices.push_back(baseIndex);
    indices.push_back(baseIndex + 1);
    indices.push_back(baseIndex + 2);


}

void DisplayEdges(Mesh& m, const Affine& A, const Matrix& Proj, const Vector& color) {

    vertices.clear();
    indices.clear();
    colors.clear();

    temp_verts.resize(m.VertexCount());
    Matrix obj2dev = Proj * A;




    // Transform all vertices and prepare for drawing
    for (int i = 0; i < m.VertexCount(); ++i) {
        Hcoords v = obj2dev * m.GetVertex(i);
        temp_verts[i] = (1.0f / v.w) * v;
    }

    // Prepare indices for edges
    for (int j = 0; j < m.EdgeCount(); j++) {
        const Point& P = temp_verts[m.GetEdge(j).index1],
            Q = temp_verts[m.GetEdge(j).index2];
        AddLineSegment(P, Q, color, vertices, indices, colors);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glUseProgram(shaderProgram);
    glUniform4f(glGetUniformLocation(shaderProgram, "c_pos"), color.x, color.y, color.z, 1.0f);

    glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

float magnitude(const Vector& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}



void DisplayFaces(Mesh& m, const Affine& A, const Matrix& Proj, const Vector& baseColor) {
    vertices.clear();
    indices.clear();
    colors.clear();
    temp_verts.resize(m.VertexCount());
    transformed_verts.resize(m.VertexCount());

    Matrix obj2dev = Proj * A;
    Vector lightDir(0, 0, 1);
    Point camPos(0, 0, 3);

    for (int i = 0; i < m.VertexCount(); ++i) {
        temp_verts[i] = A * m.GetVertex(i);
    }

    for (int j = 0; j < m.FaceCount(); ++j) {
        Mesh::Face face = m.GetFace(j);

        const Point& P1 = temp_verts[face.index1];
        const Point& Q1 = temp_verts[face.index2];
        const Point& R1 = temp_verts[face.index3];

        Vector normal = cross(Q1 - P1, R1 - P1);
        normal.Normalize();

        Vector viewVector = P1 - camPos;
        viewVector.Normalize();

        if (dot(normal, viewVector) > 0) {
            continue;
        }

        lightDir.Normalize();
        float dotProduct = dot(lightDir, normal);
        float diffuse = max(dotProduct, 0.0f);
        Vector finalColor = diffuse * baseColor;

        Hcoords transformedP = Proj * temp_verts[face.index1];
        Hcoords transformedQ = Proj * temp_verts[face.index2];
        Hcoords transformedR = Proj * temp_verts[face.index3];

        const Point& P = (1.0f / transformedP.w) * transformedP;
        const Point& Q = (1.0f / transformedQ.w) * transformedQ;
        const Point& R = (1.0f / transformedR.w) * transformedR;

        FillFace(P, Q, R, finalColor, vertices, indices, colors);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glUseProgram(shaderProgram);
    GLint colorLocation = glGetUniformLocation(shaderProgram, "c_pos");

    for (size_t i = 0; i < indices.size(); i += 3) {
        Vector faceColor = Vector(colors[3 * indices[i]], colors[3 * indices[i] + 1], colors[3 * indices[i] + 2]);
        glUniform4f(colorLocation, faceColor.x, faceColor.y, faceColor.z, 1.0f);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(i * sizeof(unsigned int)));
    }

    glBindVertexArray(0);
}



#endif
