/*
 * The "Fiziks" module
 * 2D car handling dynamics
 */
#include "tnfs_math.h"
#include "tnfs_base.h"
#include "tnfs_collision_2d.h"
#include "tnfs_collision_3d.h"
#include "tnfs_engine.h"

// settings/flags
int is_recording_replay = 1;
int general_flags = 0x14; //800dc5a0 DAT_0014383c
int debug_sum = 0;
int selected_track = 3; //rusty springs
int g_live_player_glue = 0; //8010d2f4
int DAT_0016707c = 0; //8010d30c 0016707c

int DAT_8010c478 = 0; //DAT_00144f10

// debug stats flags
int stats_braking_init_time = 0;
int stats_braking_final_time = 0;
int stats_timer_a = 0;
int stats_timer_b = 0;
int stats_timer_c = 0;
int stats_init_time = 0;
int stats_init_track_slice = 0;
int DAT_800f62c0 = 0;
int is_performance_test_off = 0; //DAT_00146483

// PSX rally mode data
int DAT_800eb204, DAT_800eb208, DAT_800eb210, DAT_800eb20c;

void tnfs_record_best_acceleration(int a, int b, int c, int s) {
	if (a > 0)
		printf("Best 0-60 acceleration: %.2f\n", ((float) a) / 30);
	else if (b > 0)
		printf("Best 0-80 acceleration: %.2f\n", ((float) b) / 30);
	else if (c > 0)
		printf("Quarter mile time: %.2f (%d m/s)\n", (((float) c) / 30), (s >> 16));
}

void tnfs_record_best_braking(int a, int b) {
	if (a > 0)
		printf("\nBest 60-0 braking: %.2f\n", ((float) a) / 30);
	else if (b > 0)
		printf("\nBest 80-0 braking: %.2f\n", ((float) b) / 30);

}

int tnfs_drag_force(tnfs_car_data *car, signed int speed, char longitudinal) {
	int max;
	int incl_angle;
	int drag;
	int sq_speed;

	if (road_surface_type_array[car->surface_type].is_unpaved)
		car->surface_type_b = 4;
	else
		car->surface_type_b = 0;

	sq_speed = speed >> 16;
	sq_speed *= sq_speed;

	drag = fix8(road_surface_type_array[car->surface_type].velocity_drag * car->car_specs_ptr->drag) * sq_speed;

	//only PSX
	if (longitudinal) {
		drag += (car->drag_const_0x4ac + car->drag_const_0x4ae) * sq_speed;
	}

	drag += road_surface_type_array[car->surface_type].surface_drag;

	incl_angle = car->angle.z;
	if (incl_angle > 0x800000)
		incl_angle -= 0x1000000;

	if (selected_track > 2 && abs(incl_angle) > 0x9FFFF)
		drag = fix3(drag);

	max = abs(speed * car->fps);
	if (drag > max)
		drag = max;

	if (speed > 0)
		drag = -drag;

	return drag;
}

/*
 * crashing cars cheat code, from 3DO version
 */
void tnfs_cheat_crash_cars() {
	int i;
	if (cheat_crashing_cars == 4) {
		for (i = 1; i < g_total_cars_in_scene; i++) {
			tnfs_collision_rollover_start(g_car_ptr_array[i], 0xa0000, 0xa0000, 0xa0000);
		}
	}
}

void tnfs_tire_forces_locked(int *force_lat, int *force_lon, signed int max_grip, int *slide) {
	signed int residual;
	int abs_f_lon;
	int abs_f_lat;
	signed int force;

	// total force
	abs_f_lon = abs(*force_lon);
	abs_f_lat = abs(*force_lat);
	if (abs_f_lon <= abs_f_lat)
		force = (abs_f_lon >> 2) + abs_f_lat;
	else
		force = (abs_f_lat >> 2) + abs_f_lon;

	// calculate resulting forces
	if (force > max_grip) {
		residual = max_grip - fix3(3 * max_grip);
		*slide = force - residual;

		residual /= fix8(force);
		*force_lat = fix8(residual * *force_lat);
		*force_lon = fix8(residual * *force_lon);
	}
}

void tnfs_road_surface_modifier(tnfs_car_data *car_data) {
	int angle;
	int slope_r;
	int slope_1;
	int slope_0;

	int grip;
	int grip1;

	slope_0 = track_data[car_data->track_slice & g_slice_mask].slope * 0x400;
	if (slope_0 > 0x800000)
		slope_0 -= 0x1000000;

	slope_1 = track_data[(car_data->track_slice + 1)  & g_slice_mask].slope * 0x400;
	if (slope_1 > 0x800000)
		slope_1 -= 0x1000000;

	slope_r = math_mul(math_sin_3((slope_1 - slope_0) >> 14), car_data->speed_local_lon) * 4;
	if (slope_r > 0)
		slope_r += slope_r >> 1;

	angle = car_data->angle.z;
	if (angle > 0x800000)
		angle -= 0x1000000;

	car_data->road_grip_increment = 2 * (math_mul(abs(car_data->speed_local_lon), abs(angle)) >> 7) + slope_r;

	car_data->tire_grip_front += car_data->road_grip_increment;
	if (car_data->tire_grip_front < 0)
		car_data->tire_grip_front = 0;

	car_data->tire_grip_rear += car_data->road_grip_increment;
	if (car_data->tire_grip_rear < 0)
		car_data->tire_grip_rear = 0;

	// not exists on 3DO and PC demo
	grip = car_data->speed_local_lon >> 16;
	if (0 < car_data->speed_local_lon) {
		grip = grip * grip;
		grip1 = grip * car_data->drag_const_0x4a8;
		grip = grip * car_data->drag_const_0x4aa;
		car_data->tire_grip_front = grip1 + car_data->tire_grip_front;
		grip1 = grip1 + grip;
		car_data->tire_grip_rear = grip + car_data->tire_grip_rear;
		grip = grip1;
		car_data->road_grip_increment = fix2(grip) + fix3(grip1) + car_data->road_grip_increment;
	}
}

