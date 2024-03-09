#include "CubeMesh.h"

const Point CubeMesh::vertices[8] = {
    Point(-1, -1, -1), Point(1, -1, -1), 
    Point(1, 1, -1), Point(-1, 1, -1),
    Point(-1, -1, 1), Point(1, -1, 1), 
    Point(1, 1, 1), Point(-1, 1, 1)
};

const Mesh::Edge CubeMesh::edges[12] = {
    Edge(0, 1), Edge(1, 2), Edge(2, 3), Edge(3, 0),
    Edge(4, 5), Edge(5, 6), Edge(6, 7), Edge(7, 4),
    Edge(0, 4), Edge(1, 5), Edge(2, 6), Edge(3, 7)
};

const Mesh::Face CubeMesh::faces[12] = {
    Face(0, 1, 2), Face(3, 0, 2),
    Face(6, 7, 4), Face(4, 5, 6),
    Face(0, 1, 5), Face(0, 5, 4),
    Face(6, 2, 3), Face(6, 3, 7),
    Face(5, 1, 2), Face(2, 6, 5),
    Face(4, 7, 3), Face(3, 0, 4)
};


