// cs250_h2_driver.cpp
// -- simple test driver for HW #2
// cs250 5/15
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include <SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>

#include "Affine.h"
#include "DT285_Drawing.h"
#include "SnubDodecMesh.h"
using namespace std;


// global data (ugh)
bool should_close = false;

int width = 600, height = 600;
const char* name = "DT285 Sphere Solid";
double time_last;
double current_time;

const Point O(0, 0, 0);
const Vector ez(0, 0, 1);
const float PI = 4.0f * atan(1.0f);

SnubDodecMesh snub;
Point snub_center;
float snub_scale,
snub_rot_rate;
Vector snub_rot_axis;
Matrix pers;


void Init(void) {
    time_last = float(SDL_GetTicks() / 1000.0f);
    current_time = 0;

    snub_center = Point(0, 0, -1);
    snub_scale = 2.5f;
    snub_rot_rate = 2 * PI / 10.0f;
    snub_rot_axis = Vector(1, 1, 1);
    snub_rot_axis.Normalize();

    pers = PerspectiveProjection(3.0f);
}


void Draw(void) {
        // compute the time elapsed since the last call to 'Draw' (in seconds)
        double t = float(SDL_GetTicks() / 1000.0f);
        double dt = (t - time_last);
        time_last = t;

        // clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1.0f, 1.0f, 1.0f, 1);

        // modeling transformation
        Affine obj2world = Trans(snub_center - O)
            * Rot(acos(snub_rot_axis.z), cross(ez, snub_rot_axis))
            * Rot(snub_rot_rate * current_time, ez)
            * Scale(snub_scale / snub.Dimensions().x,
                snub_scale / snub.Dimensions().y,
                snub_scale / snub.Dimensions().z)
            * Trans(O - snub.Center());

        float cycleSpeed = 1.5f; // Adjust for faster or slower color changes
        float r = (sin(cycleSpeed * current_time) + 1.0f) / 2.0f; // Red component
        float g = (sin(cycleSpeed * current_time + 2.0f * PI / 3.0f) + 1.0f) / 2.0f; // Green component, phase shifted
        float b = (sin(cycleSpeed * current_time + 4.0f * PI / 3.0f) + 1.0f) / 2.0f; // Blue component, phase shifted
        Vector color(r, g, b);
        DisplayFaces(snub, obj2world, pers,color);
        current_time += dt;
}


void Resized(int W, int H) {
    int MIN = (W < H) ? W : H;
    glViewport(0, 0, MIN, MIN);
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
            }
        }
        Draw();
        SDL_GL_SwapWindow(window);
    }

    return 0;
}

