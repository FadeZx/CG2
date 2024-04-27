
#include <vector>
#include <cstdlib>
#include <cmath>
#include <gl/glew.h>
#include <SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <iostream>
#include "CubeMesh.h"
#include "RasterUtilities.h"
#include "Projection.h"

using namespace std;


// global data (ugh)
int width = 600, height = 600;
const char* name = "DT285 Z-Depth Triangle";
double time_last;
bool should_close = false;


const float PI = 4.0f * atan(1.0f);
const Point O(0, 0, 0);
const Vector ey(0, 1, 0),
ez(0, 0, 1);
Camera cam;
Affine ndc2device;
CubeMesh cube;
vector<Point> temp_device_verts,
temp_world_verts;
Vector obj_color[3]
= { Vector(255,100,100), Vector(100,100,255), Vector(0,255,0) };
Raster* raster;
Raster::byte* frame_buffer;
double current_time;


void Init(void) {
    time_last = float(SDL_GetTicks() / 1000.0f);
    float aspect = float(width) / float(height);
    cam = Camera(Point(0, 0, 1), Vector(0, 0, -1), Vector(0, 1, 0), 0.5f * PI, aspect, 0.1f, 10);
    ndc2device = Trans(Vector(0.5f * (width - 1), 0.5f * (height - 1), 0))
        * Scale(0.5f * (width - 1), 0.5f * (height - 1), 1);
    int frame_buffer_sz = 3 * width * height * sizeof(Raster::byte),
        zbuffer_sz = width * height * sizeof(float);
    char* block = new char[frame_buffer_sz + zbuffer_sz];
    frame_buffer = reinterpret_cast<Raster::byte*>(block);
    raster = new Raster(frame_buffer,
        reinterpret_cast<float*>(block + frame_buffer_sz),
        width, height, 3 * width);
    current_time = 0;
}


void Draw(void) {
    // compute the time elapsed since the last call to 'Draw' (in seconds)
    double t = float(SDL_GetTicks() / 1000.0f);
    double dt = t - time_last;
    time_last = t;

    // clear screen and z-buffer
    raster->SetColor(255, 255, 255);
    FillRect(*raster, 0, 0, width, height);
    FillRectZ(*raster, 0, 0, width, height);

    // compute modeling transformations for each cube
    Affine obj2world[3];
    Vector u(0, 1, 1);
    obj2world[0] = Trans(Point(-1, 0, -3) - O)
        * Rot(acos(u.z / abs(u)), cross(ez, u))
        * Rot(2 * PI * current_time / 10.0f, ez)
        * Scale(2.5 / cube.Dimensions().x,
            2.5 / cube.Dimensions().y,
            0.15f / cube.Dimensions().z)
        * Trans(O - cube.Center());
    u = Vector(0, 1, -1);
    obj2world[1] = Trans(Point(1, 0, -3) - O)
        * Rot(acos(u.z / abs(u)), cross(ez, u))
        * Rot(2 * PI * current_time / 8.0f, ez)
        * Scale(2.5 / cube.Dimensions().x,
            2.5 / cube.Dimensions().y,
            0.15f / cube.Dimensions().z)
        * Trans(O - cube.Center());
    obj2world[2] = Trans(Point(0, 0, 2 * sin(2 * PI * current_time / 15.0f) - 3) - O)
        * Rot(2 * PI * current_time / 7.1f, ey)
        //* Rot(2*PI*current_time/1.2f,ez)
        * Scale(2 / cube.Dimensions().x,
            2 / cube.Dimensions().y,
            0.1f / cube.Dimensions().z)
        * Trans(O - cube.Center());
    // draw cubes
    for (int n = 0; n < 3; ++n) {
        Matrix obj2device = ndc2device
            * CameraToNDC(cam)
            * WorldToCamera(cam)
            * obj2world[n];
        temp_world_verts.clear();
        temp_device_verts.clear();
        for (int i = 0; i < cube.VertexCount(); ++i) {
            temp_world_verts.push_back(obj2world[n] * cube.GetVertex(i));
            Hcoords v = obj2device * cube.GetVertex(i);
            temp_device_verts.push_back((1.0f / v.w) * v);
        }
        for (int i = 0; i < cube.FaceCount(); ++i) {
            Mesh::Face f = cube.GetFace(i);
            const Point& Pworld = temp_world_verts[f.index1],
                Qworld = temp_world_verts[f.index2],
                Rworld = temp_world_verts[f.index3];
            Vector normal = cross(Qworld - Pworld, Rworld - Pworld),
                light(0, 0, 1);
            float mu = abs(dot(normal, light)) / (abs(normal) * abs(light));
            raster->SetColor(Raster::byte(mu * obj_color[n].x),
                Raster::byte(mu * obj_color[n].y),
                Raster::byte(mu * obj_color[n].z));
            const Point& Pdevice = temp_device_verts[f.index1],
                Qdevice = temp_device_verts[f.index2],
                Rdevice = temp_device_verts[f.index3];
            FillTriangle(*raster, Pdevice, Qdevice, Rdevice);
        }
    }

    glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, frame_buffer);
    current_time += dt;
}


void key_pressed(SDL_Keycode keycode) {
    switch (keycode) {
    case '\x1b':
        exit(0);
        should_close = true;
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

