#include "Affine.h"
#include "Projection.h"
#include <iostream>

using namespace std;


int main(int argc, char* argv[]) {
    Point O(0, 0, 0);
    Vector ex(1, 0, 0), ey(0, 1, 0), ez(0, 0, 1);
    float aspect = 800.0f / 500.0f;

    Camera cam = Camera(O + ez, -ez, ey, 1.0f, aspect, 0.01f, 1);

    Point p = Point(1, 1, 1);
    Affine camera_to_world = CameraToWorld(cam);
    Affine world_to_camera = WorldToCamera(cam);
    Matrix ndc_transform = CameraToNDC(cam);

    Hcoords p1 = camera_to_world * p;
    cout << p1.x << "   " << p1.y << "   " << p1.z << "   " << p1.w << endl;
    Hcoords p2 = world_to_camera * p;
    cout << p2.x << "   " << p2.y << "   " << p2.z << "   " << p2.w << endl;
    Hcoords p3 = ndc_transform * p;
    cout << p3.x << "   " << p3.y << "   " << p3.z << "   " << p3.w << endl;

    cout << "-----------------------------------" << endl;
    Affine identity = world_to_camera * camera_to_world;
    cout << identity.row[0].x << "   " << identity.row[0].y << "   " << identity.row[0].z << "   " << identity.row[0].w << endl;
    cout << identity.row[1].x << "   " << identity.row[1].y << "   " << identity.row[1].z << "   " << identity.row[1].w << endl;
    cout << identity.row[2].x << "   " << identity.row[2].y << "   " << identity.row[2].z << "   " << identity.row[2].w << endl;
    cout << identity.row[3].x << "   " << identity.row[3].y << "   " << identity.row[3].z << "   " << identity.row[3].w << endl;
    cout << "-----------------------------------" << endl;

    return 0;
}