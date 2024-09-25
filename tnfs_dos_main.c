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
	int i, j, n;

	for (n = 0; n < 8; n++) {

		i = segment - 1 + n;
		if (i < 0) {
			i = i + road_segment_count;
		} else if (i >= road_segment_count) {
			i = i - road_segment_count;
		}
		j = i + 1;

		_moveto((int) ((-track_data[i].vf_margin_L.z - ox) * scale), (int) ((track_data[i].vf_margin_L.x - oy) * scale));
		_lineto((int) ((-track_data[j].vf_margin_L.z - ox) * scale), (int) ((track_data[j].vf_margin_L.x - oy) * scale));
		_lineto((int) ((-track_data[j].vf_margin_R.z - ox) * scale), (int) ((track_data[j].vf_margin_R.x - oy) * scale));
		_lineto((int) ((-track_data[i].vf_margin_R.z - ox) * scale), (int) ((track_data[i].vf_margin_R.x - oy) * scale));
		_lineto((int) ((-track_data[i].vf_margin_L.z - ox) * scale), (int) ((track_data[i].vf_margin_L.x - oy) * scale));
	}
}

void drawCar(tnfs_car_data *car, float x, float y) {
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
	int i;
	float x0, y0, x1, y1;

	//inverted axis from 3d world
	x0 = (float) player_car_ptr->position.z / 0x10000; //to meter scale
	y0 = (float) player_car_ptr->position.x / 0x10000;
	drawRoad(x0 - 10, y0 - 20, player_car_ptr->road_segment_a);

	for (i = 1; i < g_total_cars_in_scene; i++) {
		x1 = x0 - ((float) g_car_ptr_array[i]->position.z / 0x10000);
		y1 = y0 - ((float) g_car_ptr_array[i]->position.x / 0x10000);
		drawCar(g_car_ptr_array[i], 10 - x1, 20 - y1);
	}

	drawCar(player_car_ptr, 10, 20);
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
				player_car_ptr->handbrake = 0;
				g_control_steer = 0;
				break;
			case 80:
				g_control_brake = 1;
				g_control_throttle = 0;
				player_car_ptr->handbrake = 0;
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
				player_car_ptr->handbrake = 1;
				break;
			case 114:
				tnfs_reset_car(player_car_ptr);
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
		printf("%d m/s - %d rpm - gear %d\n", player_car_ptr->speed_local_lon >> 16, player_car_ptr->rpm_engine, player_car_ptr->gear_selected + 1);
		render();

		tnfs_update();
		delay(30);
	}

	_setvideomode(_DEFAULTMODE);
	return 0;
}
