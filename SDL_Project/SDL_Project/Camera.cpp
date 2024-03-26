#include "Camera.h"
#include "Affine.h"
#include <cmath>

Camera::Camera(void)
    : eye(Point(0, 0, 0)),
    right(Vector(1, 0, 0)),
    up(Vector(0, 1, 0)),
    back(Vector(0, 0, -1)),
    width(1),
    height(1),
    distance(1),
    near(0.1f),
    far(10.0f) {
}

Camera::Camera(const Point& E, const Vector& look, const Vector& up, float fov, float aspect, float near, float far)
    : eye(E),
    near(near),
    far(far) {
    back = -look;
    back.Normalize();
    right = cross(up, back);
    right.Normalize();
    this->up = cross(back, right);
    distance = 1.0f / tan(fov / 2.0f);
    width = 2.0f * distance * tan(fov/2);
    height = width / aspect;
}


Point Camera::Eye(void) const {
    return eye;
}

Vector Camera::Right(void) const {
    return right;
}

Vector Camera::Up(void) const {
    return up;
}

Vector Camera::Back(void) const {
    return back;
}

Vector Camera::ViewportGeometry(void) const {
    return Vector(width, height, distance);
}

float Camera::NearDistance(void) const {
    return near;
}

float Camera::FarDistance(void) const {
    return far;
}

Camera& Camera::Zoom(float factor) {
    width /= factor; 
    height /= factor; 
    return *this;
}

Camera& Camera::Forward(float distance) {
    eye = eye - distance * back;
    return *this;
}

Camera& Camera::Yaw(float angle) {
    Affine R = Rot(angle, up);
    right = R * right;
    back = R * back;
    return *this;
}

Camera& Camera::Pitch(float angle) {
    Affine R = Rot(angle, right);
    up = R * up;
    back = R * back;
    return *this;
}

Camera& Camera::Roll(float angle) {
    Affine R = Rot(angle, back);
    right = R * right;
    up = R * up;
    return *this;
}

