/*
 * tnfs_collision_2d.c
 * 2D road fence collision
 */
#include "tnfs_math.h"
#include "tnfs_base.h"
#include "tnfs_fiziks.h"
#include "tnfs_collision_3d.h"

int crash_pitch = 0;
int DAT_80111a40 = 0;

void tnfs_collision_rotate(tnfs_car_data *car_data, int angle, int lon_speed, int fence_side, int road_flag, int aux_angle) {
	int d_angle;
	int crash_speed_a;
	int crash_speed_b;
	int crash_roll;
	int crash_yaw;
	int collisionAngle;
	int rotAngle;
	int rotSide;

	if (fence_side <= 0)
		d_angle = aux_angle + angle;
	else
		d_angle = angle - aux_angle;

	rotAngle = d_angle - car_data->angle.y + (-(d_angle - car_data->angle.y < 0) & 0x1000000);
	if (rotAngle <= 0x800000)
		collisionAngle = d_angle - car_data->angle.y + (-(d_angle - car_data->angle.y < 0) & 0x1000000);
	else
		collisionAngle = 0x1000000 - rotAngle;
	if (collisionAngle > 0x400000) {
		rotAngle += 0x800000;
		collisionAngle = 0x1000000 - rotAngle;
	}
	if (rotAngle > 0x1000000)
		rotAngle -= 0x1000000;

	if (lon_speed >= 0x180000 && car_data->speed > 0x230000) {
		crash_speed_a = lon_speed - 0x60000;
		if (rotAngle >= 0x800000)
			rotSide = 0x1000000 - fix3(0x1000000 - rotAngle);
		else
			rotSide = fix3(rotAngle);

		if (rotSide > 0x800000)
			rotSide -= 0x1000000;

		if (crash_speed_a > 0xE0000)
			crash_speed_a = 0xE0000;

		crash_speed_b = fix6(crash_speed_a * (collisionAngle >> 16));

		if (rotSide <= 0) {
			crash_yaw = -crash_speed_a;
			crash_roll = -(crash_speed_a - crash_speed_b);
		} else {
			crash_yaw = crash_speed_a;
			crash_roll = crash_speed_a - crash_speed_b;
		}

		if (road_flag) {
			crash_pitch = -crash_speed_b;
		} else {
			crash_roll = -crash_roll;
			crash_pitch = fix6(crash_speed_a * (collisionAngle >> 16));
		}
		if (crash_yaw != 0) {
			tnfs_collision_rollover_start(car_data, fix2(crash_yaw), fix2(3 * crash_roll), fix2(3 * crash_pitch));
		}
	} else {
		if (abs(collisionAngle) < 0x130000 && car_data->speed_local_lon > 0) {
			if (fence_side <= 0) {
				if (rotAngle < 0x130000) {
					car_data->angle.y += rotAngle;
					car_data->angular_speed = 0x9FFFC + 4;
				}
			} else {
				if (rotAngle > 0xed0001) {
					car_data->angle.y += rotAngle;
					car_data->angular_speed -= 0x9FFFC + 4;
				}
			}
		}
	}
}


int tnfs_collision_car_size(tnfs_car_data *car_data, int fence_angle) {
	int x;

	// fast cosine
	x = abs(fence_angle - car_data->angle.y) >> 0x10;
	if (x > 0xc1) {
		x = 0x100 - x;
	} else if (x > 0x80) {
		x = x - 0x80;
	} else if (x > 0x40) {
		x = 0x80 - x;
	}

	return ((((car_data->car_length - car_data->car_width) * x) >> 6) + car_data->car_width) >> 1;
}

