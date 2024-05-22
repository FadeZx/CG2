#include <vector>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <gl/glew.h>
#include <SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include "CubeMesh.h"
#include "SnubDodecMesh.h"
#include "Projection.h"
#include "HalfSpace.h"
using namespace std;


// global data (ugh)
int width = 600, height = 600;
const char* name = "DT285 Collider";
double time_last;
double current_time;
bool should_close = false;
const float PI = 4.0f * atan(1.0f);
const Point O(0, 0, 0);
const Vector ex(1, 0, 0),
ey(0, 1, 0),
ez(0, 0, 1);

struct Object {
    Mesh* mesh_ptr;
    Affine to_world;
    Vector color;
};

Camera camera;
CubeMesh cube;
Vector cube_rot_axis;
float cube_rot_rate;
SnubDodecMesh snub;
Vector snub_move_rate;
float snub_rot_rate;
vector<Object> objects;
vector<Point> temp_world_verts,
temp_cam_verts,
temp_ndc_verts;


float frand(float a = 0, float b = 1) {
    return a + (b - a) * float(rand()) / float(RAND_MAX);
}


void Init(void) {
    srand(unsigned(time(0)));
    glEnable(GL_DEPTH_TEST);
    current_time = 1;
    time_last = float(SDL_GetTicks() / 1000.0f);
    float aspect = float(width) / float(height);
    camera = Camera(O + 2 * ez, -ez, ey, 0.5f * PI, aspect, 0.1f, 10);
    cube_rot_axis = ey;
    cube_rot_rate = 2 * PI / 10.0f;
    snub_move_rate = 2 * PI * Vector(1 / frand(5, 15), 1 / frand(5, 15), 1 / frand(5, 15));
    snub_rot_rate = 2 * PI / frand(5, 15);
}


void Draw(void) {
    // compute the time elapsed since the last call to 'Draw' (in seconds)
    double t = float(SDL_GetTicks() / 1000.0f);
    double dt = (t - time_last);
    time_last = t;
    current_time += dt;

    glClearColor(1, 1, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearDepth(1);
    glClear(GL_DEPTH_BUFFER_BIT);
    objects.clear();

    // stationary rotating cube
    Object object;
    object.mesh_ptr = &cube;
    object.to_world = Rot(cube_rot_rate * current_time, cube_rot_axis)
        * Scale(0.4f / cube.Dimensions().x)
        * Trans(O - cube.Center());
    object.color = Vector(1, 0, 0);
    objects.push_back(object);

    // moving object
    Point P = O + sin(snub_move_rate.x * current_time) * ex
        + 0.5f * sin(snub_move_rate.y * current_time) * ey
        + sin(snub_move_rate.z * current_time) * ez;
    Vector vel = snub_move_rate.x * cos(snub_move_rate.x * current_time) * ex
        + 0.5f * snub_move_rate.y * cos(snub_move_rate.y * current_time) * ey
        + snub_move_rate.z * cos(snub_move_rate.z * current_time) * ez,
        axis = vel;
    axis.Normalize();
    object.mesh_ptr = &snub;
    object.to_world = Trans(P - O)
        * Rot(acos(axis.y), cross(ey, axis))
        //* Rot(snub_rot_rate*current_time,ey)
        * Scale(1 / snub.Dimensions().x,
            2 / snub.Dimensions().y,
            1 / snub.Dimensions().z)
        * Trans(O - snub.Center());
    // test if center of cube is inside moving object
    bool contains = true;
    temp_world_verts.clear();
    for (int i = 0; i < snub.VertexCount(); ++i)
        temp_world_verts.push_back(object.to_world * snub.GetVertex(i));
    for (int i = 0; contains && i < snub.FaceCount(); ++i) {
        const Mesh::Face& face = snub.GetFace(i);
        const Point& A = temp_world_verts[face.index1],
            B = temp_world_verts[face.index2],
            C = temp_world_verts[face.index3];
        Hcoords h = HalfSpace(A, B, C, P);
        contains = dot(h, O) <= 0;
    }
    object.color = contains ? Vector(1, 0, 1) : Vector(0, 0, 1);
    objects.push_back(object);


    // render objects
    Affine world_to_cam = WorldToCamera(camera);
    Matrix cam_to_ndc = CameraToNDC(camera);
    for (unsigned i = 0; i < objects.size(); ++i) {
        Mesh& mesh = *objects[i].mesh_ptr;
        Affine obj_to_cam = world_to_cam * objects[i].to_world;
        const Vector& color = objects[i].color;
        temp_cam_verts.clear();
        temp_ndc_verts.clear();
        for (int j = 0; j < mesh.VertexCount(); ++j) {
            temp_cam_verts.push_back(obj_to_cam * mesh.GetVertex(j));
            Hcoords v = cam_to_ndc * temp_cam_verts[j];
            temp_ndc_verts.push_back((1 / v.w) * v);
        }
        for (int j = 0; j < mesh.FaceCount(); ++j) {
            const Mesh::Face& face = mesh.GetFace(j);
            const Point& Pcam = temp_cam_verts[face.index1],
                Qcam = temp_cam_verts[face.index2],
                Rcam = temp_cam_verts[face.index3],
                Pndc = temp_ndc_verts[face.index1],
                Qndc = temp_ndc_verts[face.index2],
                Rndc = temp_ndc_verts[face.index3];
            Vector normal = cross(Qcam - Pcam, Rcam - Pcam);
            normal.Normalize();
            float mu = normal.z;
            glColor3f(mu * color[0], mu * color[1], mu * color[2]);
            glBegin(GL_TRIANGLES);
            glVertex3f(Pndc.x, Pndc.y, Pndc.z);
            glVertex3f(Qndc.x, Qndc.y, Qndc.z);
            glVertex3f(Rndc.x, Rndc.y, Rndc.z);
            glEnd();
        }
    }

}



void key_pressed(SDL_Keycode keycode) {
    if (keycode == SDLK_ESCAPE) {
        should_close = true;
    }
    switch (keycode) {
        case '\x1b':
            exit(0);
    }
}




int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext context = SDL_GL_CreateContext(window);
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