/*
 * read grip table for a given slip angle, 2 x 512 bytes
 */
int tnfs_tire_slide_table(tnfs_car_data *car, int slip_angle, int is_rear_wheels) {
	if (slip_angle > 0x1ffffe)
		slip_angle = 0x1ffffe;

	return car->car_specs_ptr->grip_table[(is_rear_wheels * 512) + (slip_angle >> 12)] << 9;
}

void tnfs_tire_limit_max_grip(tnfs_car_data *car_data, //
		int *force_lat, signed int *force_lon, signed int max_grip, int *slide) {

	int f_lat_abs;
	signed int f_lon_abs;
	signed int residual_grip;
	int s_grip;
	int factor;
	signed int grip_loss;
	int f_lon_abs2;
	int f_total;

	residual_grip = max_grip;
	grip_loss = 0;

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

		s_grip = f_total;
		if (f_total > fix4(18 * max_grip))
			s_grip = fix4(18 * max_grip);

		grip_loss = s_grip - max_grip;

		if (grip_loss > 0)
			residual_grip = max_grip - grip_loss;

		factor = residual_grip / fix8(f_total);

		if ((car_data->brake <= 150 || car_data->abs_enabled) && car_data->handbrake == 0) {
			// not locked wheels
			if (*force_lat > max_grip)
				*force_lat = max_grip;
			else if (*force_lat < -max_grip)
				*force_lat = -max_grip;

			if (car_data->throttle >= 50) {
				*force_lon = fix8(factor * *force_lon);

			} else {
				if (*force_lon > max_grip)
					*force_lon = max_grip;
				else if (*force_lon < -max_grip)
					*force_lon = -max_grip;
			}
		} else {
			// locked wheels
			*force_lat = fix8(factor * *force_lat);
			*force_lon = fix8(factor * *force_lon);
			// ANGLE WARNING!!
			// printf("ANGLE WARNING!! %s=%d\n", "a", f_lat_abs);
		}
	}

	if (&car_data->slide_rear == slide) {
		if ((*force_lon > 0 && car_data->speed_local_lon > 0) || (*force_lon < 0 && car_data->speed_local_lon < 0)) {
			car_data->tire_grip_loss = grip_loss;
		}
	}
}

void tnfs_tire_forces(tnfs_car_data *car, //
		int *_result_Lat, int *_result_Lon, //
		int force_Lat, int force_Lon, //
		signed int steering, int thrust_force, int braking_force, //
		int is_front_wheels) {

	unsigned int slip_angle;
	int f_lat_loc_abs2;
	int f_lon_loc_abs2;
	int grip_force;
	int *slide;
	char *skid;
	signed int max_grip;
	int skid_amount;
	int force_lon_local;
	int force_lat_local;
	int result_brake_thrust;
	int slip_angle_grip;

	skid_amount = 0;
	if (car->time_off_ground > 0) {
		*_result_Lon = 0;
		*_result_Lat = 0;
	} else {

		if (is_front_wheels == 1) {
			math_rotate_2d(force_Lat, force_Lon, -steering, &force_lat_local, &force_lon_local);
			max_grip = car->tire_grip_front;
			skid = &car->tire_skid_front;
			slide = &car->slide_front;
		} else {
			force_lat_local = force_Lat;
			force_lon_local = force_Lon;
			max_grip = car->tire_grip_rear;
			skid = &car->tire_skid_rear;
			slide = &car->slide_rear;
		}
		*skid = 0;
		debug_sum = *_result_Lon + *_result_Lat;

		slip_angle = math_atan2(abs(force_lon_local), abs(force_lat_local));

		if (slip_angle > car->car_specs_ptr->cutoff_slip_angle) {
			slip_angle = car->car_specs_ptr->cutoff_slip_angle;
			*skid |= 1;
		}

		if ( fix2(5 * max_grip) < abs(braking_force) //
			&& (car->abs_enabled == 0 || car->handbrake != 0)) {

			// locked wheels: hard braking or handbrake
			if (abs(force_lon_local) > max_grip)
				*skid |= 1u;

			tnfs_tire_forces_locked(&force_lat_local, &force_lon_local, max_grip, slide);

		} else {
			// not locked wheels

			// lateral tire grip factor
			slip_angle_grip = math_mul(max_grip, tnfs_tire_slide_table(car, slip_angle, is_front_wheels - 1));

			if (abs(force_lat_local) > slip_angle_grip) {
				//lateral force exceeding max grip
				grip_force = 8 * slip_angle_grip;

				// for skid sounds/visual effects
				if (abs(force_lat_local) > grip_force && slip_angle > 0xf0000) {
					if (slip_angle <= 0x1E0000) {
						skid_amount = fix2(abs(force_lat_local) - grip_force);
						*skid |= 4u;
					} else {
						*skid |= 1u;
						skid_amount = abs(force_lat_local) - grip_force;
					}
				}

				if (force_lat_local <= 0)
					force_lat_local = -slip_angle_grip;
				else
					force_lat_local = slip_angle_grip;

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
					if (f_lon_loc_abs2 > max_grip && is_drifting && !car->tcs_enabled) {
						force_lat_local = fix4(force_lat_local);
					}
					tnfs_tire_limit_max_grip(car, &force_lat_local, &force_lon_local, max_grip, slide);
				} else {
					tnfs_tire_limit_max_grip(car, &force_lat_local, &force_lon_local, max_grip, slide);
				}
				if (*slide != 0) {
					*slide <<= 3;
				}
				if (*slide < skid_amount) {
					*slide = skid_amount;
				}

			} else {
				//deceleration
				if (abs(result_brake_thrust) < abs(force_lon_local)) {
					if (force_lon_local <= 0) {
						force_lon_local = -abs(result_brake_thrust);
					} else {
						force_lon_local = abs(result_brake_thrust);
					}
				}
				tnfs_tire_limit_max_grip(car, &force_lat_local, &force_lon_local, max_grip, slide);
				*slide |= skid_amount;
				if (car->abs_on)
					*slide = 0;
			}
		}

		if (*slide > 0x960000)
			*slide = 0x960000;

		// function return, rotate back to car frame
		if (steering != 0) {
			math_rotate_2d(force_lat_local, force_lon_local, steering, _result_Lat, _result_Lon);
		} else {
			*_result_Lat = force_lat_local;
			*_result_Lon = force_lon_local;
		}
	}

}

