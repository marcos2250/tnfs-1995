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
static SDL_Window *window;
static SDL_GLContext glContext;
static SDL_GameController *controller;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

GLfloat matrix[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
vector3f cam_orientation = { 0, 0, 0 };

void sys_sdl_exit() {
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);
	SDL_Quit();
	exit(0);
}

void msg_abort(char * text, char * a, char * b, char * c) {
	printf(text, a, b, c);
	sys_sdl_exit();
}

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
 * gamepad: left stick steers, triggers are throttle and brake, shoulders shift gears, A is the handbrake
 */
void handleGamepadEvents() {
	if (event.type == SDL_CONTROLLERDEVICEADDED && !controller) {
		controller = SDL_GameControllerOpen(event.cdevice.which);
		if (controller) {
			printf("Gamepad: %s\n", SDL_GameControllerName(controller));
			g_control_analog_steering = 1;
			g_control_analog_pedals = 1;
		}
	}
	if (event.type == SDL_CONTROLLERDEVICEREMOVED && controller
			&& event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller))) {
		SDL_GameControllerClose(controller);
		controller = 0;
		g_control_analog_steering = 0;
		g_control_analog_pedals = 0;
	}
	if (event.type == SDL_CONTROLLERBUTTONDOWN) {
		switch (event.cbutton.button) {
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			tnfs_change_gear_up();
			break;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
			tnfs_change_gear_down();
			break;
		case SDL_CONTROLLER_BUTTON_A:
			g_car_array[0].handbrake = 1;
			break;
		}
	}
	if (event.type == SDL_CONTROLLERBUTTONUP && event.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
		g_car_array[0].handbrake = 0;
	}
}

/*
 * gamepad axes to the joystick values of Win95 0x43d598, calibrated to the SDL axis ranges as Win95 0x43bc00 does:
 * stick: 35.0 / (range + 1) per side (range capped at 0x7ffa), 3 units dead zone, -0x20..0x1f
 * pedals (separate pedal axes mode): 36.0 / ((max - min) / 2), 0x1f pressed .. 0 released, value brake - gas
 */
void readGamepadAxes() {
	int x, gas, brake, pedal;
	int gain_stick = 0x230000 / (0x7ffa + 1);
	int gain_pedal = 0x240000 / (0x7fff >> 1);

	if (!controller) {
		return;
	}

	x = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
	if (x < 0) {
		x = ((x * gain_stick) >> 16) + 3;
		if (x > 0) {
			x = 0;
		} else if (x < -0x20) {
			x = -0x20;
		}
	} else {
		x = ((x * gain_stick) >> 16) - 3;
		if (x < 0) {
			x = 0;
		} else if (x > 0x1f) {
			x = 0x1f;
		}
	}

	// pedal axes are low when pressed, SDL triggers go 0 (released) .. 0x7fff (pressed)
	gas = 0x1f - ((((0x7fff - SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT)) >> 1) * gain_pedal) >> 16);
	brake = 0x1f - ((((0x7fff - SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT)) >> 1) * gain_pedal) >> 16);
	if (gas < 0) {
		gas = 0;
	}
	if (brake < 0) {
		brake = 0;
	}
	pedal = brake - gas;
	if (pedal < -0x1e) {
		pedal = -0x1e;
	} else if (pedal > 0x1e) {
		pedal = 0x1e;
	}

	g_control_steer_axis = x;
	g_control_pedal_axis = pedal;
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
	if (cam_orientation.x != 0) {
		glRotatef(cam_orientation.x, 1, 0, 0);
	}
	if (cam_orientation.z != 0) {
		glRotatef(cam_orientation.z, 0, 0, 1);
	}
	glRotatef(cam_orientation.y, 0, 1, 0);
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
	if (cam_orientation.x != 0) {
		glRotatef(cam_orientation.x, 1, 0, 0);
	}
	if (cam_orientation.z != 0) {
		glRotatef(cam_orientation.z, 0, 0, 1);
	}
	glRotatef(cam_orientation.y, 0, 1, 0);
	glTranslatef(((float) -camera.position.x) / 0x10000, ((float) -camera.position.y) / 0x10000, ((float) camera.position.z) / 0x10000);

	glColor3f(0.0f, 0.0f, 0.0);
	glBegin(GL_QUADS);

	max = g_road_node_count - 1;
	for (int n = 0; n < 100; n++) {

		i = camera.track_slice;
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

void gfx_update() {
	cam_orientation.x = ((float) camera.orientation.x) * 0.0000214576733981; //(360/0xFFFFFF)
	cam_orientation.y = ((float) camera.orientation.y) * 0.0000214576733981; //(360/0xFFFFFF)
	cam_orientation.z = -((float) camera.orientation.z) * 0.0000214576733981; //(360/0xFFFFFF)

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0, 1, 0.1, 1000);

	drawRoad();
	for (int i = 0; i < g_total_cars_in_scene; i++) {
		if (g_car_ptr_array[i]->field_4e9 & 4) {
			drawVehicle(g_car_ptr_array[i]);
		}
	}
	drawTach();
}

int main(int argc, char **argv) {
	char quit = 0;
	char * trifile = 0;
	char * pbsfile = 0;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
		printf("SDL could not be initialized! SDL_Error: %s\n", SDL_GetError());
		return 0;
	}

#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
	if (!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0")) {
		printf("SDL can not disable compositor bypass!\n");
		return 0;
	}
#endif

	window = SDL_CreateWindow("SDL Window", //
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, //
			SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if (!window) {
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return 0;
	}

	glContext = SDL_GL_CreateContext(window);
	if (!glContext) {
		printf("GL Context could not be created! SDL_Error: %s\n", SDL_GetError());
	}

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glClearColor(1.f, 1.f, 1.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(0.0f, 0.0f, 0.0f);

	if (argc > 1) {
		trifile = argv[1];
		if (argc > 2) {
			pbsfile = argv[2];
		}
	} else {
		printf("command usage: tnfs <files/tr1.tri> <files/car.pbs>\n");
		trifile = 0;
		pbsfile = 0;
	}

	tnfs_init_sim(trifile, pbsfile);

	while (!quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = 1;
			}
			handleKeys();
			handleGamepadEvents();
		}
		readGamepadAxes();

		tnfs_update();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gfx_update();

		SDL_GL_SwapWindow(window);
		SDL_Delay(30);
	}

	sys_sdl_exit();
	return 0;
}
