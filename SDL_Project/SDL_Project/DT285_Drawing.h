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
   

       int startIndex = static_cast<unsigned int>(vertices.size() / 3) ;

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
    Matrix obj2dev = Proj * A ;




    // Transform all vertices and prepare for drawing
    for (int i = 0; i < m.VertexCount(); ++i) {
        Hcoords v = obj2dev * m.GetVertex(i);
        temp_verts[i] = (1.0f / v.w) * v;
    }

    // Prepare indices for edges
    for (int j = 0; j < m.EdgeCount(); j++) {
        const Point& P = temp_verts[m.GetEdge(j).index1],
            Q = temp_verts[m.GetEdge(j).index2];
        AddLineSegment(P, Q,color, vertices, indices,colors);
    }

    glUseProgram(shaderProgram); // Use the shader program

    // Set the color uniform in the shader
    GLint colorLocation = glGetUniformLocation(shaderProgram, "c_pos");
    glUniform3f(colorLocation, color.x, color.y, color.z);


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
    temp_verts.resize(m.VertexCount());
    transformed_verts.resize(m.VertexCount());
  
    Matrix obj2dev = Proj * A;
    Vector lightDir(0, 0, 1); // Ensure this direction is correct for your scene
    Point camPos(0, 0, 3); // Assuming the camera is looking towards the negative z-axis

    // Transform all vertices for lighting calculations
    for (int i = 0; i < m.VertexCount(); ++i) {
        temp_verts[i] = A * m.GetVertex(i);
    }


    // Prepare indices for edges
    for (int j = 0; j < m.FaceCount(); ++j) {
        Mesh::Face face = m.GetFace(j);

        const Point& P1 = temp_verts[face.index1];
        const Point& Q1 = temp_verts[face.index2];
        const Point& R1 = temp_verts[face.index3];

        // Calculate face normal for backface culling
        Vector normal = cross(Q1 - P1,R1 - P1);
        normal.Normalize();

        Vector viewVector = P1 - camPos; // Vector from camera position to a point on the face
  
        viewVector.Normalize();
    
       
        // Backface culling check
        if (dot(normal, viewVector) > 0) {
            continue;
        }

        lightDir.Normalize();
        // Calculate the diffuse lighting component
        float dotProduct = dot(lightDir, normal);
        float diffuse = dotProduct / abs(lightDir) * abs(normal);
 
        Vector finalColor = diffuse * color;
        
        // Transform the vertices for rendering
        Hcoords transformedP = Proj * temp_verts[face.index1];
        Hcoords transformedQ = Proj * temp_verts[face.index2];
        Hcoords transformedR = Proj * temp_verts[face.index3];

        const Point& P = (1.0f / transformedP.w) * transformedP;
        const Point& Q = (1.0f / transformedQ.w) * transformedQ;
        const Point& R = (1.0f / transformedR.w) * transformedR;

        FillFace(P, Q,R, finalColor, vertices, indices, colors);
    }


    glUseProgram(shaderProgram); // Use the shader program

    // Set the color uniform in the shader
    GLint colorLocation = glGetUniformLocation(shaderProgram, "c_pos");
    glUniform3f(colorLocation, color.x, color.y, color.z);

    glLineWidth(1); // Not work with filling, 1 for consistency
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertices.data());
    glColorPointer(3, GL_FLOAT, 0, colors.data());

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, indices.data());



    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
   
}


#endif
