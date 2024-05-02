
#include <SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <random>


#include <algorithm>
#include "Projection.h"
#include "DT285_DrawingCam.h"
#include "FrustumMesh.h"
#include "CubeMesh.h"
#include "CowMesh.h"
#include "CloudMesh.h"
#include "GrassMesh.h"
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

Camera cam1, cam2,cam3,
cam2_0,cam3_0;

CowMesh* cow;
CowMesh* cow2;
vector<GrassMesh*> grassMeshes;
vector<CloudMesh*> cloudMeshes;
int cloudNum = 35;
CubeMesh cube;
Affine cube2world[9],
cow2world,cow22world;
Point cube_center(0, -1, -2);
vector<Affine> grass2world;
vector<Affine> cloud2world;
float cow_rot_rate = 2 * PI / 10.0f;
Vector cow_rot_axis(0, 1, 0);
Point cow_center(0, 0, -2);

float cow2_orbit_radius = 2.0f; 
float orbit_speed = PI / 5.0f;
bool use_cam1 = true, use_cam2 = false,
draw_solid = false;
bool should_close = false;
int currentCamera = 1;


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

    glEnable(GL_DEPTH_TEST);             
    glDepthFunc(GL_LESS);                

    time_last = float(SDL_GetTicks() / 1000.0f);
    float aspect = float(width) / float(height);
    cam1 = Camera(O + ez, -ez, ey, 0.5f * PI, aspect, 0.01f, 10);
    cam2_0 = Camera(O + 2 * ex - 2 * ez, -ex, ey, 0.5f * PI, aspect, 0.01f, 10);
    cam2 = cam2_0;
    cam3 = Camera(O + 2 * ex - 2 * ez, -ex, ey, 0.5f * PI, aspect, 0.01f, 10);


    cow = new CowMesh("Obj/cow.obj");
    float cow_scale = max(cow->Dimensions().x,
        max(cow->Dimensions().y, cow->Dimensions().z));
    cow2world = Trans(cow_center - O) 
        * Scale(2.0f / cow_scale)
        * Trans(O - cow->Center());

    cow2 = new CowMesh("Obj/cow.obj");
    float cow2_scale = cow_scale / 2.0f;
    Vector orbit_displacement(cow2_orbit_radius, 0, 0);  
    Point cow2_center = cow_center + orbit_displacement;  

  
    cow22world = Trans(cow_center - O)
        * Scale(1.0f / cow2_scale)  
        * Trans(O - cow->Center());

    for (int i = 0; i < 9; ++i) {
        GrassMesh* grass = new GrassMesh("Obj/grass.obj");
        grassMeshes.push_back(grass);
        float grass_scale = max(grass->Dimensions().x,
            max(grass->Dimensions().y, grass->Dimensions().z));
        grass2world.push_back(
            Trans(Point((i % 3) - 1, -0.9f, -(i / 3) - 1) - O)
            * Scale(0.75f / grass_scale)
            * Trans(O - grass->Center())
        );
    }


    // Determine bounds for grass positions
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    for (const auto& affine : grass2world) {
        Point pos = affine * O; // Assuming affine transformation contains translation to grass center
        minX = std::min(minX, pos.x - 2.0f);
        maxX = std::max(maxX, pos.x + 2.0f);
        minZ = std::min(minZ, pos.z - 2.0f);
        maxZ = std::max(maxZ, pos.z + 2.0f);
    }

    // Setup for random generation within the bounds
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> disX(minX, maxX);
    std::uniform_real_distribution<> disZ(minZ, maxZ);

    for (int i = 0; i < cloudNum; ++i) {
        CloudMesh* cloud = new CloudMesh("Obj/Cloud3.obj");
        cloudMeshes.push_back(cloud);
        float cloud_scale = max(cloud->Dimensions().x,
            max(cloud->Dimensions().y, cloud->Dimensions().z));

        float randomX = disX(gen);
        float randomZ = disZ(gen);
        cloud2world.push_back(
            Trans(Point(randomX, 0.8f, randomZ) - O)
            * Scale(0.75f / cloud_scale)
            * Trans(O - cloud->Center())
        );
    }

    
   /* for (int i = 0; i < 9; ++i) {
        CloudMesh* cloud = new CloudMesh("../Obj/Cloud3.obj");
        cloudMeshes.push_back(cloud);
        float cloud_scale = max(cloud->Dimensions().x,
            max(cloud->Dimensions().y, cloud->Dimensions().z));
        cloud2world.push_back(
            Trans(Point((i % 3) - 1, 0.8f, -(i / 3) - 1) - O)
            * Scale(0.75f / cloud_scale)
            * Trans(O - cloud->Center())
        );
    }*/


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
    double t = float(SDL_GetTicks() / 1000.0f);
    double dt = t - time_last;
    time_last = t;

    // Clear the screen
    glClearColor(0.796, 0.945, 1, 0); // Soft cyan background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers

    Camera& cam = (currentCamera == 1) ? cam1 : ((currentCamera == 2) ? cam2 : cam3);

    for (int i = 0; i < 9; ++i) {
        // Draw the grass mesh on top of the cube
        if (draw_solid)
        {
            DisplayFaces(cube, cube2world[i], cam, Vector(0.02, 0.42, 0.082)); // Green color
        }
        else
        {
           DisplayEdges(cube, cube2world[i], cam, Vector(0.02, 0.42, 0.082)); // Green color edges
        }
    }

    for (int i = 0; i < 9; i++)
    {
        if (draw_solid)
        {
            DisplayFaces(*grassMeshes[i], grass2world[i], cam, Vector(0.129, 0.612, 0.165)); // Lighter green for grass
        }
        else
        {
            DisplayEdges(*grassMeshes[i], grass2world[i], cam, Vector(0.129, 0.612, 0.165)); // Lighter green edges for grass
        }
    }

    for (int i = 0; i < cloudNum; i++)
    {
        if (draw_solid)
        {
            DisplayFaces(*cloudMeshes[i], cloud2world[i], cam, Vector(1, 0.984, 0.957)); // Lighter green for grass
        }
        else
        {
			DisplayEdges(*cloudMeshes[i], cloud2world[i], cam, Vector(1, 0.984, 0.957)); // Lighter green edges for grass
		}
	}

    // Draw the cow dodecahedron
    cow2world = Trans(cow_center - O)
        * Rot(cow_rot_rate * dt, cow_rot_axis)
        * Trans(O - cow_center)
        * cow2world;

    if (draw_solid)
        DisplayFaces(*cow, cow2world, cam, Vector(0.922, 0.035, 0.035)); // Use a different color for cow
    else
        DisplayEdges(*cow, cow2world, cam, Vector(0.922, 0.035, 0.035)); // Different color edges for cow


    float angle = orbit_speed * t;
    Vector orbit_vector(cos(angle) * cow2_orbit_radius, 0, sin(angle) * cow2_orbit_radius);
    Point cow2_new_position = cow_center + orbit_vector;
    

    // Update cow2 transformation matrix
    cow22world = Trans(cow2_new_position - O) * Scale(2.0f / cow2->Dimensions().x) * Trans(O - cow2->Center());

    // Set cow2 to face towards the center cow (optional, remove if not needed)
    float facing_angle = angle + PI; // To face the opposite direction
    float cow2_head = angle - PI/2.0f;
    cow22world = cow22world * RotY(facing_angle);
    Vector cow2_forward = Vector(sin(cow2_head), 0, cos(cow2_head));

    if(draw_solid)
		DisplayFaces(*cow2, cow22world, cam, Vector(0.922, 0.035, 0.035)); 
	else
		DisplayEdges(*cow2, cow22world, cam, Vector(0.922, 0.035, 0.035)); 

    cam2.LookInDirection(cow2_new_position - cam2.Eye());
    cam2.EyeMoveTo(cow_center);

    cam3.LookInDirection(cow2_forward);
    cam3.EyeMoveTo(cow2_new_position);

    // Draw the frustum of the other camera
    float aspect = float(width) / float(height);
    if (use_cam1) {
        const auto& D = cam2.ViewportGeometry().z;
        const auto& W = cam2.ViewportGeometry().x;
        const auto& H = cam2.ViewportGeometry().y;
        const auto& n = cam2.NearDistance();
        const auto& f = cam2.FarDistance();
        float fov = 2 * atan(0.5f * W / D);
        /*FrustumMesh frustum(fov, aspect, n, f);
        DisplayEdges(frustum, CameraToWorld(cam2), cam, Vector(0, 0, 0));*/
    }
    else {
        const auto& D = cam1.ViewportGeometry().z;
        const auto& W = cam1.ViewportGeometry().x;
        const auto& H = cam1.ViewportGeometry().y;
        const auto& n = cam1.NearDistance();
        const auto& f = cam1.FarDistance();
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
        currentCamera = (currentCamera % 3) + 1;
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
