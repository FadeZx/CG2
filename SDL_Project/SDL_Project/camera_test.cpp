#include "Affine.h"
#include "Camera.h"
#include <iostream>

using namespace std;

const float PI = 4.0f * atan(1.0f);
const Point O(0, 0, 0);
const Vector ex(1, 0, 0), ey(0, 1, 0), ez(0, 0, 1);
const float angle_increment = PI / 180.0f;
const float dist_increment = 0.1f;
const float zoom_increment = 0.95f;

Camera camera;

int main(int argc, char* argv[]) {
	Camera camera = Camera(O - 5 * ez, -ez, ey, 0.5f * PI, 1.5f, 1, 3);
	cout << "######## start ########" << endl;
	cout << camera.Right().x << ",  " << camera.Right().y << ",  " << camera.Right().z << endl;
	cout << camera.Up().x << ",  " << camera.Up().y << ",  " << camera.Up().z << endl;
	cout << camera.Back().x << ",  " << camera.Back().y << ",  " << camera.Back().z << endl;
	cout << camera.Eye().x << ",  " << camera.Eye().y << ",  " << camera.Eye().z << endl;
	cout << "Pitch" << endl;
	camera.Pitch(angle_increment);
	cout << "######## pitch ########" << endl;
	cout << camera.Right().x << ",  " << camera.Right().y << ",  " << camera.Right().z << endl;
	cout << camera.Up().x << ",  " << camera.Up().y << ",  " << camera.Up().z << endl;
	cout << camera.Back().x << ",  " << camera.Back().y << ",  " << camera.Back().z << endl;
	cout << camera.Eye().x << ",  " << camera.Eye().y << ",  " << camera.Eye().z << endl;
	camera.Yaw(angle_increment);
	cout << "######## yaw ########" << endl;
	cout << camera.Right().x << ",  " << camera.Right().y << ",  " << camera.Right().z << endl;
	cout << camera.Up().x << ",  " << camera.Up().y << ",  " << camera.Up().z << endl;
	cout << camera.Back().x << ",  " << camera.Back().y << ",  " << camera.Back().z << endl;
	cout << camera.Eye().x << ",  " << camera.Eye().y << ",  " << camera.Eye().z << endl;
	camera.Roll(angle_increment);
	cout << "######## roll ########" << endl;
	cout << camera.Right().x << ",  " << camera.Right().y << ",  " << camera.Right().z << endl;
	cout << camera.Up().x << ",  " << camera.Up().y << ",  " << camera.Up().z << endl;
	cout << camera.Back().x << ",  " << camera.Back().y << ",  " << camera.Back().z << endl;
	cout << camera.Eye().x << ",  " << camera.Eye().y << ",  " << camera.Eye().z << endl;
	camera.Forward(dist_increment);
	cout << "######## forward ########" << endl;
	cout << camera.Right().x << ",  " << camera.Right().y << ",  " << camera.Right().z << endl;
	cout << camera.Up().x << ",  " << camera.Up().y << ",  " << camera.Up().z << endl;
	cout << camera.Back().x << ",  " << camera.Back().y << ",  " << camera.Back().z << endl;
	cout << camera.Eye().x << ",  " << camera.Eye().y << ",  " << camera.Eye().z << endl;
	camera.Zoom(zoom_increment);
	cout << "######## zoom 1 ########" << endl;
	cout << camera.Right().x << ",  " << camera.Right().y << ",  " << camera.Right().z << endl;
	cout << camera.Up().x << ",  " << camera.Up().y << ",  " << camera.Up().z << endl;
	cout << camera.Back().x << ",  " << camera.Back().y << ",  " << camera.Back().z << endl;
	cout << camera.Eye().x << ",  " << camera.Eye().y << ",  " << camera.Eye().z << endl;
	camera.Zoom(1.0f / zoom_increment);
	cout << "######## zoom 2 ########" << endl;
	cout << camera.Right().x << ",  " << camera.Right().y << ",  " << camera.Right().z << endl;
	cout << camera.Up().x << ",  " << camera.Up().y << ",  " << camera.Up().z << endl;
	cout << camera.Back().x << ",  " << camera.Back().y << ",  " << camera.Back().z << endl;
	cout << camera.Eye().x << ",  " << camera.Eye().y << ",  " << camera.Eye().z << endl;
	return 0;
}
