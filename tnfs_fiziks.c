/*
 * The "Fiziks" module
 * 2D car handling dynamics and road fence collision
 */
#include "tnfs_math.h"
#include "tnfs_base.h"
#include "tnfs_collision.h"

char is_drifting;
int unknown_stats_array[128];

// settings/flags
int is_not_in_replay = 1;
int general_flags = 0x14;
int debug_sum = 0;
int crash_pitch = 0;
int selected_track = 3; //rusty springs
int DAT_8010d1c4 = 0;
int DAT_8010d1cc = 0;
int DAT_8010d2f4 = 0;
int DAT_8010d30c = 0;
int DAT_80111a40 = 0;

// debug stats flags
int dword_D8AE8 = 0;
int dword_D8AEC = 0;
int dword_D8AF0 = 0;
int dword_D8AF4 = 0;
int dword_D8AF8 = 0;
int dword_D8AFC = 0;
int dword_D8B00 = 0;
int dword_122CAC = 0;
int dword_132EFC = 0;
int dword_146483 = 0;


void tnfs_debug_00034309(int a, int b, int x, int y) {
	//TODO
}

void tnfs_debug_000343C2(int a, int b) {
	//TODO
}

void tnfs_debug_000502AB(char a) {
	//TODO
}

int tnfs_collision_car_size(tnfs_car_data *car_data, int a) {
	return 0x10000;
}

void tnfs_engine_rev_limiter(tnfs_car_data *car_data) {
	//TODO
}

int tnfs_engine_thrust(tnfs_car_data *car_data) {
	if (car_data->gear_RND == 3) { //drive
		if (car_data->speed < 600000)
			is_drifting = 1;
		return car_data->throttle * 10000;
	}
	if (car_data->gear_RND == 1) { //reverse
		return car_data->throttle * -10000;
	}
	return 0; //neutral
}

void tnfs_engine_auto_shift(tnfs_car_data *car_data) {
	//TODO
}

int tnfs_drag_force(tnfs_car_data *car_data, int speed) {
	return -(speed >> 4);
}

/*
 * crashing cars cheat code
 */
void tnfs_cheat_crash_cars() {
	if (cheat_mode == 4) {
		tnfs_collision_rollover_start(&car_data, 0xa0000, 0xa0000, 0xa0000);
	}
}

void tnfs_collision_rotate(tnfs_car_data *car_data, int angle, int a3, int fence_side, int road_flag, int fence_angle) {
	int v9;
	int v10;
	int v12;
	int crash_roll;
	signed int crash_yaw;
	signed int collisionAngle;
	signed int rotAngle;
	int v17;

	if (fence_side <= 0)
		v9 = fence_angle + angle;
	else
		v9 = angle - fence_angle;

	rotAngle = v9 - car_data->angle_y + (-(v9 - car_data->angle_y < 0) & 0x1000000);
	if (rotAngle <= 0x800000)
		collisionAngle = v9 - car_data->angle_y + (-(v9 - car_data->angle_y < 0) & 0x1000000);
	else
		collisionAngle = 0x1000000 - rotAngle;
	if (collisionAngle > 0x400000) {
		rotAngle += 0x800000;
		collisionAngle = 0x1000000 - rotAngle;
	}
	if (rotAngle > 0x1000000)
		rotAngle -= 0x1000000;

	if (a3 >= 0x1000000 && car_data->speed > 0x230000) {
		v10 = a3 - 0x60000;
		if (rotAngle >= 0x800000)
			v17 = 0x1000000 - fix3(0x1000000 - rotAngle);
		else
			v17 = fix3(rotAngle);

		if (v17 > 0x800000)
			v17 -= 0x1000000;
		if (v10 > 0xE0000)
			v10 = 0xE0000;

		v12 = fix6(v10 * (collisionAngle >> 16));
		if (v17 <= 0) {
			crash_yaw = -v10;
			crash_roll = -(v10 - v12);
		} else {
			crash_yaw = v10;
			crash_roll = v10 - v12;
		}
		if (road_flag) {
			crash_pitch = -v12;
		} else {
			crash_roll = -crash_roll;
			crash_pitch = fix6(v10 * (collisionAngle >> 16));
		}
		if (crash_yaw != 0) {
			tnfs_collision_rollover_start(car_data->car_data_pointer, fix2(crash_yaw), fix2(3 * crash_roll), fix2(3 * crash_pitch));
		}
	} else {
		if (abs(collisionAngle) < 0x130000 && car_data->speed_local_lon > 0) {
			if (fence_side <= 0) {
				if (rotAngle < 0x130000) {
					car_data->angle_y += rotAngle;
					car_data->angular_speed = 0x9FFFC + 4;
				}
			} else {
				if (rotAngle > 0xed0001) {
					car_data->angle_y += rotAngle;
					car_data->angular_speed -= 0x9FFFC + 4;
				}
			}
		}
	}
}