/*
 * Main physics routine
 */
void tnfs_physics_update(tnfs_car_data *car_data) {
	int braking_total;
	int sideslip;
	int weight_transfer;
	int angular_accel;
	int body_rotate;
	int stats_elapsed_brake_time_1;
	int stats_elapsed_brake_time_2;
	int stats_elapsed_acc_time_1;
	int stats_elapsed_acc_time_2;
	int local_speed_lon;
	int local_speed_lat;
	int track_heading_2;
	int track_heading_1;
	int force_lon_total;
	int limit_tire_force;
	int frameskip;
	int abs_speed_x;
	int abs_speed_y;
	int speed_lon_rear;
	int speed_lat_rear;
	int speed_lon_front;
	int speed_lat_front;
	tnfs_stats_data *stats_data_ptr;
	int braking_rear;
	int braking_front;
	int traction_rear;
	int traction_front;
	int drag_lon;
	int drag_lat;
	int thrust_force;
	int steer;
	int steering;
	int max_lateral_acc;
	int force_lon;
	int accel_lon;
	int accel_lat;
	int ftire_lon_rear;
	int ftire_lat_rear;
	int ftire_lon_front;
	int ftire_lat_front;
	int speed_lon;
	int speed_lat;
	tnfs_car_specs *car_specs;
	int body_roll_sine;
	int aux;

	stats_data_ptr = &g_stats_data[car_data->car_id2];

	is_drifting = 0;
	car_specs = car_data->car_specs_ptr;

	// fast vec2 length
	abs_speed_x = abs(car_data->speed_x);
	abs_speed_y = abs(car_data->speed_z);
	if (abs_speed_x <= abs_speed_y)
		car_data->speed = (abs_speed_x >> 2) + abs_speed_y;
	else
		car_data->speed = (abs_speed_y >> 2) + abs_speed_x;

	// framerate fixed values
	car_data->delta_time = 2184;
	car_data->fps = 30;

	// custom scales/framerate speeds, not using
	if (g_number_of_players > 1 && g_live_player_glue != 0) {
		//frameskip = ((&g_car_ptr_array)[1 - car->car_id2]->track_slice_lap - car->track_slice_lap) / 2,
		frameskip = 0;
		if (frameskip > 0) {
			if (frameskip > g_live_player_glue)
				frameskip = g_live_player_glue;
			car_data->fps = 30 - frameskip;
			car_data->delta_time = 0x10000 / car_data->fps;
		}
	}

	// TCS/ABS controls
	if ((general_flags & 0x10) != 0 && car_data->handbrake == 0) {
		car_data->abs_on = car_data->abs_enabled;
		car_data->tcs_on = car_data->tcs_enabled;
	} else {
		car_data->abs_on = 0;
		car_data->tcs_on = 0;
	}

	// gear shift control
	if ((general_flags & 4) || car_data->gear_selected == -1) {
		tnfs_engine_rev_limiter(car_data);
		if (car_data->gear_auto_selected > 0) { // automatic transmission
			switch (car_data->gear_auto_selected) {
			case 3:
				tnfs_engine_auto_shift_control(car_data);
				break;
			case 2:
				car_data->gear_selected = -1;
				break;
			case 1:
				car_data->gear_selected = -2;
				break;
			}
		}
	}

	//engine thrust
	car_data->thrust = tnfs_engine_thrust(car_data);
	thrust_force = car_data->thrust;

	if (car_data->tcs_on != 0 && car_data->gear_selected == 0)
		thrust_force -= fix2(thrust_force);

	car_data->tire_grip_loss = 0;

	// traction forces
	traction_front = fixmul(car_specs->front_drive_percentage, thrust_force);
	traction_rear = thrust_force - traction_front;

	// braking forces
	if (car_data->brake > 240) {
		// hard braking
		braking_front = 0x140000;
		braking_rear = 0x140000;
	} else {
		braking_total = fixmul(330 * car_data->brake, car_data->tire_grip_rear + car_data->tire_grip_front);
		braking_front = fixmul(car_specs->front_brake_percentage, braking_total);
		braking_rear = (braking_total - braking_front);
	}

	// handbrake
	if (car_data->handbrake == 1) {
		traction_rear = 0;
		braking_rear = 0x280000;
		tnfs_cheat_crash_cars();
	}

	// gear wheel lock
	if (abs(car_data->speed_local_lat) < 0x1999 //
	&& ((car_data->speed_local_lon > 0x10000 && car_data->gear_selected == -2) //
	|| (car_data->speed_local_lon < -0x10000 && car_data->gear_selected >= 0))) {
		if (traction_front != 0 && traction_front < car_data->tire_grip_front) {
			traction_front = 0;
			braking_front = car_specs->max_brake_force_1;
		}
		if (traction_rear != 0 && traction_rear < car_data->tire_grip_rear) {
			traction_rear = 0;
			braking_rear = car_specs->max_brake_force_1;
		}
	}

	//TCS
	if (car_data->throttle < 40) {
		car_data->tcs_on = 0;
	} else {
		if (car_data->tcs_on != 0) {
			if (0x70000 <= abs(thrust_force) && car_data->throttle > 83)
				car_data->throttle -= 12;
		}
	}
	if (car_data->brake < 40)
		car_data->abs_on = 0;

	// gear change delay
	if (car_data->gear_selected != car_data->gear_shift_current) {
		car_data->gear_shift_interval = 16;
		car_data->gear_shift_previous = car_data->gear_shift_current;
		car_data->gear_shift_current = car_data->gear_selected;
	}
	if (car_data->gear_shift_interval > 0) {
		if (car_data->gear_shift_interval > 12 || car_data->is_shifting_gears < 1)
			--car_data->gear_shift_interval;

		if (car_data->car_id2 == g_player_id //
		&& car_data->gear_shift_interval == 11 //
		&& selected_camera == 0 // dashboard view
		&& car_data->is_shifting_gears < 1) {
			//play gear shift sound
			tnfs_sfx_play(-1, 13, 0, 0, 1, 0xF00000);
		}
	}

	// aero/drag forces
	drag_lat = tnfs_drag_force(car_data, car_data->speed_local_lat, 0);
	drag_lon = tnfs_drag_force(car_data, car_data->speed_local_lon, 1);

	if (car_data->speed_local_lon > car_data->car_specs_ptr->top_speed && selected_track != 6) {
		if (drag_lon > 0 && drag_lon < thrust_force) {
			drag_lon = abs(thrust_force);
		} else if (drag_lon < 0) {
			if (abs(drag_lon) < abs(thrust_force)) {
				drag_lon = -abs(thrust_force);
			}
		}
	}

	// BEGIN of car traction/slip trajectory
	math_angle_wrap(car_data->steer_angle + car_data->angle.y);

	// convert to local frame of reference
	math_rotate_2d(car_data->speed_x, car_data->speed_z, -car_data->angle.y, &car_data->speed_local_lat, &car_data->speed_local_lon);

	// time scale speeds (m/s)
	speed_lat = car_data->fps * car_data->speed_local_lat;
	speed_lon = car_data->fps * car_data->speed_local_lon;

	// split front and rear moments
	// lateral speeds, front and rear
	aux = -math_mul(car_data->weight_distribution_front, speed_lat);
	sideslip = math_mul(car_data->wheel_base, car_data->fps * car_data->angular_speed);
	speed_lat_front = aux - (drag_lat / 2) + sideslip;
	speed_lat_rear = -(speed_lat + aux) - (drag_lat / 2) - sideslip;

	// longitudinal speeds, front and rear
	aux = -math_mul(car_data->weight_distribution_front, speed_lon);
	speed_lon_rear = -(speed_lon + aux) - (drag_lon / 2);
	speed_lon_front = aux - (drag_lon / 2);

	// adjust steer sensitivity
	steer = car_data->steer_angle;
	if (car_data->brake > 200) {
		steering = steer / 2;
	} else {
		steering = steer - fix15(fix8(car_data->speed_local_lon) * steer);
	}

	// tire forces (bicycle model)
	car_data->slide_front = 0;
	car_data->slide_rear = 0;

	tnfs_tire_forces(car_data, &ftire_lat_front, &ftire_lon_front, speed_lat_front, speed_lon_front, steering, traction_front, braking_front, 1);
	tnfs_tire_forces(car_data, &ftire_lat_rear,  &ftire_lon_rear,  speed_lat_rear,  speed_lon_rear,  0,        traction_rear,  braking_rear,  2);

	accel_lat = ftire_lat_rear + ftire_lat_front;
	force_lon = ftire_lon_rear + ftire_lon_front;

	// limit braking forces
	if (car_data->brake > 100 || car_data->handbrake) {
		if (car_data->speed < 1755054) {
			limit_tire_force = car_specs->max_brake_force_1;
		} else {
			if (car_data->speed < 2621440) {
				limit_tire_force = car_specs->max_brake_force_2;
			} else {
				if (car_specs->max_brake_force_2 <= car_specs->max_brake_force_1) {
					limit_tire_force = car_specs->max_brake_force_1;
				} else {
					limit_tire_force = car_specs->max_brake_force_2;
				}
			}
		}
		force_lon_total = force_lon;
		if (force_lon < 0) {
			force_lon_total = -force_lon;
		}
		if (force_lon_total > limit_tire_force) {
			if (force_lon >= 0) {
				force_lon = limit_tire_force;
			} else {
				force_lon = -limit_tire_force;
			}
		}
		if (selected_track == 3 && (car_data->track_slice <= 97 || car_data->track_slice >= 465)) {
			if (abs(accel_lat) >= limit_tire_force) {
				if (accel_lat < 0) {
					accel_lat = -limit_tire_force;
				} else {
					accel_lat = limit_tire_force;
				}
			}
		}
	}

	// tire lateral force limit
	max_lateral_acc = car_data->road_grip_increment + car_specs->lateral_accel_cutoff;
	if (max_lateral_acc < 0)
		max_lateral_acc = 0;
	if (abs(accel_lat) > max_lateral_acc) {
		if (accel_lat < 0)
			accel_lat = -max_lateral_acc;
		else
			accel_lat = max_lateral_acc;
	}

	// calculate grip forces
	weight_transfer = fixmul(force_lon, car_data->weight_transfer_factor);

	car_data->tire_grip_front = fix8((car_data->front_friction_factor - weight_transfer) * road_surface_type_array[car_data->surface_type].roadFriction);
	car_data->tire_grip_rear = fix8((weight_transfer + car_data->rear_friction_factor) * road_surface_type_array[car_data->surface_type].roadFriction);

	tnfs_road_surface_modifier(car_data);

	// thrust force to acc (force/mass=acc)
	accel_lon = fix8((car_data->car_specs_ptr->thrust_to_acc_factor * force_lon));

	// acceleration
	car_data->accel_lat = accel_lat;
	car_data->accel_lon = accel_lon;

	// convert speeds to world scale (m/s >> 16)
	if (abs(car_data->speed_local_lon) + abs(car_data->speed_local_lat) < 19660) {
		// car stopped
		car_data->speed_local_lat += math_mul(accel_lat, car_data->delta_time);
		car_data->speed_local_lon += math_mul(accel_lon, car_data->delta_time);

		if (car_data->gear_selected == -1 || car_data->throttle == 0) {
			car_data->speed_local_lon = 0;
			car_data->speed_local_lat = 0;
		}
	} else {
		// car moving
		car_data->speed_local_lat += math_mul((drag_lat + accel_lat + car_data->slope_force_lat), car_data->delta_time);
		car_data->speed_local_lon += math_mul((drag_lon + accel_lon + car_data->slope_force_lon), car_data->delta_time);
	}

	// rotate back to global frame of reference
	math_rotate_2d(car_data->speed_local_lat, car_data->speed_local_lon, car_data->angle.y, &car_data->speed_x, &car_data->speed_z);

	// move the car
	// 3DO version
	//car_data->position.z += math_mul(car_data->speed_z, car_data->delta_time);
	//car_data->position.x -= math_mul(car_data->speed_x, car_data->delta_time);
	// PC/PSX version, cars are sped up 25% !!!
	car_data->position.z += math_mul(fix2(car_data->speed_z) + car_data->speed_z, car_data->delta_time);
	car_data->position.x -= math_mul(fix2(car_data->speed_x) + car_data->speed_x, car_data->delta_time);

	// suspension body roll
	if (car_data->speed_local_lat + car_data->speed_local_lon > 6553) {
		car_data->body_roll += (-car_data->body_roll -fixmul(car_data->accel_lat, car_specs->body_roll_factor)) / 2;
		car_data->body_pitch += (-car_data->body_pitch -fixmul(car_data->accel_lon, car_specs->body_pitch_factor)) / 2;
	} else {
		car_data->body_roll += -car_data->body_roll / 2;
		car_data->body_pitch += -car_data->body_pitch / 2;
	}

	// body rotation torque
	angular_accel = math_mul(car_data->rear_yaw_factor, ftire_lat_rear) - math_mul(car_data->front_yaw_factor, ftire_lat_front);

	// Rally Mode on PSX version, makes the car drift longer
	if ((g_game_settings & 0x20) && car_data->wheels_on_ground) {
		DAT_800eb20c = car_data->speed_local_lon;
		if (DAT_800eb20c > 0x10000) {
			DAT_800eb20c = 0x10000;
		}
		if (DAT_800eb20c < -0x10000) {
			DAT_800eb20c = -0x10000;
		}
		if ((DAT_800eb20c < 0x8000) && (car_data->rpm_engine > 3000) && (car_data->gear_selected > 0)) {
			DAT_800eb20c = 0x8000;
		}
		DAT_800eb208 = fix2(car_data->steer_angle) + steering;
		if (abs(car_data->speed_local_lon) < abs(car_data->speed_local_lat)) {
			if (car_data->throttle < 10) {
				aux = car_data->angular_speed / 2;
			} else {
				aux = fix3(car_data->angular_speed);
			}
			DAT_800eb208 = aux + DAT_800eb208;
		}
		aux = (DAT_800eb20c >> 10) * DAT_800eb208;
		DAT_800eb208 = fix6(aux);
		if (DAT_800eb204 != 0) {
			DAT_800eb208 = (aux >> 6) << 1;
		}
		DAT_800eb210 = 0x4b0000;
		if (DAT_800eb208 < 0) {
			DAT_800eb208 = DAT_800eb208 + 0xff;
		}
		aux = -car_data->accel_lon + 0x280000;
		if (aux < 0) {
			aux = -car_data->accel_lon + 0x2800ff;
		}
		DAT_800eb208 = math_mul(DAT_800eb208, aux) / 0x28;
		if (DAT_800eb208 < car_data->angular_speed) {
			angular_accel = (DAT_800eb208 - car_data->angular_speed) * 0x10;
			if (angular_accel < -0x4b0000) {
				angular_accel = -0x4b0000;
			}
		} else if (DAT_800eb208 > car_data->angular_speed) {
			angular_accel = (DAT_800eb208 - car_data->angular_speed) * 0x10;
			if (angular_accel > 0x4b0000) {
				angular_accel = 0x4b0000;
			}
		}
	}

	// apply body rotation
	car_data->angular_speed += math_mul(car_data->delta_time, angular_accel);

	if (abs(car_data->angular_speed) > 0x960000) {
		if (car_data->angular_speed < 0) {
			car_data->angular_speed = -0x960000;
		} else {
			car_data->angular_speed = 0x960000;
		}
	}

	// rotate car body
	body_rotate = math_mul(car_data->delta_time, car_data->angular_speed);
	// 3DO version
	//car_data->angle.y += body_rotate;
	// PC/PSX version
	car_data->angle.y += fix2(body_rotate) + body_rotate;

	// further car turning due to road bank angle
	if (abs(car_data->angle.z) > 0x30000) {

		track_heading_1 = track_data[car_data->track_slice & g_slice_mask].heading * 0x400;
		if (track_heading_1 > 0x800000)
			track_heading_1 -= 0x1000000;

		track_heading_2 = track_data[(car_data->track_slice + 1) & g_slice_mask].heading * 0x400;
		if (track_heading_2 > 0x800000)
			track_heading_2 -= 0x1000000;

		body_roll_sine = math_sin_3(car_data->angle.z);// >> 14);
		angular_accel = (car_data->speed_local_lon >> 8) * (track_heading_2 - track_heading_1);
		angular_accel = math_mul(body_roll_sine, angular_accel >> 8);
		car_data->angle.y -= fix7(angular_accel);
	}

	// wrap angle
	car_data->angle.y = math_angle_wrap(car_data->angle.y);

	debug_sum = car_data->speed_local_lon + car_data->speed_local_vert + car_data->speed_local_lat;

	// track fence collision
	tnfs_track_fence_collision(car_data);

	// replay recording
	if ((general_flags & 4) && is_recording_replay == 1) {
		if (car_data->speed_local_lon / 2 <= 0)
			local_speed_lon = car_data->speed_local_lon / -2;
		else
			local_speed_lon = car_data->speed_local_lon / 2;
		if (car_data->speed_local_lat <= 0)
			local_speed_lat = -car_data->speed_local_lat;
		else
			local_speed_lat = car_data->speed_local_lat;
		if (local_speed_lat > local_speed_lon && car_data->speed_local_lat > 0x8000)
			tnfs_replay_highlight_record(87);
		if ((car_data->slide_front || car_data->slide_rear) //
			&& car_data->speed > 0x140000 //
			&& ((car_data->tire_skid_rear & 1) || (car_data->tire_skid_front & 1))) {
				tnfs_replay_highlight_record(40);
		}
		if (DAT_800f62c0 == 0) {
			if (abs(car_data->speed_local_lon) > 3276) {
				tnfs_replay_highlight_record(120);
				DAT_800f62c0 = iSimTimeClock + 1;
			}
		}
	}

	if (stats_data_ptr->top_speed < abs(car_data->speed_local_lon)) {
		stats_data_ptr->top_speed = abs(car_data->speed_local_lon);
	}

	// Performance test - only PC version and Rusty Springs track
	if (!is_performance_test_off && selected_track == 3) {
		if (car_data->track_slice <= 97 || car_data->track_slice >= 465) {
			if (car_data->speed_local_lon < 0x3333) {
				stats_init_time = iSimTimeClock;
				stats_init_track_slice = car_data->track_slice_lap;
				stats_timer_a = 99999;
				stats_timer_b = 99999;
				stats_timer_c = 99999;
			}
			stats_elapsed_acc_time_1 = iSimTimeClock - stats_init_time;
			if (car_data->throttle > 50 && stats_elapsed_acc_time_1 < 1500 && stats_elapsed_acc_time_1 > 100) {
				if (car_data->speed_local_lon > 1755447 && stats_timer_a > stats_elapsed_acc_time_1) {
					stats_timer_a = iSimTimeClock - stats_init_time;
					tnfs_record_best_acceleration(stats_elapsed_acc_time_1, 0, 0, 0);
					if (stats_data_ptr->best_accel_time_1 > stats_elapsed_acc_time_1)
						stats_data_ptr->best_accel_time_1 = stats_elapsed_acc_time_1;
				}
				if (car_data->speed_local_lon > 2926182 && stats_timer_b > stats_elapsed_acc_time_1) {
					stats_timer_b = stats_elapsed_acc_time_1;
					tnfs_record_best_acceleration(0, stats_elapsed_acc_time_1, 0, 0);
					if (stats_data_ptr->best_accel_time_2 > stats_elapsed_acc_time_1)
						stats_data_ptr->best_accel_time_2 = stats_elapsed_acc_time_1;
				}
			}
			if (car_data->track_slice_lap - stats_init_track_slice > 83) {
				stats_elapsed_acc_time_2 = iSimTimeClock - stats_init_time;
				if (stats_timer_c > iSimTimeClock - stats_init_time && stats_elapsed_acc_time_2 < 1000) {
					if (stats_elapsed_acc_time_2 < stats_data_ptr->quarter_mile_time) {
						stats_data_ptr->quarter_mile_time = stats_elapsed_acc_time_2;
						stats_data_ptr->quarter_mile_speed = car_data->speed_local_lon;
					}
					tnfs_record_best_acceleration(0, 0, stats_elapsed_acc_time_2, car_data->speed_local_lon);
					stats_timer_c = stats_elapsed_acc_time_2;
				}
			}
			if (car_data->brake > 50) {
				if (car_data->speed_local_lon > 2340290)
					stats_braking_init_time = iSimTimeClock;
				if (car_data->speed_local_lon > 1755447)
					stats_braking_final_time = iSimTimeClock;
				if (car_data->speed_local_lon < 6553 && stats_braking_final_time > 0) {
					stats_elapsed_brake_time_1 = iSimTimeClock - stats_braking_final_time;

					printf("TUNING STATS : 60-0 in seconds %d = feet %d", 100 * (iSimTimeClock - stats_braking_final_time) / 60, stats_elapsed_brake_time_1);
					tnfs_record_best_braking(stats_elapsed_brake_time_1, 0);
					stats_braking_final_time = 0;
					if (stats_data_ptr->best_brake_time_1 > stats_elapsed_brake_time_1)
						stats_data_ptr->best_brake_time_1 = stats_elapsed_brake_time_1;
				}
				if (car_data->speed_local_lon < 6553 && stats_braking_init_time > 0) {
					stats_elapsed_brake_time_2 = iSimTimeClock - stats_braking_init_time;

					printf("TUNING STATS : 80-0 in %d seconds = %d feet", 100 * (iSimTimeClock - stats_braking_init_time) / 60, stats_elapsed_brake_time_2);
					tnfs_record_best_braking(0, stats_elapsed_brake_time_2);
					stats_braking_init_time = 0;
					if (stats_data_ptr->best_brake_time_2 > stats_elapsed_brake_time_2)
						stats_data_ptr->best_brake_time_2 = stats_elapsed_brake_time_2;
				}
			}
		}
		if (car_data->tire_skid_rear) {
			if (abs(car_data->angular_speed) > 3276800)
				tnfs_replay_highlight_record(87);
		}
	}

	debug_sum = car_data->position.z + car_data->position.y + car_data->position.x;
}

