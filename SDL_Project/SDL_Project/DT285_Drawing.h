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
vector<Point> face_indices;

vector<float> vertices;
vector<unsigned int> indices;
vector<unsigned int> faces;
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
     int startIndex = vertices.size() / 3; // 3 components (x, y, z) per vertex
    indices.push_back(startIndex);
    indices.push_back(startIndex + 1);
}


void FillFace(const Point& p1, const Point& p2, const Point& p3, const Vector& color,
    vector<float>& vertices, vector<unsigned int>& indices, vector<float>& colors) {



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
        unsigned int baseIndex = static_cast<unsigned int>(vertices.size() / 3) - 3;
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
    

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

float magnitude(const Vector& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

void DisplayFaces(Mesh& m, const Affine& A, const Matrix& Proj, const Vector& color)
{
    vertices.clear();
    indices.clear();
    colors.clear();
    temp_verts.resize(m.EdgeCount());
    Matrix obj2dev = Proj * A;

    Vector lightDir(0, 0, 1); // Ensure this direction is correct for your scene
    Vector viewVector(0, 0, 3); // Assuming the camera is looking towards the negative z-axis

    // Transform all vertices and prepare for drawing
    for (int i = 0; i < m.VertexCount(); ++i) {
        Hcoords v = obj2dev * m.GetVertex(i);
        temp_verts[i] = (1.0f / v.w) * v;
    }

    // Prepare indices for edges
    for (int j = 0; j < m.FaceCount(); ++j) {
        const Point& P = temp_verts[m.GetFace(j).index1],
            Q = temp_verts[m.GetFace(j).index2],R = temp_verts[m.GetFace(j).index3];

        Mesh::Face face = m.GetFace(j);

        // Calculate face normal for backface culling
        Vector normal = cross(
            temp_verts[face.index2] - temp_verts[face.index1],
            temp_verts[face.index3] - temp_verts[face.index1]
        );

       
        // Backface culling check
        if (dot(normal, viewVector) <= 0) {
            continue;
        }

        // Calculate the diffuse lighting component
        float dotProduct = dot(lightDir, normal);
        float diffuse = std::max(dotProduct, 0.0f);
 
        Vector finalColor = diffuse * color;
 
        FillFace(P, Q,R, finalColor, vertices, indices, colors);
    }

    glLineWidth(1); // Not work with filling, 1 for consistency
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertices.data());
    glColorPointer(3, GL_FLOAT, 0, colors.data());

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, indices.data());


    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
   
}


#endif