void tnfs_track_fence_collision(tnfs_car_data *car_data) {
	int abs_speed;
	int distance;
	int aux;
	long fence_angle;
	int fenceSide;
	int fence_flag;
	int rebound_speed_x;
	int rebound_speed_z;
	int rebound_x;
	int rebound_z;
	int sfxA;
	int sfxB;
	int fence_sin;
	int fence_cos;
	int re_speed;
	int lm_speed;
	int roadPositionX;
	int roadPositionZ;
	int roadLeftMargin;
	int roadRightMargin;
	int roadLeftFence;
	int roadRightFence;

	roadPositionX = track_data[car_data->track_slice & g_slice_mask].pos.x;
	roadPositionZ = track_data[car_data->track_slice & g_slice_mask].pos.z;

	roadLeftMargin = track_data[car_data->track_slice & g_slice_mask].roadLeftMargin;
	roadRightMargin = track_data[car_data->track_slice & g_slice_mask].roadRightMargin;
	roadLeftFence = track_data[car_data->track_slice & g_slice_mask].roadLeftFence;
	roadRightFence = track_data[car_data->track_slice & g_slice_mask].roadRightFence;

	fence_angle = track_data[car_data->track_slice & g_slice_mask].heading * 0x400;
	fence_sin = math_sin_3(fence_angle);
	fence_cos = math_cos_3(fence_angle);
	fence_flag = 0;
	fenceSide = 0;
	distance = fixmul(fence_sin, roadPositionZ - car_data->position.z) - fixmul(fence_cos, roadPositionX - car_data->position.x);

	rebound_speed_x = 0;
	if (distance < roadLeftMargin * -0x2000) {
		// left road side
		car_data->surface_type = track_data[car_data->track_slice & g_slice_mask].shoulder_surface_type >> 4;
		aux = roadLeftFence * -0x2000 + tnfs_collision_car_size(car_data, fence_angle);
		if (distance < aux) {
			fenceSide = -0x100;
			rebound_speed_x = aux - distance - 0x4ccc;
			fence_flag = track_data[car_data->track_slice & g_slice_mask].fence_flag >> 4;
		}

	} else if (distance > roadRightMargin * 0x2000) {
		// right road side
		car_data->surface_type = track_data[car_data->track_slice & g_slice_mask].shoulder_surface_type & 0xf;
		aux = roadRightFence * 0x2000 - tnfs_collision_car_size(car_data, fence_angle);
		if (distance > aux) {
			fenceSide = 0x100;
			rebound_speed_x = distance + 0x4ccc - aux;
			fence_flag = track_data[car_data->track_slice & g_slice_mask].fence_flag & 0xf;
		}
	} else {
		car_data->surface_type = 0;
	}
	if (fenceSide == 0) {
		return;
	}

	// reposition the car back off the fence
	math_rotate_2d(fix8(fenceSide *  abs(rebound_speed_x)), 0, fence_angle, &rebound_x, &rebound_z);
	car_data->position.x = car_data->position.x - rebound_x;
	car_data->position.z = car_data->position.z + rebound_z;

	// rotate speed vector to fence reference
	math_rotate_2d(-car_data->speed_x, car_data->speed_z, fence_angle, &rebound_speed_x, &rebound_speed_z);

	// play collision sound
	abs_speed = abs(rebound_speed_x);
	if ((fence_flag == 0) && (abs_speed >= 0x60001)) {
		if (sound_flag == 0) {
			if (car_data->car_id2 == 0) {
				if (DAT_80111a40 != 0) {
					if (distance <= 0)
						sfxA = 0x280000;
					else
						sfxA = 0xd70000;
					sfxB = 1;
				} else {
					tnfs_car_local_position_vector(car_data, &sfxA, &sfxB);
				}
			}
		} else {
			if (car_data->car_id2 <= 0) {
				sfxA = 0x400000;
			} else {
				sfxA = 0xc00000;
			}
		}
		tnfs_sfx_play(-1, 2, 9, abs_speed, sfxB, sfxA);
		if (abs_speed > 0x140000) {
			tnfs_replay_highlight_record(0x32);
		}
	}

	// dampen collision rebound speed
	if (abs_speed > 0x180000) {
		if (rebound_speed_x > +0x20000)
			rebound_speed_x = +0x20000;
		if (rebound_speed_x < -0x20000)
			rebound_speed_x = -0x20000;
	} else {
		rebound_speed_x = fix2(rebound_speed_x);
	}
	if (rebound_speed_z < -0x30000) {
		if ((g_game_settings & 0x20) == 0) {
			rebound_speed_z = abs_speed + rebound_speed_z;
			lm_speed = fix2(abs_speed);
		} else {
			lm_speed = abs_speed / 2;
		}
		rebound_speed_z = rebound_speed_z + lm_speed;
		if (0 < rebound_speed_z) {
			rebound_speed_z = 0;
		}
	} else if (rebound_speed_z > 0x30000) {
		if ((g_game_settings & 0x20) == 0) {
			lm_speed = fix2(abs_speed);
			aux = abs_speed;
		} else {
			lm_speed = fix3(abs_speed);
			aux = abs_speed / 2;
		}
		rebound_speed_z = rebound_speed_z - lm_speed - aux;
		if (rebound_speed_z < 0) {
			rebound_speed_z = 0;
		}
	}

	// set collision angle side
	re_speed = fix2(abs_speed);
	if (re_speed > 0xa0000) {
		re_speed = 0xa0000;
	}
	if (fenceSide < 1) {
		fence_angle += 0x20000;
		car_data->collision_x = -re_speed;
	} else {
		fence_angle -= 0x20000;
		car_data->collision_x = re_speed;
	}

	car_data->collision_y = -re_speed;
	car_data->collision_a = 0x1e;
	car_data->collision_b = 0x1e;

	// reflect car speeds 180deg
	math_rotate_2d(rebound_speed_x, rebound_speed_z, fence_angle, &car_data->speed_x, &car_data->speed_z);

	// ???
	if (fence_flag == 0) {
		aux = 0x30000;
		fence_flag = 0;
	} else {
		aux = 0x18000;
	}
	tnfs_collision_rotate(car_data, fence_angle, abs_speed, fenceSide, fence_flag, aux);

	if (car_data->speed_y > 0) {
		car_data->speed_y = 0;
	}
}