void tnfs_height_3B76F(tnfs_car_data *car) {
	tnfs_vec3 front;
	tnfs_vec3 side;

	math_rotate_2d(0, car->car_length, -car->angle_dy, &front.x, &front.z);
	math_rotate_2d(-car->car_width, 0, -car->angle_dy, &side.x, &side.z);

	car->front_edge.x = front.x;
	car->front_edge.y = 0;
	car->front_edge.z = front.z;
	car->side_edge.x = side.x;
	car->side_edge.y = 0;
	car->side_edge.z = side.z;
}

void tnfs_height_3B6AB(tnfs_car_data *car) {
	tnfs_vec3 front;
	tnfs_vec3 side;

	math_rotate_2d(0, car->car_length, -car->angle.y, &front.x, &front.z);
	math_rotate_2d(-car->car_width, 0, -car->angle.y, &side.x, &side.z);

	car->front_edge.x = front.x;
	car->front_edge.y = 0;
	car->front_edge.z = front.z;
	car->side_edge.x = side.x;
	car->side_edge.y = 0;
	car->side_edge.z = side.z;
}

void tnfs_height_road_position(tnfs_car_data *car_data, int mode) {
	tnfs_vec3 *pR;
	tnfs_vec3 pC;
	tnfs_vec3 pB;
	tnfs_vec3 pA;
	int node;

	if (mode) {
		tnfs_height_3B76F(car_data);
		car_data->world_position.x = car_data->position.x;
		car_data->world_position.y = car_data->position.y;
		car_data->world_position.z = car_data->position.z;
		pR = &car_data->world_position;
	} else {
		tnfs_height_3B6AB(car_data);
		car_data->road_ground_position.x = car_data->position.x;
		car_data->road_ground_position.y = car_data->position.y;
		car_data->road_ground_position.z = car_data->position.z;
		pR = &car_data->road_ground_position;
	}

	// get 3 surface points to triangulate surface position
	node = car_data->track_slice & g_slice_mask;
	pA.x = track_data[node].pos.x;
	pA.y = track_data[node].pos.y;
	pA.z = track_data[node].pos.z;

	pB.x = (track_data[node].side_normal_x) * 2 + pA.x;
	pB.y = (track_data[node].side_normal_y) * 2 + pA.y;
	pB.z = (track_data[node].side_normal_z) * 2 + pA.z;

	node = (car_data->track_slice + 1) & g_slice_mask;
	pC.x = track_data[node].pos.x;
	pC.y = track_data[node].pos.y;
	pC.z = track_data[node].pos.z;

	math_height_coordinates(&car_data->front_edge, &car_data->side_edge, pR, &pA, &pB, &pC);
}

