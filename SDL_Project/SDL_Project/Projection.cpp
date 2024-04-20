#include "Projection.h"
#include <iostream>

Affine CameraToWorld(const Camera& cam)
{
  return { cam.Right(), cam.Up(), cam.Back(), cam.Eye() };
}

Affine WorldToCamera(const Camera& cam)
{
    return Inverse(CameraToWorld(cam));
}




Matrix CameraToNDC(const Camera& cam) {
    float W = cam.ViewportGeometry().x;
    float H = cam.ViewportGeometry().y;
    float D = cam.ViewportGeometry().z;
    float n = cam.NearDistance();
    float f = cam.FarDistance();


    Matrix NDC;
    NDC[0] = Hcoords((2 * D )/ W, 0, 0, 0);
    NDC[1] = Hcoords(0, (2 * D) / H, 0, 0);
    NDC[2] = Hcoords(0, 0, (n + f) / (n - f), (2 * n * f) / (n - f));
    NDC[3] = Hcoords(0, 0, -1, 0); 
    return NDC;
}