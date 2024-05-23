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
#include "HalfSpace.h"
#include "Clip.h"

using namespace std;

int width = 600, height = 600;
const char* name = "Final Project First Draft";
double time_last;
double current_time;
int frame_count;
double frame_time;
bool should_close = false;
SDL_Window* window;

const Point O(0, 0, 0);
const Vector ex(1, 0, 0),
ey(0, 1, 0),
ez(0, 0, 1);
const float PI = 4.0f * atan(1.0f);

struct Object {
    Mesh* mesh_ptr;
    Affine to_world;
    Vector color;
};

Camera cam1, cam2, cam3, cam2_0, cam3_0;
vector<Object> cows;
vector<Point> temp_world_verts, temp_cam_verts, temp_ndc_verts;

CowMesh* cow;
CowMesh* cow2;
Object cow2_obj;
vector<GrassMesh*> grassMeshes;
vector<CloudMesh*> cloudMeshes;
int cloudNum = 35;
CubeMesh cube;
Affine cube2world[9], cow2world, cow22world;
vector<Affine> grass2world;
vector<Affine> cloud2world;
float cow_rot_rate;
Vector cow_rot_axis;
Point cow_center(0, 0, -2);
float cow2_rot_rate;
Vector cow2_move_rate;
bool use_cam1 = true, use_cam2 = false, draw_solid = false;
int currentCamera = 1;


struct AABB {
    Point min;
    Point max;
};