void tnfs_height_car_inclination(tnfs_car_data *car, int rX, int rZ) {
	int angle_x;
	int angle_z;
	int dz;
	int dx;
	int aux;

	angle_x = math_atan2(car->car_length, rX - car->front_edge.y);
	angle_z = math_atan2(car->car_width,  -rZ - car->side_edge.y);

	// track slice inclination transition
	dx = abs(fix2(angle_x - car->angle.x));
	dz = abs(fix2(angle_z - car->angle.z));

	aux = car->angle.x - angle_x;
	if (aux <= dx) {
		if (aux < -dx)
			car->angle.x += dx;
	} else {
		car->angle.x -= dx;
	}

	aux = car->angle.z - angle_z;
	if (aux <= dz) {
		if (aux < -dz)
			car->angle.z += dz;
	} else {
		car->angle.z -= dz;
	}
}

void tnfs_height_get_surface_inclination(tnfs_car_data *car, int *x, int *z) {
	*x = math_atan2(car->car_length, -car->front_edge.y);
	*z = math_atan2(car->car_width, -car->side_edge.y);
}

int DAT_00145380 = 0;

/*
 * Simulate gravity and position the car above the road.
 */
void tnfs_height_position(tnfs_car_data *car, int is_driving_mode) {
	int bounce;
	int angleX;
	int angleZ;
	int nextHeight;
	int aux;
	int aux2 = 0;

	tnfs_height_road_position(car, 0);

	aux2 = car->road_ground_position.y;
	DAT_00145380 = car->track_slice;

	/* apply gravity */
	car->speed_y -= ((car->delta_time >> 2) * 0x9cf5c) >> 14;

	if ((g_game_settings & 0x20) == 0) {
		aux = ((car->delta_time >> 2) * 0x9cf5c);
		car->speed_y -= ((aux >> 14) - (aux >> 0x1f)) >> 1;
	}

	// next frame falling height
	nextHeight = (car->delta_time >> 4) * ((fix2(car->speed_y) + car->speed_y) >> 0xc) + car->position.y;

	if (!is_driving_mode) {
		// in crash mode
		car->position.y = aux2;
		car->speed_y = 0;
		car->slope_force_lon = 0;
		car->slope_force_lat = 0;
		return;
	}

	if (car->road_ground_position.y >= nextHeight - 0x40) {
		// hit the ground

		nextHeight = car->road_ground_position.y;

		tnfs_height_car_inclination(car, 0, 0);
		tnfs_height_get_surface_inclination(car, &angleX, &angleZ);

		// vertical ramp speed
		car->speed_y = -fixmul(car->speed_local_lat, math_sin_3(angleZ)) - fixmul(car->speed_local_lon, math_sin_3(angleX));

		if (car->time_off_ground > 20) {
			if (g_game_settings & 0x20)
				//bounce = fix2(car_data->time_off_ground / 6) * (((car_data->delta_time >> 2) * 0x9cf5c) >> 14); //PSX
				bounce = (((car->delta_time >> 2) * 0x9cf5c) >> 14) * (((car->time_off_ground >> 0x1f) - car->time_off_ground) >> 1);
			else
				bounce = (((car->delta_time >> 2) * 0x9cf5c) >> 14) * fix2(car->time_off_ground);

			car->speed_y += bounce;
			printf("Boink %d %d\n", car->speed_y >> 16, 0);
		}

		if (car->time_off_ground > 10) {
			tnfs_replay_highlight_record(84);
			car->time_off_ground = -2;
			car->angle.x = angleX;
			car->angle.z = angleZ;
			if (car->car_id2 == 0)
				DAT_8010c478 = 5;
		}

		if (car->time_off_ground < 0)
			car->time_off_ground++;
		else
			car->time_off_ground = 0;

		if (car->time_off_ground == -1 && car->car_id2 == 0) {
			tnfs_sfx_play(-1, 4, 1, abs(car->speed_y), 1, 0x50000);
		}

		if (!car->wheels_on_ground && car->gear_selected != -1)
			car->is_gear_engaged = 1;

		car->wheels_on_ground = 1;

	} else {
		// wheels on air
		if (nextHeight - car->road_ground_position.y >= 0x4000) {
			car->is_gear_engaged = 0;
			car->wheels_on_ground = 0;
			car->time_off_ground++;

			if (car->angle.z >= 0)
				tnfs_height_car_inclination(car, 983 * car->time_off_ground, -655 * car->time_off_ground);
			else
				tnfs_height_car_inclination(car, 983 * car->time_off_ground, 655 * car->time_off_ground);
		} else {
			tnfs_height_car_inclination(car, 0, 0);

			if (!car->wheels_on_ground && car->gear_selected != -1)
				car->is_gear_engaged = 1;

			car->wheels_on_ground = 1;
		}
	}

	car->slope_force_lon = (math_sin_3(car->angle.x) >> 8) * 0x9cf;
	car->unknown_flag_3DD = 0;
	car->slope_force_lat = 0;

	if (car->speed_y > 0) {
		car->slope_force_lon = math_mul(0x2666, car->slope_force_lon);
	} else {
		if (car->speed_y < 0)
			car->slope_force_lon = math_mul(0x10000, car->slope_force_lon);
	}

	if (DAT_8010c478)
		DAT_8010c478--;

	car->position.y = nextHeight;
}

/*
 * Main simulation loop
 * EXE address:
 * - 3DO 0xf8d8,
 * - PC DOS DEMO 0x3b4f6
 * - PC DOS 0x580A5
 * - PSX 8002df18
 * - WIN95 TNFSSE 0x4286a0
 */
void tnfs_driving_main(tnfs_car_data *car) {
	tnfs_physics_update(car);
	tnfs_track_node_update(car);
	tnfs_height_position(car, 1);
}
