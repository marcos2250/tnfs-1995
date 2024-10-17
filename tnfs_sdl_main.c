/*
 * TNFS 1995 car physics code
 * Recreation and analysis of the physics engine from the classic racing game
 */
#include <SDL.h>
#include <SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "tnfs_math.h"
#include "tnfs_base.h"

static SDL_Event event;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

GLfloat matrix[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
float cam_angle = 0;

void handleKeys() {
	if (event.type == SDL_KEYDOWN) {
		switch (event.key.keysym.sym) {
		case SDLK_LEFT:
			g_control_steer = -1;
			break;
		case SDLK_RIGHT:
			g_control_steer = 1;
			break;
		case SDLK_UP:
			g_control_throttle = 1;
			break;
		case SDLK_DOWN:
			g_control_brake = 1;
			break;
		case SDLK_SPACE:
			g_car_array[0].handbrake = 1;
			break;
		}
	}
	if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
		switch (event.key.keysym.sym) {
		case SDLK_a:
			tnfs_change_gear_up();
			break;
		case SDLK_z:
			tnfs_change_gear_down();
			break;
		case SDLK_r:
			tnfs_reset_car(g_car_ptr_array[0]);
			break;
		case SDLK_c:
			tnfs_change_camera();
			break;
		case SDLK_d:
			tnfs_crash_car();
			break;
		case SDLK_F1:
			tnfs_abs();
			break;
		case SDLK_F2:
			tnfs_tcs();
			break;
		case SDLK_F3:
			tnfs_change_traction();
			break;
		case SDLK_F4:
			tnfs_change_transmission_type();
			break;
		case SDLK_F5:
			tnfs_cheat_mode();
			break;
		default:
			break;
		}
	}
	if (event.type == SDL_KEYUP) {
		switch (event.key.keysym.sym) {
		case SDLK_UP:
			g_control_throttle = 0;
			break;
		case SDLK_DOWN:
			g_control_brake = 0;
			break;
		case SDLK_LEFT:
			g_control_steer = 0;
			break;
		case SDLK_RIGHT:
			g_control_steer = 0;
			break;
		case SDLK_SPACE:
			g_car_array[0].handbrake = 0;
			break;
		default:
			break;
		}
	}
}

/*
 * TNFS coord system
 * position X+ right Y+ up Z+ north
 * angle X+ pitch down Y+ yaw clockwise Z+ roll left
 */
void drawVehicle(tnfs_car_data * car) {
	// TNFS uses LHS, convert to OpenGL's RHS
	glMatrixMode(GL_MODELVIEW);
	matrix[0] = (float) car->matrix.ax / 0x10000;
	matrix[1] = (float) car->matrix.ay / 0x10000;
	matrix[2] = (float) -car->matrix.az / 0x10000;
	matrix[3] = 0;
	matrix[4] = (float) car->matrix.bx / 0x10000;
	matrix[5] = (float) car->matrix.by / 0x10000;
	matrix[6] = (float) -car->matrix.bz / 0x10000;
	matrix[7] = 0;
	matrix[8] = (float) car->matrix.cx / 0x10000;
	matrix[9] = (float) car->matrix.cy / 0x10000;
	matrix[10] = (float) -car->matrix.cz / 0x10000;
	matrix[11] = 0;
	matrix[12] = ((float) car->position.x) / 0x10000;
	matrix[13] = ((float) car->position.y) / 0x10000;
	matrix[14] = ((float) -car->position.z) / 0x10000;
	matrix[15] = 1;

	glLoadIdentity();
	glRotatef(cam_angle, 0, 1, 0);
	glTranslatef(((float) -camera.position.x) / 0x10000, ((float) -camera.position.y) / 0x10000, ((float) camera.position.z) / 0x10000);
	glMultMatrixf(matrix);

	if (car == player_car_ptr) {
		glColor3f(0.0f, 0.0f, 1.0f); //player
	} else if (car->ai_state & 0x4) {
		glColor3f(1.0f, 0.0f, 0.0f); //opponents
	} else if (car->ai_state & 0x8) {
		glColor3f(0.2f, 0.2f, 0.2f); //police
	} else {
		glColor3f(0.0f, 0.5f, 0.0f); //traffic
	}

	glBegin(GL_QUADS);

	// front bumper
	glVertex3f(-1, 0, 2.1f);
	glVertex3f(1, 0, 2.1f);
	glVertex3f(1, 0.8f, 2.0f);
	glVertex3f(-1, 0.8f, 2.0f);

	// hood
	glVertex3f(-1, 0.8f, 2.0f);
	glVertex3f(1, 0.8f, 2.0f);
	glVertex3f(1, 0.9f, 0.3f);
	glVertex3f(-1, 0.9f, 0.3f);

	// windshield
	glVertex3f(-1, 0.9f, 0.3f);
	glVertex3f(1, 0.9f, 0.3f);
	glVertex3f(1, 1.4f, -0.3f);
	glVertex3f(-1, 1.4f, -0.3f);

	// roof/trunk
	glVertex3f(-1, 1.4f, -0.3f);
	glVertex3f(1, 1.4f, -0.3f);
	glVertex3f(1, 1.0f, -2.3f);
	glVertex3f(-1, 1.0f, -2.3f);

	// rear bumper
	glVertex3f(-1, 0, -2.3f);
	glVertex3f(1, 0, -2.3f);
	glVertex3f(1, 1.0f, -2.3f);
	glVertex3f(-1, 1.0f, -2.3f);

	// bottom
	glVertex3f(-1, 0, -2.3f);
	glVertex3f(1, 0, -2.3f);
	glVertex3f(1, 0, 2.1f);
	glVertex3f(-1, 0, 2.1f);

	glEnd();
}

