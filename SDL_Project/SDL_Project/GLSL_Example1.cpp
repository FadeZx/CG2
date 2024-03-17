#include <iostream>
#include <SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>


using namespace std;


GLuint program_id;
GLuint vertex_shader_id, fragment_shader_id, p;
GLuint VBO, VAO;

// global data (ugh)
int width = 600, height = 600;
const char* name = "GLSL Test";

bool should_close = false;

float points[9] = { -0.8f, -0.5f, 0.0f, 0.0, 0.9, 0.0f, 0.5f, -0.7f, 0.0f };
float colors[9] = {
        1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f
};

GLuint indices[3] = { 0, 1, 2 };


const char* vertex_shader_str = "#version 330 core\n"
"layout (location = 0) in vec3 a_pos;\n"
"void main() {\n"
"  gl_Position = vec4(a_pos, 1.0);\n"
"}\n";

const char* fragment_shader_str = "#version 330 core\n"
"out vec4 frag_color;\n"
"uniform vec4 c_pos;\n"
"void main() {\n"
"  frag_color = c_pos;\n"
"}\n";


void Init(void) {

    program_id = glCreateProgram();

    vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertex_shader_id, 1, &vertex_shader_str, NULL);
    glShaderSource(fragment_shader_id, 1, &fragment_shader_str, NULL);

    glCompileShader(vertex_shader_id);
    glCompileShader(fragment_shader_id);

    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);
    glUseProgram(program_id);



    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}


void Draw(void) {
    double t = float(SDL_GetTicks() / 1000.0f);
    // clear the screen
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    int vertexColorLocation = glGetUniformLocation(program_id, "c_pos");
    glUniform4f(vertexColorLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}


void key_pressed(SDL_Keycode keycode) {
    if (keycode == SDLK_ESCAPE) {
        should_close = true;
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
