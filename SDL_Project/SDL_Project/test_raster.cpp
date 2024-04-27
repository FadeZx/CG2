#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include "RasterUtilities.h"
#include "Projection.h"
#include <iostream>


using namespace std;


// global data (ugh)
const int width = 600, height = 600;
const char* name = "DT285 Raster";
double time_last;
GLuint programID;
bool should_close = false;

const float pi = 4.0f * atan(1.0f);
const Point O(0, 0, 0);
const Vector ex(1, 0, 0),
ey(0, 1, 0),
ez(0, 0, 1);

struct Sphere {
	Point center;
	float radius;
	Vector color;
};

Camera camera;
Matrix world_to_device;
Raster* raster;
Raster::byte* frame_buffer0, * frame_buffer;
float* zbuffer0, * zbuffer;

Point S(-2, 0, -2.1f),
C(2, 0, -6.1f);
Vector a0, b0;
float rot_rate = 2 * pi / 20.0f;
Affine R;
bool rotating = true;


float frand(float a = 0, float b = 1) {
	return a + (b - a) * float(rand()) / float(RAND_MAX);
}


void Init(void) {
	srand(unsigned(time(0)));
	time_last = float(SDL_GetTicks() / 1000.0f);
	// camera stuff
	camera = Camera(O, -ez, ey, 0.5f * pi, float(width) / float(height), 0.1f, 10.0f);
	Affine ndc_to_device = Trans(Vector(0.5f * width, 0.5f * height, 0))
		* Scale(0.5f * (width - 1), 0.5f * (height - 1), 1);
	world_to_device = ndc_to_device * CameraToNDC(camera) * WorldToCamera(camera);
	// create memory for 2 frame/z-buffers
	frame_buffer0 = new Raster::byte[3 * width * height];
	frame_buffer = new Raster::byte[3 * width * height];
	zbuffer0 = new float[width * height];
	zbuffer = new float[width * height];
	raster = new Raster(frame_buffer, zbuffer, width, height, 3 * width);
	// initialize default frame buffer and z-buffer
	Vector n = S - C,
		u = cross(ey, n),
		v = cross(n, u);
	u.Normalize();
	v.Normalize();
	vector<Sphere> spheres;
	for (int i = 0; i < 5; ++i)
		for (int j = 0; j < 5; ++j) {
			Sphere s;
			s.center = 0.5f * (S + C) + (0.3f * i - 0.6f) * u + (0.3f * j - 0.6f) * v;
			s.radius = 0.15f;
			s.color = Vector(frand(200, 255), frand(200, 255), frand(200, 255));
			spheres.push_back(s);
		}
	FillRectZ(*raster, 0, 0, width, height, 10);
	raster->SetColor(0, 0, 0);
	FillRect(*raster, 0, 0, width, height);
	const Vector& dim = camera.ViewportGeometry();
	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < width; ++i) {
			Vector OP(0.5f * (float(i) / float(width) - 0.5f) * dim.x,
				0.5f * (float(j) / float(height) - 0.5f) * dim.y, -dim.z);
			float a = dot(OP, OP);
			for (int n = 0; n < spheres.size(); ++n) {
				const Point& C = spheres[n].center;
				const float& r = spheres[n].radius;
				const Vector& clr = spheres[n].color;
				Vector CO = O - C;
				float b = 2 * dot(OP, CO),
					c = dot(CO, CO) - r * r,
					disc = b * b - 4 * a * c;
				if (disc > 0) {
					float t = 0.5f * (-b - sqrt(disc)) / a;
					Point Q = O + t * OP;
					Hcoords v = world_to_device * Q;
					float z = v.z / v.w;
					raster->GotoPoint(i, j);
					if (z < raster->GetZ()) {
						Vector norm = Q - C;
						norm.Normalize();
						float mu = abs(norm.z);
						raster->SetColor(Raster::byte(mu * clr.x),
							Raster::byte(mu * clr.y),
							Raster::byte(mu * clr.z));
						raster->WritePixel();
						raster->WriteZ(z);
					}
				}
			}
		}
	}
	memcpy(frame_buffer0, frame_buffer, 3 * width * height);
	memcpy(zbuffer0, zbuffer, width * height * sizeof(float));

	// source/target parameters
	a0 = cross(ey, S - C);
	a0.Normalize();
	a0 = 2 * a0;
	b0 = cross(S - C, a0);
	b0.Normalize();
	b0 = 2 * b0;
	R = Scale(1);
}


void Draw(void) {
	// compute the time elapsed since the last call to 'Draw' (in seconds)
	double t = float(SDL_GetTicks() / 1000.0f);
	double dt = (t - time_last);
	time_last = t;

	// copy default frame buffer
	memcpy(frame_buffer, frame_buffer0, 3 * width * height);
	memcpy(zbuffer, zbuffer0, width * height * sizeof(float));

	// draw target rectangle and rays from source to target
	Hcoords hV0 = world_to_device * S;
	Point V0 = (1.0f / hV0.w) * hV0;
	if (rotating)
		R = Rot(rot_rate * dt, C - S) * R;
	Vector a = R * a0,
		b = R * b0;
	raster->SetColor(255, 0, 0);
	for (int i = 0; i <= 4; ++i) {
		float x = 0.5f * i - 1;
		Hcoords hV1 = world_to_device * (C + x * a - b),
			hV2 = world_to_device * (C + x * a + b),
			hV3 = world_to_device * (C - a + x * b),
			hV4 = world_to_device * (C + a + x * b);
		Point V1 = (1.0f / hV1.w) * hV1,
			V2 = (1.0f / hV2.w) * hV2,
			V3 = (1.0f / hV3.w) * hV3,
			V4 = (1.0f / hV4.w) * hV4;
		DrawLine(*raster, V1, V2);
		DrawLine(*raster, V3, V4);
		for (int j = 0; j <= 4; ++j) {
			float y = 0.5f * j - 1;
			Hcoords hV = world_to_device * (C + x * a + y * b);
			Point V = (1.0f / hV.w) * hV;
			DrawLine(*raster, V0, V);
		}
	}

	// display the frame buffer
	glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, frame_buffer);

}


void key_pressed(SDL_Keycode keycode) {
	switch (keycode) {
	case '\x1b':
		exit(0);
		should_close = true;
		break;
	case ' ':
		rotating = !rotating;
		break;
	}
}

void Resized(int W, int H) {
	glViewport(0, 0, W, H);
}


int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	SDL_GLContext context = SDL_GL_CreateContext(window);
	Resized(width, height);
	// GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		printf("ERROR: %s\n", glewGetErrorString(err));
	}

	Init();

	while (!should_close) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				should_close = true;
				break;
			case SDL_KEYDOWN:
				key_pressed(event.key.keysym.sym);
				break;
			}
		}
		Draw();
		SDL_GL_SwapWindow(window);
	}

	return 0;
}


