/*
 * TNFS 1995 car physics code
 * Recreation and analysis of the physics engine from the classic racing game
 */
#include <stdio.h>
#include <math.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "tnfs_math.h"
#include "tnfs_base.h"

static SDL_Event event;

GLfloat matrix[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
tnfs_vec3 camera_position = { 0, -5, -10 };

void handleKeys() {
	if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
		switch (event.key.keysym.sym) {
		case SDLK_LEFT:
			car_data.steer_angle = -0x1B0000;
			break;
		case SDLK_RIGHT:
			car_data.steer_angle = 0x1B0000;
			break;
		case SDLK_UP:
			car_data.throttle = 0xFF;
			break;
		case SDLK_DOWN:
			car_data.brake = 100;
			break;
		case SDLK_SPACE:
			car_data.handbrake = 1;
			break;
		case SDLK_a:
			car_data.gear_RND = 3; //Drive
			car_data.gear_speed = 0;
			break;
		case SDLK_z:
			car_data.gear_RND = 1; //Reverse
			car_data.gear_speed = -2;
			break;
		case SDLK_r:
			tnfs_reset();
			break;
		case SDLK_c:
			cheat_mode = 4;
			tnfs_cheat_crash_cars();
			break;
		case SDLK_F4:
			cheat_mode = 4;
			break;
		default:
			break;
		}
	}
	if (event.type == SDL_KEYUP && event.key.repeat == 0) {
		switch (event.key.keysym.sym) {
		case SDLK_UP:
			car_data.throttle = 0;
			break;
		case SDLK_DOWN:
			car_data.brake = 0;
			break;
		case SDLK_LEFT:
			car_data.steer_angle = 0;
			break;
		case SDLK_RIGHT:
			car_data.steer_angle = 0;
			break;
		case SDLK_SPACE:
			car_data.handbrake = 0;
			break;
		default:
			break;
		}
	}
}


void renderGl() {

	glMatrixMode( GL_MODELVIEW);

	matrix[0] = (float) car_data.matrix.ax / 0x10000;
	matrix[1] = (float) car_data.matrix.ay / 0x10000;
	matrix[2] = (float) car_data.matrix.az / 0x10000;
	matrix[3] = 0;
	matrix[4] = (float) car_data.matrix.bx / 0x10000;
	matrix[5] = (float) car_data.matrix.by / 0x10000;
	matrix[6] = (float) car_data.matrix.bz / 0x10000;
	matrix[7] = 0;
	matrix[8] = (float) car_data.matrix.cx / 0x10000;
	matrix[9] = (float) car_data.matrix.cy / 0x10000;
	matrix[10] = (float) car_data.matrix.cz / 0x10000;
	matrix[11] = 0;
	matrix[12] = ((float) car_data.position.x) / 0x10000 + camera_position.x;
	matrix[13] = ((float) car_data.position.y) / 0x10000 + camera_position.y;
	matrix[14] = -(((float) car_data.position.z) / 0x10000) + camera_position.z;
	matrix[15] = 1;
	glLoadMatrixf(&matrix);

	glBegin(GL_QUADS);
	glVertex3f(-1, -0.5f, -2);
	glVertex3f(1, -0.5f, -2);
	glVertex3f(1, 0.5f, -2);
	glVertex3f(-1, 0.5f, -2);

	glVertex3f(-1, -0.5f, 2);
	glVertex3f(1, -0.5f, 2);
	glVertex3f(1, 0.5f, 2);
	glVertex3f(-1, 0.5f, 2);

	glVertex3f(-1, 0.5f, -2);
	glVertex3f(1, 0.5f, -2);
	glVertex3f(1, 0.5f, 2);
	glVertex3f(-1, 0.5f, 2);

	glVertex3f(-1, -0.5f, 2);
	glVertex3f(1, -0.5f, 2);
	glVertex3f(1, -0.5f, -2);
	glVertex3f(-1, -0.5f, -2);

	glEnd();
}

int main(int argc, char **argv) {
	char quit = 0;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not be initialized! SDL_Error: %s\n", SDL_GetError());
		return 0;
	}

#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
	if (!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0")) {
		printf("SDL can not disable compositor bypass!\n");
		return 0;
	}
#endif

	SDL_Window *window = SDL_CreateWindow("SDL Window", //
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, //
			800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if (!window) {
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return 0;
	}

	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	if (!glContext) {
		printf("GL Context could not be created! SDL_Error: %s\n", SDL_GetError());
	}

	glViewport(0, 0, 800, 600);
	glClearColor(1.f, 1.f, 1.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(0.0f, 0.0f, 0.0f);

	glMatrixMode( GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0, 1, 0.1, 1000);

	tnfs_reset();

	while (!quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = 1;
			}
			handleKeys();
		}

		tnfs_update();

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderGl();
		SDL_GL_SwapWindow(window);

		SDL_Delay(30);
	}
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
