/*
 * TNFS 1995
 * MS-DOS build w/ OpenWatcom 1.9/DOS4GW
 */
#include <stdio.h>
#include <conio.h>
#include <graph.h>
#include <math.h>
#include "tnfs_math.h"
#include "tnfs_base.h"

int scale = 10;

void drawRect(float x, float y, double a, float l, float w) {
	float cay, cax, cby, cbx;
	double s, c;

	scale = 10;
	s = sin(a);
	c = cos(a);
	cay = w * s + l * c;
	cax = w * c - l * s;
	cby = w * s - l * c;
	cbx = w * c + l * s;

	_moveto((x + cax) * scale, (y + cay) * scale);
	_lineto((x + cbx) * scale, (y + cby) * scale);
	_lineto((x - cax) * scale, (y - cay) * scale);
	_lineto((x - cbx) * scale, (y - cby) * scale);
	_lineto((x + cax) * scale, (y + cay) * scale);
}

void drawRoad(float ox, float oy, int segment) {
	int i, j, n, max;

	max = road_segment_count - 1;
	for (n = 0; n < 8; n++) {

		i = segment - 1 + n;
		if (i < 0) {
			i = i + max;
		} else if (i >= max) {
			i = i - max;
		}
		j = i + 1;

		_moveto((int) ((-track_data[i].vf_margin_L.z - ox) * scale), (int) ((track_data[i].vf_margin_L.x - oy) * scale));
		_lineto((int) ((-track_data[j].vf_margin_L.z - ox) * scale), (int) ((track_data[j].vf_margin_L.x - oy) * scale));
		_lineto((int) ((-track_data[j].vf_margin_R.z - ox) * scale), (int) ((track_data[j].vf_margin_R.x - oy) * scale));
		_lineto((int) ((-track_data[i].vf_margin_R.z - ox) * scale), (int) ((track_data[i].vf_margin_R.x - oy) * scale));
		_lineto((int) ((-track_data[i].vf_margin_L.z - ox) * scale), (int) ((track_data[i].vf_margin_L.x - oy) * scale));
	}
}

void drawCar(tnfs_car_data * car, float x, float y) {
	float a, as, s, c, cay, cax, cby, cbx;

	a = (float) car->angle_y / 2670179; //to radians
	as = a + ((float) car->steer_angle / 2670179);

	//body
	drawRect(x, y, a, 1, 2);

	//wheels
	s = sin(a);
	c = cos(a);
	cay = 1.25 * s + 0.9 * c;
	cax = 1.25 * c - 0.9 * s;
	cby = 1.25 * s - 0.9 * c;
	cbx = 1.25 * c + 0.9 * s;
	drawRect(x + cax, y + cay, as, 0.1, 0.3);
	drawRect(x + cbx, y + cby, as, 0.1, 0.3);
	drawRect(x - cax, y - cay, a, 0.1, 0.3);
	drawRect(x - cbx, y - cby, a, 0.1, 0.3);
}

void render() {
	float x, y;

	//inverted axis from 3d world
	x = (float) car_data.position.z / 0x10000; //to meter scale
	y = (float) car_data.position.x / 0x10000;
	drawRoad(x - 10, y - 20, car_data.road_segment_a);

	x -= (float) xman_car_data.position.z / 0x10000;
	y -= (float) xman_car_data.position.x / 0x10000;
	drawCar(&xman_car_data, 10 - x, 20 - y);

	drawCar(&car_data, 10, 20);
}

int main(int argc, char **argv) {
	char *trifile;
	int i;
	int v;
	int playing;

	trifile = 0;
	// command usage: tnfs [tr1.tri] (read a track file)
	if (argc > 1) {
		trifile = argv[1];
	}

	tnfs_init_sim(trifile);

	_clearscreen(_GCLEARSCREEN);
	_setvideomode(_VRES16COLOR);

	playing = 1;
	while (playing) {

		// read keys, not very responsive
		if (kbhit()) {
			switch (getch()) {
			case 72:
				g_control_throttle = 1;
				g_control_brake = 0;
				car_data.handbrake = 0;
				g_control_steer = 0;
				break;
			case 80:
				g_control_brake = 1;
				g_control_throttle = 0;
				car_data.handbrake = 0;
				break;
			case 75:
				g_control_steer = -1;
				break;
			case 77:
				g_control_steer = 1;
				break;
			case 27:
				playing = 0;
				break;
			case 32:
				car_data.handbrake = 1;
				break;
			case 114:
				tnfs_reset_car(&car_data);
				break;
			case 97:
				tnfs_change_gear_up();
				break;
			case 122:
				tnfs_change_gear_down();
				break;
			}
		}

		_clearscreen(_GCLEARSCREEN);
		printf("%d m/s - %d rpm - gear %d\n", car_data.speed_local_lon >> 16, car_data.rpm_engine, car_data.gear_selected + 1);
		render();

		tnfs_update();
		delay(30);
	}

	_setvideomode(_DEFAULTMODE);
	return 0;
}