AABB ComputeAABB(const Object& obj) {
    AABB aabb;
    aabb.min = Point(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    aabb.max = Point(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

    for (int i = 0; i < obj.mesh_ptr->VertexCount(); ++i) {
        Point p = obj.to_world * obj.mesh_ptr->GetVertex(i);
        aabb.min.x = std::min(aabb.min.x, p.x);
        aabb.min.y = std::min(aabb.min.y, p.y);
        aabb.min.z = std::min(aabb.min.z, p.z);
        aabb.max.x = std::max(aabb.max.x, p.x);
        aabb.max.y = std::max(aabb.max.y, p.y);
        aabb.max.z = std::max(aabb.max.z, p.z);
    }

    return aabb;
}

bool AABBIntersect(const AABB& a, const AABB& b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
        (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
        (a.min.z <= b.max.z && a.max.z >= b.min.z);
}


std::string loadShaderSourceFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

float frand(float a = 0, float b = 1) {
    return a + (b - a) * float(rand()) / float(RAND_MAX);
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

    // Initialize first rotating cow
    cow = new CowMesh("Obj/cow.obj");
    float cow_scale = max(cow->Dimensions().x,
        max(cow->Dimensions().y, cow->Dimensions().z));
    cow2world = Trans(cow_center - O)
        * Scale(2.0f / cow_scale)
        * Trans(O - cow->Center());

    cow_rot_axis = ey;
    cow_rot_rate = 2 * PI / 10.0f;

    // Initialize moving cow2
    cow2 = new CowMesh("Obj/cow.obj");
    float cow2_scale = cow_scale / 2.0f;
    cow2_rot_rate = 2 * PI / frand(5, 15);
    cow2_move_rate = 2 * PI * Vector(1 / frand(5, 15), 1 / frand(5, 15), 1 / frand(5, 15));

    cow2_obj.mesh_ptr = cow2;
    cow2_obj.to_world = Trans(cow_center - O)
        * Scale(1.0f / cow2_scale)
        * Trans(O - cow2->Center());
    cow2_obj.color = Vector(0.922f, 0.035f, 0.035f); // Initial color

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
        Point pos = affine * O;
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
    float current_time = float(SDL_GetTicks() / 1000.0f);  // Update current_time
    float dt = current_time - time_last;
    time_last = current_time;

    // Frame rate
    ++frame_count;
    frame_time += dt;
    if (frame_time >= 0.5) {
        double fps = frame_count / frame_time;
        frame_count = 0;
        frame_time = 0;
        stringstream ss;
        ss << name << " [fps=" << int(fps) << "]";
        SDL_SetWindowTitle(window, ss.str().c_str());
    }

    // Clear the screen
    glClearColor(0.796f, 0.945f, 1.0f, 0.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Camera& cam = (currentCamera == 1) ? cam1 : ((currentCamera == 2) ? cam2 : cam3);

    // Draw the grass mesh on top of the cube
    for (int i = 0; i < 9; ++i) {
        if (draw_solid) {
            DisplayFaces(cube, cube2world[i], cam, Vector(0.02f, 0.42f, 0.082f));
        }
        else {
            DisplayEdges(cube, cube2world[i], cam, Vector(0.02f, 0.42f, 0.082f));
        }
    }

    for (int i = 0; i < 9; ++i) {
        if (draw_solid) {
            DisplayFaces(*grassMeshes[i], grass2world[i], cam, Vector(0.129f, 0.612f, 0.165f));
        }
        else {
            DisplayEdges(*grassMeshes[i], grass2world[i], cam, Vector(0.129f, 0.612f, 0.165f)); 
        }
    }

    for (int i = 0; i < cloudNum; ++i) {
        if (draw_solid) {
            DisplayFaces(*cloudMeshes[i], cloud2world[i], cam, Vector(1.0f, 0.984f, 0.957f)); 
        }
        else {
            DisplayEdges(*cloudMeshes[i], cloud2world[i], cam, Vector(1.0f, 0.984f, 0.957f));
        }
    }

    // Draw the first cow (rotating cow)
    cow2world = Trans(cow_center - O)
        * Rot(cow_rot_rate * dt, cow_rot_axis)
        * Trans(O - cow_center)
        * cow2world;

    if (draw_solid) {
        DisplayFaces(*cow, cow2world, cam, Vector(0.922f, 0.035f, 0.035f));
    }
    else {
        DisplayEdges(*cow, cow2world, cam, Vector(0.922f, 0.035f, 0.035f));
    }

    // Update and draw the moving cow2
    Point P = O + 0.5f * sin(cow2_move_rate.x * current_time) * ex 
        + 0.25f * sin(cow2_move_rate.y * current_time) * ey  
        + 3.0f * sin(cow2_move_rate.z * current_time) * ez;  
    Vector vel = cow2_move_rate.x * cos(cow2_move_rate.x * current_time) * ex
        + 0.5f * cow2_move_rate.y * cos(cow2_move_rate.y * current_time) * ey
        + 1.0f * cow2_move_rate.z * cos(cow2_move_rate.z * current_time) * ez;
    vel.Normalize();

    cow2_obj.to_world = Trans(P - O)
        * Rot(acos(vel.y), cross(ey, vel))
        * Scale(2.0f / cow2->Dimensions().x)
        * Trans(O - cow2->Center());

    //// Collision detection and color change
    //bool contains = true;
    //temp_world_verts.clear();
    //for (int i = 0; i < cow2->VertexCount(); ++i)
    //    temp_world_verts.push_back(cow2_obj.to_world * cow2->GetVertex(i));

    //for (int i = 0; contains && i < cow2->FaceCount(); ++i) {
    //    const Mesh::Face& face = cow2->GetFace(i);
    //    const Point& A = temp_world_verts[face.index1],
    //        B = temp_world_verts[face.index2],
    //        C = temp_world_verts[face.index3];
    //    Hcoords h = HalfSpace(A, B, C, P);
    //    contains = dot(h, O) <= 0;
    //}
    //cow2_obj.color = contains ? Vector(1.0f, 0.0f, 1.0f) : Vector(0.922f, 0.035f, 0.035f);
    //std::cout << "Collision detected: " << contains << std::endl;
    
    // Compute AABBs for cow1 and cow2
    Object cow1_obj = { cow, cow2world, Vector(0.922f, 0.035f, 0.035f) };
    AABB aabb1 = ComputeAABB(cow1_obj);
    AABB aabb2 = ComputeAABB(cow2_obj);

    bool collision = AABBIntersect(aabb1, aabb2);
    cow2_obj.color = collision ? Vector(0.451f, 0.224f, 0.031f) : Vector(0.922f, 0.035f, 0.035f);
    std::cout << "Collision detected: " << collision << std::endl;

    if (draw_solid) {
        DisplayFaces(*cow2, cow2_obj.to_world, cam, cow2_obj.color); 
    }
    else {
        DisplayEdges(*cow2, cow2_obj.to_world, cam, cow2_obj.color);
    }

    // Update camera2 to look at cow2
    cam2.LookInDirection(P - cam2.Eye());
    cam2.EyeMoveTo(cow_center);

    // Update camera3 to look forward from cow2's position
    cam3.LookInDirection(vel);
    cam3.EyeMoveTo(P);

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
    window = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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
