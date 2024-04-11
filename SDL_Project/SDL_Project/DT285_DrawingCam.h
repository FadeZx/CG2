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

void DisplayEdges(Mesh& mesh, const Affine& obj2world, const Camera& cam, const Vector& clr) {
  
    Affine viewMatrix = WorldToCamera(cam);
    Matrix projectionMatrix = CameraToNDC(cam);
  
    vertices.clear();
    indices.clear();
    colors.clear();

    temp_verts.resize(mesh.VertexCount());

    Matrix obj2dev = projectionMatrix * viewMatrix;


    // Prepare indices for edges
    for (int j = 0; j < mesh.EdgeCount(); j++) {

        Mesh::Edge edge = mesh.GetEdge(j);

        Hcoords P = obj2world * mesh.GetVertex(edge.index1);
        Hcoords Q = obj2world * mesh.GetVertex(edge.index2);

        Hcoords Pcam = WorldToCamera(cam) * P;
        Hcoords Qcam = WorldToCamera(cam) * Q;

        if (Pcam.z >= 0 || Qcam.z >= 0) {
            continue; 
        }

        P = obj2dev * P;
        Q = obj2dev * Q;

        P.x /= P.w;
        P.y /= P.w;
        P.z /= P.w;
        P = (1.0f / P.w) * P;

        Q.x /= Q.w;
        Q.y /= Q.w;
        Q.z /= Q.w;
        Q = (1.0f / Q.w) * Q;

        AddLineSegment(P, Q, clr, vertices, indices, colors);
        
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

void DisplayFaces(Mesh& mesh, const Affine& obj2world, const Camera& cam, const Vector& clr) {


    vertices.clear();
    indices.clear();
    colors.clear();
    temp_verts.resize(mesh.VertexCount());
    transformed_verts.resize(mesh.VertexCount());

    Affine viewMatrix = WorldToCamera(cam);
    Matrix projectionMatrix = CameraToNDC(cam);

    Matrix obj2dev = projectionMatrix * viewMatrix;

    Vector lightDir = cam.Back();
    lightDir.Normalize();
    Point camPos = cam.Eye();

    for (int i = 0; i < mesh.VertexCount(); ++i) {
        temp_verts[i] = obj2world * mesh.GetVertex(i);
    }

    for (int j = 0; j < mesh.FaceCount(); ++j) {

        Mesh::Face face = mesh.GetFace(j);

        Point P1 = temp_verts[face.index1];
        Point Q1 = temp_verts[face.index2];
        Point R1 = temp_verts[face.index3];

        Point P1cam = viewMatrix * P1;
        Point Q1cam = viewMatrix * Q1;
        Point R1cam = viewMatrix * R1;

        if (P1cam.z >= 0 || Q1cam.z >= 0 || R1cam.z >= 0) {
            continue;
        }

        Vector normal = cross(Q1 - P1, R1 - P1);
        normal.Normalize();

        Vector viewVector = P1 - camPos;
        viewVector.Normalize();

        if (dot(normal, viewVector) > 0) {
            continue;
        }

        float dotProduct = dot(lightDir, normal);
        float diffuse = max(dotProduct, 0.0f);
        Vector finalColor = diffuse * clr;


        Hcoords transformedP = obj2dev * P1;
        Hcoords transformedQ = obj2dev * Q1;
        Hcoords transformedR = obj2dev * R1;


        transformedP.x /= transformedP.w;
        transformedQ.x /= transformedQ.w;
        transformedR.x /= transformedR.w;

        transformedP.y /= transformedP.w;
        transformedQ.y /= transformedQ.w;
        transformedR.y /= transformedR.w;

        transformedP.z /= transformedP.w;
        transformedQ.z /= transformedQ.w;
        transformedR.z /= transformedR.w;

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