void tnfs_track_fence_collision(tnfs_car_data *car_data) {
	int abs_speed;
	int distance;
	int aux;
	long fence_angle;
	int fenceSide;
	int road_flag;
	int rebound_speed_x;
	int rebound_speed_z;
	int rebound_x;
	int rebound_z;
	int local_angle;
	int local_length;
	int pos_x;
	int pos_y;
	int re_speed;
	int lm_speed;

	//fence_angle = (dword_12DECC + 36 * (dword_1328E4 & car_data->road_segment_a) + 22) >> 16 << 10;
	fence_angle = road_segment_heading * 0x400;
	pos_x = math_sin_2(fence_angle >> 8);
	pos_y = math_cos_2(fence_angle >> 8);
	road_flag = 0;
	fenceSide = 0;
	distance = fixmul(pos_x, road_segment_pos_z - car_data->position.z) - fixmul(pos_y, road_segment_pos_x - car_data->position.x);

	rebound_speed_x = 0;
	if (distance < roadLeftMargin * -0x2000) {
		// left road side
		car_data->surface_type = roadConstantB >> 4;
		aux = tnfs_collision_car_size(car_data, fence_angle);
		aux = roadLeftFence * -0x2000 + aux;
		if (distance < aux) {
			fenceSide = -0x100;
			rebound_speed_x = aux - distance - 0x8000;
			road_flag = roadConstantA >> 4;
		}

	} else if (distance > roadRightMargin * 0x2000) {
		// right road side
		car_data->surface_type = roadConstantB & 0xf;
		aux = tnfs_collision_car_size(car_data, fence_angle);
		aux = roadRightFence * 0x2000 - aux;
		if (distance > aux) {
			fenceSide = 0x100;
			rebound_speed_x = distance + 0x8000 - aux;
			road_flag = roadConstantA & 0xf;
		}
	} else {
		car_data->surface_type = 0;
	}
	if (fenceSide == 0) {
		return;
	}

	// impact bounce off
	rebound_speed_x = fixmul(fenceSide, abs(rebound_speed_x));
	rebound_speed_z = 0;

	// reposition the car back off the fence
	math_rotate_2d(rebound_speed_x, 0, fence_angle, &rebound_x, &rebound_z);
	car_data->position.x = car_data->position.x - rebound_x;
	car_data->position.z = car_data->position.z + rebound_z;

	// change speed direction
	math_rotate_2d(-car_data->speed_x, car_data->speed_z, road_segment_heading * -0x400, &rebound_speed_x, &rebound_speed_z);

	abs_speed = abs(rebound_speed_x);
	if ((road_flag == 0) && (abs_speed >= 0x60001)) {
		if (sound_flag == 0) {
			if (car_data->unknown_flag_475 == 0) {
				if (DAT_80111a40 != 0) {
					if (distance <= 0)
						local_angle = 0x280000;
					else
						local_angle = 0xd70000;
					local_length = 1;
				} else {
					tnfs_physics_car_vector(car_data, &local_angle, &local_length);
				}
			}
		} else {
			if (car_data->unknown_flag_475 <= 0) {
				local_angle = 0x400000;
			} else {
				local_angle = 0xc00000;
			}
		}

		//play collision sound
		tnfs_sfx_play(-1, 2, 9, abs_speed, local_length, local_angle);
		if (abs_speed > 0x140000) {
			tnfs_debug_000502AB(0x32);
		}
	}

	// limit collision speed
	if (abs_speed > 0x180000) {
		if (rebound_speed_x > +0x20000)
			rebound_speed_x = +0x20000;
		if (rebound_speed_x < -0x20000)
			rebound_speed_x = -0x20000;
	} else {
		rebound_speed_x = fix2(rebound_speed_x);
	}
	if (rebound_speed_z < -0x30000) {
		if ((DAT_8010d1c4 & 0x20) == 0) {
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
		if ((DAT_8010d1c4 & 0x20) == 0) {
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
		fence_angle = fence_angle + 0x20000; //+180
		car_data->collision_x = -re_speed;
	} else {
		fence_angle = fence_angle - 0x20000; //-180
		car_data->collision_x = re_speed;
	}

	car_data->collision_y = -re_speed;
	car_data->collision_a = 0x1e;
	car_data->collision_b = 0x1e;

	// reflect car speeds 180deg
	//FIXME (fence_angle * 0x400) for correct angle scale
	math_rotate_2d(rebound_speed_x, rebound_speed_z, fence_angle * 0x400, &car_data->speed_x, &car_data->speed_z);

	// ???
	if (road_flag == 0) {
		local_angle = 0x30000;
		road_flag = 0;
	} else {
		local_angle = 0x18000;
	}
	tnfs_collision_rotate(car_data, fence_angle, abs_speed, fenceSide, road_flag, local_angle);
	if (car_data->speed_y > 0) {
		car_data->speed_y = 0;
	}
}

void tnfs_tire_forces_locked(int *force_lat, int *force_lon, signed int max_grip, int *slide) {
	signed int v5;
	int v6;
	int v7;
	int v8;
	signed int v10;

	// total force
	v7 = abs(*force_lat);
	v8 = abs(*force_lon);
	if (v7 <= v8)
		v10 = (v7 >> 2) + v8;
	else
		v10 = (v8 >> 2) + v7;

	// calculates resulting forces
	if (v10 > max_grip) {
		v5 = max_grip - fix3(3 * max_grip);
		v6 = fix8(v10);
		*slide = v10 - v5;

		v5 = v5 / v6;
		*force_lat = fix8(v5 * *force_lat);
		*force_lon = fix8(v5 * *force_lon);
	}
}

void tnfs_road_surface_modifier(tnfs_car_data *car_data) {
	int v4;
	int v5;
	signed int v7;
	signed int v8;
	signed int v9;
	signed int v10;
	int iVar3;

	//v10 = *(dword_12DECC + 36 * (dword_1328E4 & car_data->road_segment_a) + 18) >> 16 << 10;
	v10 = road_segment_slope * 0x400;
	if (v10 > 0x800000)
		v10 -= 0x1000000;

	//if (dword_1328DC) {
	//	v9 = *(dword_12DECC + 36 * (dword_1328E4 & dword_1328E4 & (car_data->road_segment_a + 1)) + 18) >> 16 << 10;
	//} else {
	//	v9 = *(dword_12DECC + 36 * (dword_1328E4 & (car_data->road_segment_a + 1)) + 18) >> 16 << 10;
	//}
	v9 = road_segment_slope * 0x400;
	if (v9 > 0x800000)
		v9 -= 0x1000000;

	iVar3 = math_sin((v9 - v10) >> 14);
	iVar3 = math_mul(iVar3, car_data->speed_local_lon);
	v8 = iVar3 * 4;

	if (v8 > 0)
		v8 += v8 >> 1;

	v7 = car_data->angle_z;
	if (v7 > 0x800000)
		v7 -= 0x1000000;

	v4 = abs(v7);
	v5 = abs(car_data->speed_local_lon);
	car_data->road_grip_increment = 2 * (fixmul(v5, v4) >> 7) + v8;

	car_data->tire_grip_front += car_data->road_grip_increment;
	if (car_data->tire_grip_front < 0)
		car_data->tire_grip_front = 0;

	car_data->tire_grip_rear += car_data->road_grip_increment;
	if (car_data->tire_grip_rear < 0)
		car_data->tire_grip_rear = 0;
}

/*
 read grip table for a given slip angle
 original array from car_specs offset 884, table size 2 x 512 bytes
 */
// slide table - default values for front engine RWD cars
unsigned int slide_table[64] = {
// front values
		2, 14, 26, 37, 51, 65, 101, 111, 120, 126, 140, 150, 160, 165, 174, 176, 176, 176, 176, 176, 176, 177, 177, 177, 200, 200, 200, 174, 167, 161, 152, 142,
		// rear values
		174, 174, 174, 174, 175, 176, 177, 177, 200, 200, 200, 200, 177, 177, 177, 177, 177, 177, 177, 177, 177, 177, 177, 177, 200, 176, 173, 170, 165, 162, 156, 153 };
/* 3C78B */
int tnfs_tire_slide_table(tnfs_car_data *a1, unsigned int slip_angle, int is_rear_wheels) {
	signed int v4;

	v4 = slip_angle;
	if (slip_angle > 2097150)
		v4 = 2097150;

	//original
	//return *(a1->car_specs_ptr + (is_rear_wheels << 9) + (v4 >> 12) + 884) << 9;
	//smaller array
	return slide_table[(is_rear_wheels << 5) + (v4 >> 16)] << 9;
}

void tnfs_tire_limit_max_grip(tnfs_car_data *car_data, //
		int *force_lat, signed int *force_lon, signed int max_grip, int *slide) {

	int f_lat_abs;
	signed int f_lon_abs;
	signed int v9;
	int v10;
	int v11;
	signed int v12;
	int f_lon_abs2;
	int f_total;

	v9 = max_grip;

	// total force
	f_lat_abs = abs(*force_lat);
	f_lon_abs = abs(*force_lon);
	f_lon_abs2 = f_lon_abs;
	if (f_lat_abs <= f_lon_abs)
		f_total = (f_lat_abs >> 2) + f_lon_abs;
	else
		f_total = (f_lon_abs >> 2) + f_lat_abs;

	if (car_data->tcs_on) {
		if (f_total > max_grip) {
			if (f_lon_abs <= f_total - max_grip) {
				*force_lon = 0;
				f_lon_abs2 = 0;
			} else {
				f_lon_abs2 = f_lon_abs - (f_total - max_grip);
				if (*force_lon <= 0)
					*force_lon = -f_lon_abs2;
				else
					*force_lon = f_lon_abs2;
			}
		}
		if (f_lat_abs <= f_lon_abs2)
			f_total = (f_lat_abs >> 2) + f_lon_abs2;
		else
			f_total = (f_lon_abs2 >> 2) + f_lat_abs;
	}

	if (f_total > max_grip) {
		// decrease slide
		*slide = f_total - max_grip;

		v10 = f_total;
		if (f_total > fix4(18 * max_grip))
			v10 = fix4(18 * max_grip);

		v12 = (v10 - max_grip);

		if (v10 - max_grip > 0)
			v9 = max_grip - v12;

		v11 = v9 / fix8(f_total);

		if ((car_data->brake <= 150 || car_data->abs_enabled) && car_data->handbrake == 0) {
			// not locked wheels
			if (*force_lat > max_grip)
				*force_lat = max_grip;
			else if (*force_lat < -max_grip)
				*force_lat = -max_grip;

			if (car_data->throttle >= 50) {
				*force_lon = fix8(v11 * *force_lon);

			} else {
				if (*force_lon > max_grip)
					*force_lon = max_grip;
				else if (*force_lon < -max_grip)
					*force_lon = -max_grip;
			}
		} else {
			// locked wheels
			*force_lat = fix8(v11 * *force_lat);
			*force_lon = fix8(v11 * *force_lon);
			// ANGLE WARNING!!
			printf("ANGLE WARNING!! %s=%d\n", "a", f_lat_abs);
		}
	}

	if (&car_data->slide_rear == slide) {
		if ((*force_lon > 0 && car_data->speed_local_lon > 0) || (*force_lon < 0 && car_data->speed_local_lon < 0)) {
			car_data->slide = v12;
		}
	}
}

void tnfs_tire_forces(tnfs_car_data *_car_data, //
		int *_result_Lat, int *_result_Lon, //
		int force_Lat, int force_Lon, //
		signed int steering, int thrust_force, int braking_force, //
		int is_front_wheels) {

	unsigned int slip_angle;
	int v11;
	int v14;
	int v15;
	int f_lat_loc_abs2;
	int f_lon_loc_abs2;
	int v23;
	int v24;
	int force_lat_local_abs;
	struct tnfs_car_data *car_data;
	int *slide;
	char *skid;
	signed int max_grip;
	int v34;
	int force_lon_local;
	int force_lat_local;
	int result_brake_thrust;
	int v38;
	struct tnfs_car_specs *car_specs;

	car_data = _car_data;
	car_specs = car_data->car_specs_ptr;

	v34 = 0;
	if (_car_data->wheels_on_ground == 0) {
		*_result_Lon = 0;
		*_result_Lat = 0;
		return;
	}

	if (is_front_wheels == 1) {
		math_rotate_2d(force_Lat, force_Lon, -steering, &force_lat_local, &force_lon_local);
		max_grip = car_data->tire_grip_front;
		skid = &car_data->tire_skid_front;
		slide = &car_data->slide_front;
	} else {
		force_lat_local = force_Lat;
		force_lon_local = force_Lon;
		max_grip = _car_data->tire_grip_rear;
		skid = &_car_data->tire_skid_rear;
		slide = &_car_data->slide_rear;
	}
	*skid = 0;
	debug_sum = *_result_Lon + *_result_Lat;

	force_lat_local_abs = abs(force_lat_local);
	if (force_lon_local <= 0)
		slip_angle = math_atan2(-force_lon_local, force_lat_local_abs);
	else
		slip_angle = math_atan2(force_lon_local, force_lat_local_abs);

	if (slip_angle > car_specs->max_slip_angle) {
		slip_angle = car_specs->max_slip_angle;
		*skid |= 1u;
	}

	if ( ( car_data->brake <= 245 || car_data->abs_enabled ) //
		&& ( fix2(5 * max_grip) >= abs(braking_force) //
		|| (car_data->abs_enabled != 0 && car_data->handbrake == 0))) {
		// not locked wheels

		// lateral tire grip factor
		v11 = tnfs_tire_slide_table(car_data, slip_angle, is_front_wheels - 1);

		v38 = fixmul(max_grip, v11);
		v24 = abs(force_lat_local);

		if (v24 > v38) {
			//lateral force exceeding max grip
			v23 = 8 * v38;

			// visual effects
			if (abs(force_lat_local) > v23 && slip_angle > 0xf0000) {
				if (slip_angle <= 1966080) {
					v34 = fix2(abs(force_lat_local) - v23);
					*skid |= 4u;
				} else {
					*skid |= 1u;
					v34 = abs(force_lat_local) - v23;
				}
			}

			if (force_lat_local <= 0)
				force_lat_local = -v38;
			else
				force_lat_local = v38;

		}

		if (force_lon_local <= 0)
			braking_force = -braking_force;

		result_brake_thrust = braking_force + thrust_force;

		if (abs(thrust_force) > abs(braking_force)) {
			//acceleration
			force_lon_local = result_brake_thrust;
			f_lon_loc_abs2 = abs(force_lon_local);
			f_lat_loc_abs2 = abs(force_lat_local);
			if (f_lat_loc_abs2 + f_lon_loc_abs2 > max_grip) {
				*skid = 2;
				if (f_lon_loc_abs2 > max_grip && is_drifting && !car_data->tcs_enabled) {
					force_lat_local = fix4(force_lat_local);
				}
			}
			tnfs_tire_limit_max_grip(car_data, &force_lat_local, &force_lon_local, max_grip, slide);
			if (*slide != 0) {
				*slide *= 8;
			}
			if (*slide < v34) {
				*slide = v34;
			}

		} else {
			//deceleration
			v15 = abs(result_brake_thrust);
			v14 = abs(force_lon_local);
			if (v15 < v14) {
				if (force_lon_local <= 0) {
					force_lon_local = -v15;
				} else {
					force_lon_local = v15;
				}
			}
			tnfs_tire_limit_max_grip(car_data, &force_lat_local, &force_lon_local, max_grip, slide);
			*slide |= v34;
			if (car_data->abs_on)
				*slide = 0;
		}

	} else {
		// locked wheels: hard braking or handbrake
		if (abs(force_lon_local) > max_grip)
			*skid |= 1u;

		tnfs_tire_forces_locked(&force_lat_local, &force_lon_local, max_grip, slide);
	}


	if (*slide > 9830400)
		*slide = 9830400;

	// function return, rotate back to car frame
	if (steering != 0) {
		math_rotate_2d(force_lat_local, force_lon_local, steering, _result_Lat, _result_Lon);
	} else {
		*_result_Lat = force_lat_local;
		*_result_Lon = force_lon_local;
	}

}

/* 
 * Main physics routine
 */
void tnfs_physics_main(tnfs_car_data *a1) {
	signed int v2;
	int v3;
	signed int v4;
	int v5;
	signed int v6;
	signed int v7;
	signed int v8;
	signed int v9;
	int v14;
	signed int v17;
	signed int v18;
	int v19;
	signed int v20;
	int v24;
	int v25;
	signed int v26;
	signed int v27;
	int v33;
	int force_lon_total;
	int limit_tire_force;
	int v43;
	signed int v44;
	signed int v45;
	tnfs_car_data *car_data;
	int rLon;
	int rLat;
	int fLon;
	int fLat;
	int *v51;
	int braking_rear;
	int braking_front;
	int traction_rear;
	int traction_front;
	int drag_lon;
	int drag_lat;
	signed int thrust_force;
	signed int v59;
	int steering;
	int v61;
	int force_Lon;
	int force_lon_adj;
	int force_Lat;
	int f_R_Lon;
	int f_R_Lat;
	int f_F_Lon;
	int f_F_Lat;
	int sLon;
	int sLat;
	int v71;
	int v72;
	tnfs_car_specs *car_specs;
	int body_roll_sine;
	int iVar2;

	// gather basic car data
	car_data = a1;

	//v51 = &dword_144728[116 * a1->dword475];
	v51 = &unknown_stats_array[0];

	is_drifting = 0;
	car_specs = a1->car_specs_ptr;

	// fast vec2 length
	v44 = abs(a1->speed_x);
	v45 = abs(a1->speed_z);
	if (v44 <= v45)
		a1->speed = (v44 >> 2) + v45;
	else
		a1->speed = (v45 >> 2) + v44;

	// framerate fixed values
	a1->scale_a = 2184;
	a1->scale_b = 30;

	// custom scales/framerate speeds, not using
	if (DAT_8010d1cc > 1 && DAT_8010d2f4 != 0) {
		//v43 = (*(dword_132F84[1 - a1->dword475] + 76) - a1->road_segment_b) / 2;
		v43 = 0;
		if (v43 > 0) {
			if (v43 > DAT_8010d2f4)
				v43 = DAT_8010d2f4;
			a1->scale_b = 30 - v43;
			a1->scale_a = 0x10000 / a1->scale_b;
		}
	}

	// TCS/ABS controls
	if ((general_flags & 0x10) != 0 && a1->handbrake == 0) {
		a1->abs_on = a1->abs_enabled;
		a1->tcs_on = a1->tcs_enabled;
	} else {
		a1->abs_on = 0;
		a1->tcs_on = 0;
	}

	// gear shift control
	if ((general_flags & 4) || a1->gear_speed == -1) {
		tnfs_engine_rev_limiter(a1);
		if (car_data->gear_RND > 0) {
			switch (car_data->gear_RND) {
			case 3:
				tnfs_engine_auto_shift(car_data);
				break;
			case 2:
				car_data->gear_speed = -1;
				break;
			case 1:
				car_data->gear_speed = -2;
				break;
			}
		}
	}
	car_data->thrust = tnfs_engine_thrust(car_data);
	thrust_force = car_data->thrust;

	if (car_data->tcs_on != 0 && car_data->gear_speed == 0)
		thrust_force -= fix2(thrust_force);

	car_data->slide = 0;

	// traction forces
	traction_front = fixmul(car_specs->front_drive_percentage, thrust_force);
	traction_rear = thrust_force - traction_front;

	// braking forces
	if (car_data->brake <= 240) {
		v2 = fixmul(330 * car_data->brake, car_data->tire_grip_rear + car_data->tire_grip_front);
		braking_front = fixmul(car_specs->front_brake_percentage, v2);
		braking_rear = (v2 - braking_front);
	} else {
		// hard braking
		braking_front = 0x140000;
		braking_rear = 0x140000;
	}
	// handbrake
	if (car_data->handbrake == 1) {
		traction_rear = 0;
		braking_rear = 0x280000;
		tnfs_cheat_crash_cars();
	}

	// gear wheel lock
	if (abs(car_data->speed_local_lat) < 0x1999 //
	&& ((car_data->speed_local_lon > 0x10000 && car_data->gear_speed == -2) //
	|| (car_data->speed_local_lon < -0x10000 && car_data->gear_speed >= 0))) {
		if (traction_front != 0 && traction_front < car_data->tire_grip_front) {
			traction_front = 0;
			braking_front = car_specs->max_brake_force_1;
		}
		if (traction_rear != 0 && traction_rear < car_data->tire_grip_rear) {
			traction_rear = 0;
			braking_rear = car_specs->max_brake_force_1;
		}
	}

	// gear change delay
	if (car_data->throttle >= 40) {
		if (car_data->tcs_on != 0) {
			if (abs(thrust_force) > 0x70000 && car_data->throttle > 83)
				car_data->throttle -= 12;
		}
	} else {
		car_data->tcs_on = 0;
	}
	if (car_data->brake < 40)
		car_data->abs_on = 0;
	if (car_data->gear_speed != car_data->gear_speed_selected) {
		car_data->gear_shift_interval = 16;
		car_data->gear_speed_previous = car_data->gear_speed_selected;
		car_data->gear_speed_selected = car_data->gear_speed;
	}
	if (car_data->gear_shift_interval > 0) {
		if (car_data->gear_shift_interval > 12 || car_data->is_shifting_gears < 1)
			--car_data->gear_shift_interval;

		if (car_data->unknown_flag_475 == DAT_8010d30c //
		&& car_data->gear_shift_interval == 11 //
		&& selected_camera == 0 // dashboard view
		&& car_data->is_shifting_gears < 1) {
			//play gear shift sound
			tnfs_sfx_play(-1, 13, 0, 0, 1, 15728640);
		}
	}

	// aero/drag forces
	drag_lat = tnfs_drag_force(car_data, car_data->speed_local_lat);
	drag_lon = tnfs_drag_force(car_data, car_data->speed_local_lon);
	if (car_data->speed_local_lon > car_specs->max_speed && selected_track != 6) {
		if (drag_lon > 0 && drag_lon < thrust_force) {
			drag_lon = abs(thrust_force);
		} else if (drag_lon < 0) {
			if (abs(drag_lon) < abs(thrust_force)) {
				drag_lon = -abs(thrust_force);
			}
		}
	}

	// BEGIN of car traction/slip trajectory
	v72 = math_angle_wrap(car_data->steer_angle + car_data->angle_y);

	// convert to local frame of reference
	math_rotate_2d(car_data->speed_x, car_data->speed_z, -car_data->angle_y, &car_data->speed_local_lat, &car_data->speed_local_lon);

	// scale speeds
	sLat = car_data->scale_b * car_data->speed_local_lat;
	sLon = car_data->scale_b * car_data->speed_local_lon;

	// sideslip
	v3 = fixmul(car_data->wheel_base, car_data->scale_b * car_data->angular_speed);

	// front and rear moment of inertia, inverted
	fLat = -fixmul(car_data->weight_distribution, sLat) - (drag_lat / 2) + v3;
	rLat = -(sLat - fixmul(car_data->weight_distribution, sLat)) - (drag_lat / 2) - v3;
	rLon = -(sLon - fixmul(car_data->weight_distribution, sLon)) - (drag_lon / 2);
	fLon = -fixmul(car_data->weight_distribution, sLon) - (drag_lon / 2);

	// adjust steer sensitivity
	v59 = car_data->steer_angle;
	if (car_data->brake <= 200) {
		v4 = fix8(car_data->speed_local_lon) * v59;
		steering = v59 - fix15(v4);
	} else {
		steering = v59 / 2;
	}

	// tire forces (bicycle model)
	car_data->slide_front = 0;
	car_data->slide_rear = 0;

	tnfs_tire_forces(car_data, &f_F_Lat, &f_F_Lon, fLat, fLon, steering, traction_front, braking_front, 1);
	tnfs_tire_forces(car_data, &f_R_Lat, &f_R_Lon, rLat, rLon, 0, traction_rear, braking_rear, 2);

	force_Lat = f_R_Lat + f_F_Lat;
	force_Lon = f_R_Lon + f_F_Lon;

	// limit braking forces
	if (car_data->brake > 100 || car_data->handbrake) {
		if (car_data->speed >= 1755054) {
			if (car_data->speed >= 2621440) {
				if (car_specs->max_brake_force_2 <= car_specs->max_brake_force_1) {
					limit_tire_force = car_specs->max_brake_force_1;
				} else {
					limit_tire_force = car_specs->max_brake_force_2;
				}
			} else {
				limit_tire_force = car_specs->max_brake_force_2;
			}
		} else {
			limit_tire_force = car_specs->max_brake_force_1;
		}
		if (force_Lon <= 0) {
			force_lon_total = -force_Lon;
		} else {
			force_lon_total = f_R_Lon + f_F_Lon;
		}
		if (force_lon_total > limit_tire_force) {
			if (force_Lon < 0) {
				force_Lon = -limit_tire_force;
			} else {
				force_Lon = limit_tire_force;
			}
		}
		if (selected_track == 3 && (car_data->road_segment_a <= 97 || car_data->road_segment_a >= 465)) {
			v33 = force_Lat <= 0 ? -force_Lat : f_R_Lat + f_F_Lat;
			if (v33 > limit_tire_force) {
				if (force_Lat < 0) {
					force_Lat = -limit_tire_force;
				} else {
					force_Lat = limit_tire_force;
				}
			}
		}
	}

	// tire lateral force limit
	v61 = car_data->road_grip_increment + car_specs->max_tire_lateral_force;
	if (v61 < 0)
		v61 = 0;
	if (abs(force_Lat) > v61) {
		if (force_Lat < 0)
			force_Lat = -v61;
		else
			force_Lat = v61;
	}

	// calculate grip forces
	v5 = fixmul(force_Lon, car_data->weight_transfer_factor);

	v6 = (car_data->front_friction_factor - v5) * road_surface_type_array[4 * car_data->surface_type];
	car_data->tire_grip_front = fix8(v6);

	v7 = (v5 + car_data->rear_friction_factor) * road_surface_type_array[4 * car_data->surface_type];
	car_data->tire_grip_rear = fix8(v7);

	tnfs_road_surface_modifier(car_data);

	// thrust force to acc (force/mass=acc?)
	v8 = car_specs->thrust_to_acc_factor * force_Lon;
	force_lon_adj = fix8(v8);

	// suspension inclination
	car_data->susp_incl_lat = force_Lat;
	car_data->susp_incl_lon = force_lon_adj;

	// convert speeds to world scale (m/s >> 16)
	if (abs(car_data->speed_local_lon) + abs(car_data->speed_local_lat) >= 19660) {
		// car moving
		car_data->speed_local_lat += fixmul(car_data->scale_a, (drag_lat + force_Lat + car_data->slope_force_lat));
		car_data->speed_local_lon += fixmul(car_data->scale_a, (drag_lon + force_lon_adj + car_data->slope_force_lon));
	} else {
		// car stopped
		car_data->speed_local_lat += fixmul(car_data->scale_a, force_Lat);
		car_data->speed_local_lon += fixmul(car_data->scale_a, force_lon_adj);

		if (car_data->gear_speed == -1 || car_data->throttle == 0) {
			car_data->speed_local_lon = 0;
			car_data->speed_local_lat = 0;
		}
	}

	// rotate back to global frame of reference
	math_rotate_2d(car_data->speed_local_lat, car_data->speed_local_lon, car_data->angle_y, &car_data->speed_x, &car_data->speed_z);

	// move the car
	car_data->position.z += fixdiv(car_data->speed_z, car_data->scale_a);
	car_data->position.x -= fixdiv(car_data->speed_x, car_data->scale_a);

	// suspension body roll
	if (car_data->speed_local_lat + car_data->speed_local_lon <= 6553) {
		car_data->body_roll += -car_data->body_roll / 2;
		car_data->body_pitch += -car_data->body_pitch / 2;
	} else {
		car_data->body_roll += (-(fixmul(car_data->susp_incl_lat, car_specs->body_roll_factor)) - car_data->body_roll) / 2;
		car_data->body_pitch += (-(fixmul(car_data->susp_incl_lon, car_specs->body_pitch_factor)) - car_data->body_pitch) / 2;
	}

	// apply body rotation torque
	v71 = fixmul(car_data->rear_yaw_factor, f_R_Lat) - fixmul(car_data->front_yaw_factor, f_F_Lat);
	car_data->angular_speed += fixmul(car_data->scale_a, v71);

	if (abs(car_data->angular_speed) > 9830400) {
		if (car_data->angular_speed >= 0)
			car_data->angular_speed = 9830400;
		else
			car_data->angular_speed = -9830400;
	}

	// rotate car body
	v9 = fixmul(car_data->scale_a, car_data->angular_speed);
	car_data->angle_y += fix2(v9) + v9;

	// further car turning due to road bank angle
	if (abs(car_data->angle_z) > 0x30000) {

		////v27 = *(dword_12DECC + 36 * (dword_1328E4 & car_data->road_segment_a) + 22) >> 16 << 10;
		v27 = road_segment_heading * 0x400;
		if (v27 > 0x800000)
			v27 -= 0x1000000;

		//if (dword_1328DC) {
		//	v26 = *(dword_12DECC + 36 * (dword_1328E4 & dword_1328E4 & (car_data->road_segment_a + 1)) + 22) >> 16 << 10;
		//} else {
		//	v26 = *(dword_12DECC + 36 * (dword_1328E4 & (car_data->road_segment_a + 1)) + 22) >> 16 << 10;
		//}
		v26 = road_segment_heading * 0x400; //FIXME actually next segment
		if (v26 > 0x800000)
			v26 -= 0x1000000;

		body_roll_sine = math_sin(car_data->angle_z >> 14);
		iVar2 = fix8(car_data->speed_local_lon) * (v26 - v27);
		iVar2 = math_mul(body_roll_sine, fix8(iVar2));
		car_data->angle_y -= fix7(iVar2);
	}

	// wrap angle
	car_data->angle_y = math_angle_wrap(car_data->angle_y);

	debug_sum = car_data->speed_local_lon + car_data->speed_local_vert + car_data->speed_local_lat;

	// track fence collision
	tnfs_track_fence_collision(car_data);


	// used for debug
	if ((general_flags & 4) && is_not_in_replay == 1) {
		if (car_data->speed_local_lon / 2 <= 0)
			v24 = car_data->speed_local_lon / -2;
		else
			v24 = car_data->speed_local_lon / 2;
		if (car_data->speed_local_lat <= 0)
			v25 = -car_data->speed_local_lat;
		else
			v25 = car_data->speed_local_lat;
		if (v25 > v24 && car_data->speed_local_lat > 0x8000)
			tnfs_debug_000502AB(87);
		if ((car_data->slide_front || car_data->slide_rear) //
		&& car_data->speed > 0x140000 //
		&& ((car_data->tire_skid_rear & 1) || (car_data->tire_skid_front & 1))) {
			tnfs_debug_000502AB(40);
		}
		if (!dword_132EFC) {
			if (abs(car_data->speed_local_lon) > 3276) {
				tnfs_debug_000502AB(120);
				dword_132EFC = dword_122CAC + 1;
			}
		}
	}

	if (v51[113] < abs(car_data->speed_local_lon)) {
		v51[113] = abs(car_data->speed_local_lon);
	}

	// used for debug - Rusty Springs track
	if (!dword_146483 && selected_track == 3) {
		if (car_data->road_segment_a <= 97 || car_data->road_segment_a >= 465) {
			if (car_data->speed_local_lon < 13107) {
				dword_D8AFC = dword_122CAC;
				dword_D8B00 = car_data->road_segment_b;
				dword_D8AF0 = 99999;
				dword_D8AF4 = 99999;
				dword_D8AF8 = 99999;
			}
			v19 = dword_122CAC - dword_D8AFC;
			if (car_data->throttle > 50 && v19 < 1500 && v19 > 100) {
				if (car_data->speed_local_lon > 1755447 && dword_D8AF0 > v19) {
					dword_D8AF0 = dword_122CAC - dword_D8AFC;
					tnfs_debug_00034309(v19, 0, 0, 0);
					if (v51[102] > v19)
						v51[102] = v19;
				}
				if (car_data->speed_local_lon > 2926182 && dword_D8AF4 > v19) {
					dword_D8AF4 = v19;
					tnfs_debug_00034309(0, v19, 0, 0);
					if (v51[103] > v19)
						v51[103] = v19;
				}
			}
			if (car_data->road_segment_b - dword_D8B00 > 83) {
				//TODO dword_D8B00 = &unk_F423F;
				v20 = dword_122CAC - dword_D8AFC;
				if (dword_D8AF8 > dword_122CAC - dword_D8AFC && v20 < 1000) {
					if (v20 < v51[107]) {
						v51[107] = v20;
						v51[106] = car_data->speed_local_lon;
					}
					tnfs_debug_00034309(0, 0, v20, car_data->speed_local_lon);
					dword_D8AF8 = v20;
				}
			}
			if (car_data->brake > 50) {
				if (car_data->speed_local_lon > 2340290)
					dword_D8AE8 = dword_122CAC;
				if (car_data->speed_local_lon > 1755447)
					dword_D8AEC = dword_122CAC;
				if (car_data->speed_local_lon < 6553 && dword_D8AEC > 0) {
					v17 = dword_122CAC - dword_D8AEC;

					printf("TUNING STATS : 60-0 in seconds %d = feet %d", 100 * (dword_122CAC - dword_D8AEC) / 60, v17);
					tnfs_debug_000343C2(v17, 0);
					dword_D8AEC = 0;
					if (v51[105] > v17)
						v51[105] = v17;
				}
				if (car_data->speed_local_lon < 6553 && dword_D8AE8 > 0) {
					v18 = dword_122CAC - dword_D8AE8;

					printf("TUNING STATS : 80-0 in %d seconds = %d feet", 100 * (dword_122CAC - dword_D8AE8) / 60, v18);
					tnfs_debug_000343C2(0, v18);
					dword_D8AE8 = 0;
					if (v51[104] > v18)
						v51[104] = v18;
				}
			}
		}
		if (car_data->tire_skid_rear) {
			v14 = car_data->tire_skid_rear;
			if (abs(car_data->angular_speed) > 3276800)
				tnfs_debug_000502AB(87);
		}
	}

	debug_sum = car_data->position.z + car_data->position.y + car_data->position.x;
}
