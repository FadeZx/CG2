#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include "Affine.h"
using namespace std;


int width = 600, height = 600;
const char* name = "DT285 Rotating Cube";
double time_last;
const Point cube_verts[8] = { Point(0.5f, 0.5f,0.5f), Point(0.5f, 0.5f,-0.5f),
                              Point(0.5f,-0.5f,0.5f), Point(0.5f,-0.5f,-0.5f),
                              Point(-0.5f, 0.5f,0.5f), Point(-0.5f, 0.5f,-0.5f),
                              Point(-0.5f,-0.5f,0.5f), Point(-0.5f,-0.5f,-0.5f) };
const int cube_edges[12][2] = { {0,1}, {0,2}, {0,4}, {1,3},
                                {1,5}, {2,3}, {2,6}, {3,7},
                                {4,5}, {4,6}, {5,7}, {6,7} };
int cube_count;
Point center;
float m_size, scale, rot_rate;
Vector rot_axis;
float current_time;
Matrix Pers;
vector<Point> temp_verts(8);
bool should_close = false;

const float PI = 4.0f * atan(1.0f);
const Point O(0, 0, 0);
const Vector ez(0, 0, 1);
SDL_Window* window;



float frand(float a = 0, float b = 1) {
    return a + (b - a) * float(rand()) / float(RAND_MAX);
}


void Init(void) {
    srand(unsigned(time(0)));
    time_last = float(SDL_GetTicks() / 1000.0f);
    current_time = 0;
    // elementary perspective projection
    Pers.row[0] = Hcoords(1, 0, 0, 0);
    Pers.row[1] = Hcoords(0, 1, 0, 0);
    Pers.row[2] = Hcoords(0, 0, 1, 0);
    Pers.row[3] = Hcoords(0, 0, -1, 0);
    // rotating figure parameters
    center = Point(0, 0, -1.25f);
    cube_count = 3;
    m_size = 1.0f / cube_count;
    scale = 0.75f;
    rot_rate = 2.0f * PI / 10.0f;
    rot_axis = Vector(frand(), frand(), frand());
    rot_axis.Normalize();
}

void AddLineSegment(const Point& start, const Point& end, const Point& color,
    vector<float>& vertices, vector<float>& colors, vector<int>& indices) {
    int startIndex = vertices.size() / 3; // 3 components (x, y, z) per vertex

    // Add start and end points to the vertices list
    vertices.push_back(start.x);
    vertices.push_back(start.y);
    vertices.push_back(start.z);
    vertices.push_back(end.x);
    vertices.push_back(end.y);
    vertices.push_back(end.z);

    // Add color for both the start and end vertices
    for (int i = 0; i < 2; ++i) { // Repeat twice, once for start, once for end
        colors.push_back(color.x);
        colors.push_back(color.y);
        colors.push_back(color.z);
    }

    // Add indices for this line segment
    indices.push_back(startIndex);
    indices.push_back(startIndex + 1);
}



void Draw(void) {
    // compute the time elapsed since the last call to 'Draw' (in seconds)
    float t = float(SDL_GetTicks() / 1000.0f);
    float dt = t - time_last;
    time_last = t;

    // clear the screen
    glClearColor(1, 1, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    vector<float> cube_vertices;
    vector<float> cube_colors;
    vector<int> cube_indices;

    // draw an array of rotating cubes
    Affine R = Rot(acos(dot(ez, rot_axis)), cross(ez, rot_axis))
        * Rot(rot_rate * current_time, ez);

    cube_vertices.clear();
    cube_colors.clear();
    cube_indices.clear();

    for (int i = 0; i < cube_count; ++i) {
        for (int j = 0; j < cube_count; ++j) {
            for (int k = 0; k < cube_count; ++k) {
                Point color(i * m_size, j * m_size, k * m_size);
                Vector offset = m_size * (Vector(i, j, k) - 0.5f * (cube_count - 1) * Vector(1, 1, 1));
                Affine obj2world = Trans(center - O) * R * Trans(offset)
                    * Scale(scale * m_size);
                Matrix obj2dev = Pers * obj2world;

                for (int n = 0; n < 8; ++n) {
                    Hcoords v = obj2dev * cube_verts[n];
                    temp_verts[n] = (1.0f / v.w) * v;
                }
                
                for (int n = 0; n < 12; ++n) {
                    const Point& P = temp_verts[cube_edges[n][0]],
                        Q = temp_verts[cube_edges[n][1]];
                    Point color = Point(i * m_size, j * m_size, k * m_size);
                    AddLineSegment(P, Q, color, cube_vertices, cube_colors, cube_indices);
                }
            }
        }
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, cube_vertices.data());
    glColorPointer(3, GL_FLOAT, 0, cube_colors.data());

    glDrawElements(GL_LINES, cube_indices.size(), GL_UNSIGNED_INT, cube_indices.data());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    current_time += dt;
}


void key_pressed(SDL_Keycode keycode) {

    if (keycode == SDLK_ESCAPE) {
        should_close = true;
    }
    else if (keycode == SDLK_SPACE) {
        cube_count = 1 + cube_count % 6;
        m_size = 1.0f / cube_count;
    }

}


int main(int argc, char* argv[]) {

    // SDL: initialize and create a window
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    Init();
    //// animation loop
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

    // clean up
    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}
