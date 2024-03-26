#include "Projection.h"
#include <iostream>
Affine CameraToWorld(const Camera& cam) {
    return Affine(cam.Right(), cam.Up(), cam.Back(), cam.Eye());
}

Affine WorldToCamera(const Camera& cam) {
    Vector right = cam.Right();
    Vector up = cam.Up();
    Vector back = cam.Back();
    Point eye = cam.Eye();

    // Convert the eye point to a vector for translation
    Vector eyeVec(eye.x, eye.y, eye.z);

    // Create the rotation matrix
    Matrix rotation;
    rotation[0] = Hcoords(right.x, up.x, back.x, 0);
    rotation[1] = Hcoords(right.y, up.y, back.y, 0);
    rotation[2] = Hcoords(right.z, up.z, back.z, 0);
    rotation[3] = Hcoords(0, 0, 0, 1);

    // Create the translation matrix
    Matrix translation = Trans(-eyeVec);

    // Combine the rotation and translation
    return Affine(rotation * translation);
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