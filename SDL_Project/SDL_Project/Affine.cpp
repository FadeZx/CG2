#include "Affine.h"


Hcoords::Hcoords(void) : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}

Hcoords::Hcoords(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {}

Point::Point(void) : Hcoords() { assert(Hcoords::Near(w, 1.0f)); }

Point::Point(float X, float Y, float Z) : Hcoords(X, Y, Z, 1.0f) { assert(Hcoords::Near(w, 1.0f)); }

Vector::Vector(void) : Hcoords() { assert(Hcoords::Near(w, 1.0f)); }

Vector::Vector(float X, float Y, float Z) : Hcoords(X, Y, Z, 0.0f) { assert(Hcoords::Near(w, 0.0f)); }


bool Vector::Normalize(void) {
    float length = abs(*this);
    if (length > 0.0f) {
        float invLength = 1.0f / length;
        x *= invLength;
        y *= invLength;
        z *= invLength;
        w = 0.0f;
        return true;
    }
    return false;
}

Matrix::Matrix(void) {
    for (int i = 0; i < 4; ++i) {
        row[i] = Hcoords();
    }
}

Affine::Affine(void) : Matrix() {}

Affine::Affine(const Vector& Lx, const Vector& Ly, const Vector& Lz, const Point& D) {
    row[0] = Hcoords(Lx.x, Ly.x, Lz.x, D.x);
    row[1] = Hcoords(Lx.y, Ly.y, Lz.y, D.y);
    row[2] = Hcoords(Lx.z, Ly.z, Lz.z, D.z);
    row[3] = Hcoords(0.0f, 0.0f, 0.0f, 1.0f);
}


Hcoords operator+(const Hcoords& u, const Hcoords& v) {
    assert((u.w == 1.0f && v.w == 0.0f) || (u.w == 0.0f && v.w == 1.0f) || (u.w == 0.0f && v.w == 0.0f));
    return Hcoords(u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w);
}


Hcoords operator-(const Hcoords& u, const Hcoords& v) {
    return Hcoords(u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w);
}

Hcoords operator-(const Hcoords& v) {
    return Hcoords(-v.x, -v.y, -v.z, -v.w);
}

Hcoords operator*(float r, const Hcoords& v) {
    return Hcoords(r * v.x, r * v.y, r * v.z, r * v.w);
}

Hcoords operator*(const Matrix& A, const Hcoords& v) {
    Hcoords result;
    for (int i = 0; i < 4; ++i) {
        result[i] = A[i][0] * v.x + A[i][1] * v.y + A[i][2] * v.z + A[i][3] * v.w;
    }
    return result;
}

Matrix operator*(const Matrix& A, const Matrix& B) {
    Matrix result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result[i][j] = A[i][0] * B[0][j] + A[i][1] * B[1][j] + A[i][2] * B[2][j] + A[i][3] * B[3][j];
        }
    }
    return result;
}

float dot(const Vector& u, const Vector& v) {
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

float abs(const Vector& v) {
    return std::sqrt(dot(v, v));
}

Vector cross(const Vector& u, const Vector& v) {
    return Vector(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
}

Affine Rot(float t, const Vector& v) {
    float cosT = std::cos(t);
    float sinT = std::sin(t);
    Vector normV = v;
    normV.Normalize();
    float oneMinusCosT = 1.0f - cosT;

    Matrix rotationMatrix;
    rotationMatrix[0] = Hcoords(cosT + oneMinusCosT * normV.x * normV.x, oneMinusCosT * normV.x * normV.y - sinT * normV.z, oneMinusCosT * normV.x * normV.z + sinT * normV.y, 0.0f);
    rotationMatrix[1] = Hcoords(oneMinusCosT * normV.x * normV.y + sinT * normV.z, cosT + oneMinusCosT * normV.y * normV.y, oneMinusCosT * normV.y * normV.z - sinT * normV.x, 0.0f);
    rotationMatrix[2] = Hcoords(oneMinusCosT * normV.x * normV.z - sinT * normV.y, oneMinusCosT * normV.y * normV.z + sinT * normV.x, cosT + oneMinusCosT * normV.z * normV.z, 0.0f);
    rotationMatrix[3] = Hcoords(0.0f, 0.0f, 0.0f, 1.0f);

    return Affine(rotationMatrix);
}

Affine RotY(float t) {
    float cosT = std::cos(t);
    float sinT = std::sin(t);

    Matrix rotationMatrix;
    rotationMatrix[0] = Hcoords(cosT, 0.0f, sinT, 0.0f);
    rotationMatrix[1] = Hcoords(0.0f, 1.0f, 0.0f, 0.0f);
    rotationMatrix[2] = Hcoords(-sinT, 0.0f, cosT, 0.0f);
    rotationMatrix[3] = Hcoords(0.0f, 0.0f, 0.0f, 1.0f);

    return Affine(rotationMatrix);
}


Affine Trans(const Vector& v) {
    Affine result;
    result[0] = Hcoords(1.0f, 0.0f, 0.0f, v.x);
    result[1] = Hcoords(0.0f, 1.0f, 0.0f, v.y);
    result[2] = Hcoords(0.0f, 0.0f, 1.0f, v.z);
    result[3] = Hcoords(0.0f, 0.0f, 0.0f, 1.0f);
    return result;
}

Affine Scale(float r) {
    Affine result;
    result[0] = Hcoords(r, 0.0f, 0.0f, 0.0f);
    result[1] = Hcoords(0.0f, r, 0.0f, 0.0f);
    result[2] = Hcoords(0.0f, 0.0f, r, 0.0f);
    result[3] = Hcoords(0.0f, 0.0f, 0.0f, 1.0f);
    return result;
 }

Affine Scale(float rx, float ry, float rz) {
        
    Affine result;
    result[0] = Hcoords(rx, 0.0f, 0.0f, 0.0f);
    result[1] = Hcoords(0.0f, ry, 0.0f, 0.0f);
    result[2] = Hcoords(0.0f, 0.0f, rz, 0.0f);
    result[3] = Hcoords(0.0f, 0.0f, 0.0f, 1.0f);
    return result;

}

Affine Inverse(const Affine& A)
{
    float det = A[0][0] * A[1][1] * A[2][2]
        + A[0][1] * A[1][2] * A[2][0]
        + A[0][2] * A[1][0] * A[2][1]
        - A[2][0] * A[1][1] * A[0][2]
        - A[2][1] * A[1][2] * A[0][0]
        - A[2][2] * A[1][0] * A[0][1];

    Affine inverseTran{ {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {-A[0][3], -A[1][3], -A[2][3]} };

    Affine adj{
        (1 / det) * Vector {
 A[1][1] * A[2][2] - A[2][1] * A[1][2],    -(A[1][0] * A[2][2] - A[2][0] * A[1][2]),   A[1][0] * A[2][1] - A[2][0] * A[1][1]
},
(1 / det) * Vector {
-(A[0][1] * A[2][2] - A[2][1] * A[0][2]), A[0][0] * A[2][2] - A[2][0] * A[0][2],      -(A[0][0] * A[2][1] - A[2][0] * A[0][1])
},
(1 / det) * Vector {
A[0][1] * A[1][2] - A[1][1] * A[0][2],    -(A[0][0] * A[1][2] - A[1][0] * A[0][2]),   A[0][0] * A[1][1] - A[1][0] * A[0][1]
},
{}
    };

    return adj * inverseTran;
}
