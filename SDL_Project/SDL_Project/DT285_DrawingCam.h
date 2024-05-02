// cs250_h4.h
// -- display edges/faces of a mesh, second version
// cs250 1/15

#ifndef DT285_DRAWING_CAM_H
#define DT285_DRAWING_CAM_H

#include <cstdlib>
#include "Camera.h"
#include "Mesh.h"
#include "Projection.h"
#include "Affine.h"
#include <vector>
#include <GL/glew.h>  
#include <iostream>

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

void DisplayEdges(Mesh& mesh, const Affine& obj2world, const Camera& cam, const Vector& clr)
{
    vertices.clear();
    indices.clear();
    colors.clear();

    const Matrix viewMatrix = WorldToCamera(cam) * obj2world;
    const Matrix modelMatrix = CameraToNDC(cam) * viewMatrix;

    for (int i = 0; i < mesh.EdgeCount(); ++i) {
        Mesh::Edge edge = mesh.GetEdge(i);

        Hcoords p1 = modelMatrix * mesh.GetVertex(edge.index1);
        Hcoords p2 = modelMatrix * mesh.GetVertex(edge.index2);

        if (p1.z < 0 || p2.z < 0) continue; 

        Point start(p1.x / p1.w, p1.y / p1.w, p1.z / p1.w);
        Point end(p2.x / p2.w, p2.y / p2.w, p2.z / p2.w);

        AddLineSegment(start, end, clr, vertices, indices, colors);
    }
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glUseProgram(shaderProgram);
    glUniform4f(glGetUniformLocation(shaderProgram, "c_pos"), clr.x, clr.y, clr.z, 1.0f);

    glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    
}

void DisplayFaces(Mesh& mesh, const Affine& obj2world, const Camera& cam, const Vector& clr)
{
    vertices.clear();
    indices.clear();
    colors.clear();

    Point Eye = cam.Eye();
    Vector LightDir = cam.Back();
    float Lmag = abs(LightDir);

    Matrix viewMatrix = WorldToCamera(cam);
    Matrix projMatrix = CameraToNDC(cam);

    for (int i = 0; i < mesh.FaceCount(); i++)
    {
        Mesh::Face face = mesh.GetFace(i);
        Hcoords P = obj2world * mesh.GetVertex(face.index1);
        Hcoords Q = obj2world * mesh.GetVertex(face.index2);
        Hcoords R = obj2world * mesh.GetVertex(face.index3);

        Hcoords P1 = viewMatrix * P;
        Hcoords Q1 = viewMatrix * Q;
        Hcoords R1 = viewMatrix * R;

        if (P1.z >= 0 || Q1.z >= 0 || R1.z >= 0)
            continue;

        Vector normal = cross(Q - P, R - P);
        normal.Normalize();

        if (dot(normal, Eye - P) <= 0)
            continue;

        Hcoords projV1 = projMatrix * P1;
        Hcoords projV2 = projMatrix * Q1;
        Hcoords projV3 = projMatrix * R1;

        float nMag = abs(normal);
        float diffuse = dot(LightDir, normal) / (Lmag * nMag);
        Vector diffusedColor = diffuse * clr;

        Point v1(projV1.x / projV1.w, projV1.y / projV1.w, projV1.z / projV1.w);
        Point v2(projV2.x / projV2.w, projV2.y / projV2.w, projV2.z / projV2.w);
        Point v3(projV3.x / projV3.w, projV3.y / projV3.w, projV3.z / projV3.w);

        FillFace(v1, v2, v3, diffusedColor, vertices, indices, colors);
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