void drawRoad() {
	int max, i, j;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(cam_angle, 0, 1, 0);
	glTranslatef(((float) -camera.position.x) / 0x10000, ((float) -camera.position.y) / 0x10000, ((float) camera.position.z) / 0x10000);

	glColor3f(0.0f, 0.0f, 0.0);
	glBegin(GL_QUADS);

	max = g_road_node_count - 1;
	for (int n = 0; n < 100; n++) {

		i = g_car_array[camera.car_id].track_slice;
		i = i - 50 + n;
		if (i < 0) {
			i = i + max;
		} else if (i >= max) {
			i = i - max;
		}
		j = i + 1;

		// road
		glVertex3f(track_data[i].vf_margin_L.x, track_data[i].vf_margin_L.y, track_data[i].vf_margin_L.z);
		glVertex3f(track_data[j].vf_margin_L.x, track_data[j].vf_margin_L.y, track_data[j].vf_margin_L.z);
		glVertex3f(track_data[j].vf_margin_R.x, track_data[j].vf_margin_R.y, track_data[j].vf_margin_R.z);
		glVertex3f(track_data[i].vf_margin_R.x, track_data[i].vf_margin_R.y, track_data[i].vf_margin_R.z);
		// left fence
		glVertex3f(track_data[i].vf_fence_L.x, track_data[i].vf_fence_L.y, track_data[i].vf_fence_L.z);
		glVertex3f(track_data[i].vf_fence_L.x, track_data[i].vf_fence_L.y+1, track_data[i].vf_fence_L.z);
		glVertex3f(track_data[j].vf_fence_L.x, track_data[j].vf_fence_L.y+1, track_data[j].vf_fence_L.z);
		glVertex3f(track_data[j].vf_fence_L.x, track_data[j].vf_fence_L.y, track_data[j].vf_fence_L.z);
		// right fence
		glVertex3f(track_data[j].vf_fence_R.x, track_data[j].vf_fence_R.y, track_data[j].vf_fence_R.z);
		glVertex3f(track_data[j].vf_fence_R.x, track_data[j].vf_fence_R.y+1, track_data[j].vf_fence_R.z);
		glVertex3f(track_data[i].vf_fence_R.x, track_data[i].vf_fence_R.y+1, track_data[i].vf_fence_R.z);
		glVertex3f(track_data[i].vf_fence_R.x, track_data[i].vf_fence_R.y, track_data[i].vf_fence_R.z);
	}
	glEnd();
}

void drawTach() {
	float c,s,r;
	r = ((float) g_car_array[0].rpm_engine / (float) g_car_array[0].rpm_redline) * 3.14 - 1.56;
	c = -cosf(r);
	s = sinf(r);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, -1.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
	matrix[0] = c; matrix[1] = -s; matrix[2] = 0; matrix[3] = 0;
	matrix[4] = s; matrix[5] = c; matrix[6] = 0; matrix[7] = 0;
	matrix[8] = 0; matrix[9] = 0; matrix[10] = 0; matrix[11] = 0;
	matrix[12] = 100; matrix[13] = 520; matrix[14] = 0; matrix[15] = 1;
	glLoadMatrixf(matrix);

	glColor3f(1.0f, 0.0f, 0.0);
	glBegin(GL_TRIANGLES);
	glVertex3f(-2, 0, 0);
	glVertex3f(+2, 0, 0);
	glVertex3f(0, 80, 0);
	glEnd();
}

void renderGl() {
	cam_angle = ((float) camera.orientation.y) * 0.0000214576733981; //(360/0xFFFFFF)

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0, 1, 0.1, 1000);

	drawRoad();
	for (int i = 0; i < g_total_cars_in_scene; i++) {
	  drawVehicle(g_car_ptr_array[i]);
	}
	drawTach();
}

int main(int argc, char **argv) {
	char quit = 0;
	char * trifile = 0;

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

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glClearColor(1.f, 1.f, 1.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(0.0f, 0.0f, 0.0f);

	// command usage: tnfs [tr1.tri] (read a track file)
	if (argc > 1) {
		trifile = argv[1];
	}

	tnfs_init_sim(trifile);

	while (!quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = 1;
			}
			handleKeys();
		}

		tnfs_update();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderGl();

		SDL_GL_SwapWindow(window);
		SDL_Delay(30);
	}

	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
