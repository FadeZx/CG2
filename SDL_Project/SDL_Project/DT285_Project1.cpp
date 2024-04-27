
#include <SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>

#include <algorithm>
#include "Projection.h"
#include "DT285_DrawingCam.h"
#include "FrustumMesh.h"
#include "CubeMesh.h"
#include "CowMesh.h"
#include <iostream>
#include <fstream>
#include <sstream>


using namespace std;


// global data (ugh)
int width = 500, height = 500;
const char* name = "DT285 - Projection";
float time_last;
int mouse_prev_x = 0, mouse_prev_y = 0,
mouse_curr_x = 0, mouse_curr_y = 0;

const Point O(0, 0, 0);
const Vector ex(1, 0, 0),
ey(0, 1, 0),
ez(0, 0, 1);
const float PI = 4.0f * atan(1.0f);

Camera cam1, cam2,
cam2_0;
CowMesh* cow;
CubeMesh cube;
Affine cube2world[9],
cow2world;
float cow_rot_rate = 2 * PI / 10.0f;
Vector cow_rot_axis(0, 1, 0);
Point cow_center(0, 0, -2);
bool use_cam1 = true,
draw_solid = false;
bool should_close = false;


std::string loadShaderSourceFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void Init(void) {

    time_last = float(SDL_GetTicks() / 1000.0f);
    float aspect = float(width) / float(height);
    cam1 = Camera(O + ez, -ez, ey, 0.5f * PI, aspect, 0.01f, 1);
    cam2_0 = Camera(O + 2 * ex - 2 * ez, -ex, ey, 0.5f * PI, aspect, 0.01f, 1);
    cam2 = cam2_0;
    cow = new CowMesh("../Obj/Statue.obj");
    float cow_scale = max(cow->Dimensions().x,
        max(cow->Dimensions().y, cow->Dimensions().z));
    cow2world = Trans(cow_center - O)
        * Scale(2.0f / cow_scale)
        * Trans(O - cow->Center());
    for (int i = 0; i < 9; ++i)
        cube2world[i] = Trans(Point((i % 3) - 1, -1.1f, -(i / 3) - 1) - O)
        * Scale(0.75f / cube.Dimensions().x,
            0.1f / cube.Dimensions().y,
            0.75f / cube.Dimensions().z)
        * Trans(O - cube.Center());


    // Load and compile shaders
    std::string vertexShaderCode = loadShaderSourceFromFile("vertex_shader.glsl");
    std::string fragmentShaderCode = loadShaderSourceFromFile("fragment_shader.glsl");

    const char* vertexShaderSource = vertexShaderCode.c_str();
    const char* fragmentShaderSource = fragmentShaderCode.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glUseProgram(shaderProgram);

    // Delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    InitBuffer();
}


void Draw(void) {

    // compute the time elapsed since the last call to 'Draw' (in seconds)
    double t = float(SDL_GetTicks() / 1000.0f);
    double dt = t - time_last;
    time_last = t;

    // clear the screen
    glClearColor(1, 1, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    Camera& cam = use_cam1 ? cam1 : cam2;

    // Draw cubes
    for (int i = 0; i < 9; ++i)
        if (draw_solid)
            DisplayFaces(cube, cube2world[i], cam, Vector(1, 0, 1));
        else
            DisplayEdges(cube, cube2world[i], cam, Vector(1, 0, 1));

    // Draw cow dodecahedron
    cow2world = Trans(cow_center - O)
        * Rot(cow_rot_rate * dt, cow_rot_axis)
        * Trans(O - cow_center)
        * cow2world;
    if (draw_solid)
        DisplayFaces(*cow, cow2world, cam, Vector(0, 1, 0));
    else
        DisplayEdges(*cow, cow2world, cam, Vector(0, 1, 0));

    // Draw (other) camera mesh
    float aspect = float(width) / float(height);
    if (use_cam1) {
        const float& D = cam2.ViewportGeometry().z,
            W = cam2.ViewportGeometry().x,
            H = cam2.ViewportGeometry().y,
            n = cam2.NearDistance(),
            f = cam2.FarDistance();
        float fov = 2 * atan(0.5f * W / D);
        FrustumMesh frustum(fov, aspect, n, f);
        DisplayEdges(frustum, CameraToWorld(cam2), cam, Vector(0, 0, 0));
    }
    else {
        const float& D = cam1.ViewportGeometry().z,
            W = cam1.ViewportGeometry().x,
            H = cam1.ViewportGeometry().y,
            n = cam1.NearDistance(),
            f = cam1.FarDistance();
        float fov = 2 * atan(0.5f * W / D);
        FrustumMesh frustum(fov, aspect, n, f);
        DisplayEdges(frustum, CameraToWorld(cam1), cam, Vector(1, 0, 0));
    }
}


void key_pressed(SDL_Keycode keycode) {
    const float angle_increment = PI / 180.0f,
        dist_increment = 0.1f,
        zoom_increment = 0.95f;
    switch (keycode) {
    case '\x1b':
        exit(0);
        should_close = true;
        break;
    case 'w':
        cam2.Pitch(angle_increment);
        break;
    case 'x':
        cam2.Pitch(-angle_increment);
        break;
    case 'a':
        cam2.Yaw(angle_increment);
        break;
    case 'd':
        cam2.Yaw(-angle_increment);
        break;
    case 's':
        cam2.Roll(angle_increment);
        break;
    case 'j':
        cam2.Roll(-angle_increment);
        break;
    case 'u':
        cam2.Forward(dist_increment);
        break;
    case 'm':
        cam2.Forward(-dist_increment);
        break;
    case ' ':
        draw_solid = !draw_solid;
        break;
    case '1':
        cam2.Zoom(zoom_increment);
        break;
    case '2':
        cam2.Zoom(1.0f / zoom_increment);
        break;
    case '3':
        use_cam1 = !use_cam1;
        break;
    case '4':
        cam2 = cam2_0;
    }
}


void Resized(int W, int H) {
    width = W;
    height = H;
    glViewport(0, 0, width, height);
    float aspect = float(width) / float(height);
    cam1 = Camera(O + ez, -ez, ey, 0.5f * PI, aspect, 0.01f, 1);
    cam2_0 = Camera(O + 2 * ex - 2 * ez, -ex, ey, 0.5f * PI, aspect, 0.01f, 1);
    cam2 = cam2_0;
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
