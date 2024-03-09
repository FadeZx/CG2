// -- display a rotating cube (uses CubeMesh)
#include <iostream>

#include "DT285_Drawing.h"
#include "CubeMesh.h"
#include "SnubDodecMesh.h"

#include <SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>

using namespace std;


// global data (ugh)
int width = 600, height = 600;
const char *name = "DT285 - Cube Test";
float time_last;
GLuint programID;

int mouse_prev_x=0, mouse_prev_y=0,
    mouse_curr_x=0, mouse_curr_y=0;

const float PI = 4.0f*atan(1.0f);
const Point O(0,0,0);
const Vector ez(0,0,1);
const Vector BLUE(0,0,1),
             GREEN(0,1,0);

CubeMesh cube;
Point cube_center;
float cube_scale,
      cube_rot_rate;
Vector cube_rot_axis(1,1,1);
bool draw_solid = false;
Affine ObjToWorld;
Matrix PersProj;
bool should_close = false;


void Init(void) {
  time_last = float(SDL_GetTicks() / 1000.0f);
  cube_center = Point(0,0,-1);
  cube_scale = 1.4f;
  cube_rot_rate = 2*PI/10.0f;
  cube_rot_axis = Vector(1,1,1);
  cube_rot_axis.Normalize();

  ObjToWorld = Trans(cube_center-O)
               * Rot(acos(dot(ez,cube_rot_axis)),cross(ez,cube_rot_axis))
               * Scale(cube_scale/cube.Dimensions().x,
                       cube_scale/cube.Dimensions().y,
                       cube_scale/cube.Dimensions().z)
               * Trans(O-cube.Center());
  PersProj = PerspectiveProjection(3.0f);
}


void Draw(void) {
  // compute the time elapsed since the last call to 'Draw' (in seconds)
    double t = float(SDL_GetTicks() / 1000.0f);
    double dt = (t-time_last);
    time_last = t;

    // clear the screen
    glClearColor(1,1,1,0);
    glClear(GL_COLOR_BUFFER_BIT);

    // rotate cube
    float cube_rot_angle = cube_rot_rate * dt;
    ObjToWorld = Trans(cube_center-O)
                * Rot(cube_rot_angle,cube_rot_axis)
                * Trans(O-cube_center)
                * ObjToWorld;

    // draw rotating cube
	if (draw_solid)
	{
		DisplayFaces(cube, ObjToWorld, PersProj, GREEN);
	}
	else
	{
		DisplayEdges(cube, ObjToWorld, PersProj, BLUE);
	}
}



void Resized(int W, int H) {
  int MIN = (W < H) ? W : H;
  glViewport(0,0,MIN,MIN);
}


void key_pressed(SDL_Keycode keycode) {
    if (keycode == SDLK_ESCAPE) {
        should_close = true;
    }
    switch (keycode) {
    case ' ':
        draw_solid = !draw_solid;
        break;
    case '\x1b':
        exit(0);
    }
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
