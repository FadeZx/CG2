// cs250project1.empty.cpp
// -- template for first draft of the final project
// cs250 5/15

#include <sstream>
#include <cstdlib>
#include <SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <iostream>

using namespace std;


int width = 600, height = 600;
const char* name = "Final Project First Draft";
double time_last;
double current_time;
int frame_count;
double frame_time;
bool should_close = false;
SDL_Window* window;


void Resized(int W, int H) {
	width = W;
	height = H;
	glViewport(0, 0, W, H);
}


void Init(void) {
	glEnable(GL_DEPTH_TEST);
	time_last = float(SDL_GetTicks() / 1000.0f);
	frame_count = 0;
	frame_time = 0;
	current_time = 0;
	Resized(width, height);
}


void Draw(void) {
	float t = float(SDL_GetTicks() / 1000.0f);
	float dt = t - time_last;
	time_last = t;

	// frame rate
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

	// clear the screen
	glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	// clear the z-buffer
	glClearDepth(1);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void key_pressed(SDL_Keycode keycode) {
	if (keycode == SDLK_ESCAPE) {
		should_close = true;
	}
	switch (keycode) {
	case '\x1b':
		exit(0);
		break;
	}
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
			}
		}
		Draw();
		SDL_GL_SwapWindow(window);
	}

	return 0;
}




