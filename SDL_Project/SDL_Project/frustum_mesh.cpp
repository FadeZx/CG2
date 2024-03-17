#include "DT285_Drawing.h"
#include "Camera.h"
#include "FrustumMesh.h"
#include "CubeMesh.h"
#include <iostream>

#include <SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <sstream>


using namespace std;

// global data (ugh)
int width = 600, height = 600;
const char* name = "DT285 - FrustumMesh";
double time_last;
int mouse_prev_x = 0, mouse_prev_y = 0, mouse_curr_x = 0, mouse_curr_y = 0;

const float PI = 4.0f * atan(1.0f);
const Point O(0, 0, 0);
const Vector ex(1, 0, 0), ey(0, 1, 0), ez(0, 0, 1);
const Vector BLUE(0, 0, 1), GREEN(0, 1, 0);

Camera camera;
Matrix PersProj;
bool draw_solid = false;
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
    camera = Camera(O - 5 * ez, -ez, ey, 0.5f * PI, 1.5f, 1, 3);
    PersProj = PerspectiveProjection(3.0f);

    // Load and compile shaders
    std::string vertexShaderCode = loadShaderSourceFromFile("vertex_shader.glsl");
    std::string fragmentShaderCode = loadShaderSourceFromFile("fragment_shader.glsl");

    const char* vertexShaderSource = vertexShaderCode.c_str();
    const char* fragmentShaderSource = fragmentShaderCode.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Check for shader compile errors...

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Check for shader compile errors...

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Check for linking errors...

    glUseProgram(shaderProgram);

    // Delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    InitBuffer(); // Initialize buffer for mesh data
}




void Draw(void) {
    // compute the time elapsed since the last call to 'Draw' (in seconds)
    double t = float(SDL_GetTicks() / 1000.0f);
    double dt = (t - time_last);
    time_last = t;

    // clear the screen
    glClearColor(1, 1, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw view frustum
    float fov = 2 * atan(0.5f * camera.ViewportGeometry().x / camera.ViewportGeometry().z),
        aspect = camera.ViewportGeometry().x / camera.ViewportGeometry().y;
    FrustumMesh frustum(fov, aspect, camera.NearDistance(), camera.FarDistance());
    Affine camera_to_world(camera.Right(), camera.Up(), camera.Back(), camera.Eye());

    if (draw_solid)
        DisplayFaces(frustum, camera_to_world, PersProj, GREEN);
    else
        DisplayEdges(frustum, camera_to_world, PersProj, BLUE);
}


void key_pressed(SDL_Keycode keycode) {
    if (keycode == SDLK_ESCAPE) {
        should_close = true;
    }
    const float angle_increment = PI / 180.0f,
        dist_increment = 0.1f,
        zoom_increment = 0.95f;
    switch (keycode) {
        case '\x1b':
            exit(0);
            break;
        case 'w':
            camera.Pitch(angle_increment);
            break;
        case 'x':
            camera.Pitch(-angle_increment);
            break;
        case 'a':
            camera.Yaw(angle_increment);
            break;
        case 'd':
            camera.Yaw(-angle_increment);
            break;
        case 's':
            camera.Roll(angle_increment);
            break;
        case 'j':
            camera.Roll(-angle_increment);
            break;
        case 'u':
            camera.Forward(dist_increment);
            break;
        case 'm':
            camera.Forward(-dist_increment);
            break;
        case '1':
            camera.Zoom(zoom_increment);
            break;
        case '2':
            camera.Zoom(1.0f / zoom_increment);
            break;
        case ' ':
            draw_solid = !draw_solid;
        }
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
