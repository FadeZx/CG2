#ifndef DT285_CAMERA_H
#define DT285_CAMERA_H

#include "Affine.h"


class Camera {
  public:
    Camera(void);
    Camera(const Point& E, const Vector& look, const Vector& up,
           float fov, float aspect=1, float near=0, float far=1);
    Point Eye(void) const;
    Vector Right(void) const;
    Vector Up(void) const;
    Vector Back(void) const;
    Vector ViewportGeometry(void) const;
    float NearDistance(void) const;
    float FarDistance(void) const;
    Camera& Zoom(float factor);
    Camera& Forward(float distance);
    Camera& Yaw(float angle);
    Camera& Pitch(float angle);
    Camera& Roll(float angle);
  private:
    Point eye;
    Vector right, up, back;
    float width, height, distance,
          near, far;
};


#endif

