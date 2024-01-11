/*
 * The "Fiziks" module
 * 2D car handling dynamics
 */
#include "tnfs_math.h"
#include "tnfs_base.h"
#include "tnfs_collision_2d.h"
#include "tnfs_collision_3d.h"
#include "tnfs_engine.h"

int unknown_stats_array[128];

// settings/flags
int is_recording_replay = 1;
int general_flags = 0x14;
int debug_sum = 0;
int selected_track = 3; //rusty springs
int DAT_8010d1cc = 0;
int DAT_8010d2f4 = 0;
int DAT_8010d30c = 0;

int g_car_road_segment;
int DAT_8010c478;

// debug stats flags
int stats_braking_init_time = 0;
int stats_braking_final_time = 0;
int stats_timer_a = 0;
int stats_timer_b = 0;
int stats_timer_c = 0;
int stats_init_time = 0;
int stats_init_road_segment = 0;
int DAT_800f62c0 = 0;
int debug_mode_pc_version = 0; //DAT_00146483

void tnfs_debug_00034309(int a, int b, int x, int y) {
	//TODO
}

void tnfs_debug_000343C2(int a, int b) {
	//TODO
}

int tnfs_drag_force(tnfs_car_data *a1, signed int speed) {
	int max;
	int incl_angle;
	int drag;
	int sq_speed;

	if (road_surface_type_array[a1->surface_type_a * 4 + 3])
		a1->surface_type_b = 4;
	else
		a1->surface_type_b = 0;

	sq_speed = speed >> 16;
	sq_speed *= sq_speed;

	drag = fix8(road_surface_type_array[a1->surface_type_a * 4 + 1] * a1->car_specs_ptr->drag) * sq_speed;

	incl_angle = a1->angle_z;
	if (incl_angle > 0x800000)
		incl_angle -= 0x1000000;

	if (selected_track > 2 && abs(incl_angle) > 0x9FFFF)
		drag = fix3(drag);

	max = abs(speed * a1->fps);
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
	if (cheat_mode == 4) {
		tnfs_collision_rollover_start(&car_data, 0xa0000, 0xa0000, 0xa0000);
	}
}

void tnfs_tire_forces_locked(int *force_lat, int *force_lon, signed int max_grip, int *slide) {
	signed int residual;
	int abs_f_lon;
	int abs_f_lat;
	signed int force;

	// total force
	abs_f_lon = abs(*force_lat);
	abs_f_lat = abs(*force_lon);
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
	signed int angle;
	signed int slope_r;
	signed int slope_1;
	signed int slope_0;

	//slope_0 = *(DAT_80012DECC + 36 * (DAT_8001328E4 & car_data->road_segment_a) + 18) >> 16 << 10;
	slope_0 = track_data[car_data->road_segment_a].slope; // >> 16; //* 0x400;
	if (slope_0 > 0x800000)
		slope_0 -= 0x1000000;

	//if (DAT_8001328DC) {
	//	slope_1 = *(DAT_80012DECC + 36 * (DAT_8001328E4 & DAT_8001328E4 & (car_data->road_segment_a + 1)) + 18) >> 16 << 10;
	//} else {
	//	slope_1 = *(DAT_80012DECC + 36 * (DAT_8001328E4 & (car_data->road_segment_a + 1)) + 18) >> 16 << 10;
	//}
	slope_1 = track_data[car_data->road_segment_a + 1].slope; // >> 16; //* 0x400;
	if (slope_1 > 0x800000)
		slope_1 -= 0x1000000;

	slope_r = math_mul(math_sin_3(slope_1 - slope_0), car_data->speed_local_lon) * 4;
	if (slope_r > 0)
		slope_r += slope_r >> 1;

	angle = car_data->angle_z;
	if (angle > 0x800000)
		angle -= 0x1000000;

	car_data->road_grip_increment = 2 * (fixmul(abs(car_data->speed_local_lon), abs(angle)) >> 7) + slope_r;

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
int tnfs_tire_slide_table(tnfs_car_data *a1, unsigned int slip_angle, int is_rear_wheels) {
	signed int angle;

	angle = slip_angle;
	if (slip_angle > 2097150)
		angle = 2097150;

	//original
	//return *(a1->car_specs_ptr + (is_rear_wheels << 9) + (angle >> 12) + 884) << 9;
	//smaller array
	return a1->car_specs_ptr->grip_table[(is_rear_wheels << 5) + (angle >> 16)] << 9;
}

void tnfs_tire_limit_max_grip(tnfs_car_data *car_data, //
		int *force_lat, signed int *force_lon, signed int max_grip, int *slide) {

	int f_lat_abs;
	signed int f_lon_abs;
	signed int r_grip;
	int s_grip;
	int factor;
	signed int v12;
	int f_lon_abs2;
	int f_total;

	r_grip = max_grip;

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

		v12 = (s_grip - max_grip);

		if (s_grip - max_grip > 0)
			r_grip = max_grip - v12;

		factor = r_grip / fix8(f_total);

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
			printf("ANGLE WARNING!! %s=%d\n", "a", f_lat_abs);
		}
	}

	if (&car_data->slide_rear == slide) {
		if ((*force_lon > 0 && car_data->speed_local_lon > 0) || (*force_lon < 0 && car_data->speed_local_lon < 0)) {
			car_data->slide = v12;
		}
	}
}

void tnfs_tire_forces(tnfs_car_data *car, //
		int *_result_Lat, int *_result_Lon, //
		int force_Lat, int force_Lon, //
		signed int steering, int thrust_force, int braking_force, //
		int is_front_wheels) {

	unsigned int slip_angle;
	int force_decc;
	int f_lat_loc_abs2;
	int f_lon_loc_abs2;
	int grip_force;
	int lateral_force;
	int force_lat_local_abs;
	int *slide;
	char *skid;
	signed int max_grip;
	int skid_amount;
	int force_lon_local;
	int force_lat_local;
	int result_brake_thrust;
	int slip_angle_grip;
	struct tnfs_car_specs *car_specs;

	car_specs = car->car_specs_ptr;

	skid_amount = 0;
	if (car->time_off_ground > 0) {
		*_result_Lon = 0;
		*_result_Lat = 0;
		return;
	}

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

	force_lat_local_abs = abs(force_lat_local);
	if (force_lon_local <= 0)
		slip_angle = math_atan2(-force_lon_local, force_lat_local_abs);
	else
		slip_angle = math_atan2(force_lon_local, force_lat_local_abs);

	if (slip_angle > car_specs->cutoff_slip_angle) {
		slip_angle = car_specs->cutoff_slip_angle;
		*skid |= 1u;
	}

	if ((car->brake <= 245 || car->abs_enabled) //
		&& ( fix2(5 * max_grip) >= abs(braking_force) //
		|| (car->abs_enabled != 0 && car->handbrake == 0))) {
		// not locked wheels

		// lateral tire grip factor
		slip_angle_grip = fixmul(max_grip, tnfs_tire_slide_table(car, slip_angle, is_front_wheels - 1));
		lateral_force = abs(force_lat_local);

		if (lateral_force > slip_angle_grip) {
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
			}
			tnfs_tire_limit_max_grip(car, &force_lat_local, &force_lon_local, max_grip, slide);
			if (*slide != 0) {
				*slide *= 8;
			}
			if (*slide < skid_amount) {
				*slide = skid_amount;
			}

		} else {
			//deceleration
			force_decc = abs(result_brake_thrust);
			if (force_decc < abs(force_lon_local)) {
				if (force_lon_local <= 0) {
					force_lon_local = -force_decc;
				} else {
					force_lon_local = force_decc;
				}
			}
			tnfs_tire_limit_max_grip(car, &force_lat_local, &force_lon_local, max_grip, slide);
			*slide |= skid_amount;
			if (car->abs_on)
				*slide = 0;
		}

	} else {
		// locked wheels: hard braking or handbrake
		if (abs(force_lon_local) > max_grip)
			*skid |= 1u;

		tnfs_tire_forces_locked(&force_lat_local, &force_lon_local, max_grip, slide);
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

/*
 * Main physics routine
 */
void tnfs_physics_update(tnfs_car_data *a1) {
	signed int braking_total;
	int sideslip;
	int tr_weight;
	signed int body_rotate;
	signed int stats_elapsed_brake_time_1;
	signed int stats_elapsed_brake_time_2;
	int stats_elapsed_acc_time_1;
	signed int stats_elapsed_acc_time_2;
	int local_speed_lon;
	int local_speed_lat;
	signed int track_heading_2;
	signed int track_heading_1;
	int force_lon_total;
	int limit_tire_force;
	int frameskip;
	signed int abs_speed_x;
	signed int abs_speed_y;
	tnfs_car_data *car_data;
	int rLon;
	int rLat;
	int fLon;
	int fLat;
	int *stats_data_ptr;
	int braking_rear;
	int braking_front;
	int traction_rear;
	int traction_front;
	int drag_lon;
	int drag_lat;
	signed int thrust_force;
	signed int v59;
	int steering;
	int max_lateral_acc;
	int force_Lon;
	int force_lon_adj;
	int force_Lat;
	int f_R_Lon;
	int f_R_Lat;
	int f_F_Lon;
	int f_F_Lat;
	int sLon;
	int sLat;
	tnfs_car_specs *car_specs;
	int body_roll_sine;
	int track_turning;

	// gather basic car data
	car_data = a1;

	//v51 = &DAT_800144728[116 * a1->dword475];
	stats_data_ptr = &unknown_stats_array[0];

	is_drifting = 0;
	car_specs = a1->car_specs_ptr;

	// fast vec2 length
	abs_speed_x = abs(a1->speed_x);
	abs_speed_y = abs(a1->speed_z);
	if (abs_speed_x <= abs_speed_y)
		a1->speed = (abs_speed_x >> 2) + abs_speed_y;
	else
		a1->speed = (abs_speed_y >> 2) + abs_speed_x;

	// framerate fixed values
	a1->delta_time = 2184;
	a1->fps = 30;

	// custom scales/framerate speeds, not using
	if (DAT_8010d1cc > 1 && DAT_8010d2f4 != 0) {
		//frameskip = (*(DAT_800132F84[1 - a1->dword475] + 76) - a1->road_segment_b) / 2;
		frameskip = 0;
		if (frameskip > 0) {
			if (frameskip > DAT_8010d2f4)
				frameskip = DAT_8010d2f4;
			a1->fps = 30 - frameskip;
			a1->delta_time = 0x10000 / a1->fps;
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
	if ((general_flags & 4) || a1->gear_selected == -1) {
		tnfs_engine_rev_limiter(a1);
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

	car_data->slide = 0;

	// traction forces
	traction_front = fixmul(car_specs->front_drive_percentage, thrust_force);
	traction_rear = thrust_force - traction_front;

	// braking forces
	if (car_data->brake <= 240) {
		braking_total = fixmul(330 * car_data->brake, car_data->tire_grip_rear + car_data->tire_grip_front);
		braking_front = fixmul(car_specs->front_brake_percentage, braking_total);
		braking_rear = (braking_total - braking_front);
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

	// gear change delay
	if (car_data->gear_selected != car_data->gear_shift_current) {
		car_data->gear_shift_interval = 16;
		car_data->gear_shift_previous = car_data->gear_shift_current;
		car_data->gear_shift_current = car_data->gear_selected;
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

	if (car_data->speed_local_lon > car_specs->top_speed && selected_track != 6) {
		if (drag_lon > 0 && drag_lon < thrust_force) {
			drag_lon = abs(thrust_force);
		} else if (drag_lon < 0) {
			if (abs(drag_lon) < abs(thrust_force)) {
				drag_lon = -abs(thrust_force);
			}
		}
	}

	// BEGIN of car traction/slip trajectory
	math_angle_wrap(car_data->steer_angle + car_data->angle_y);

	// convert to local frame of reference
	math_rotate_2d(car_data->speed_x, car_data->speed_z, -car_data->angle_y, &car_data->speed_local_lat, &car_data->speed_local_lon);

	// scale speeds
	sLat = car_data->fps * car_data->speed_local_lat;
	sLon = car_data->fps * car_data->speed_local_lon;

	// sideslip
	sideslip = fixmul(car_data->wheel_base, car_data->fps * car_data->angular_speed);

	// split front and rear moment of inertia, inverted
	fLat = -fixmul(car_data->weight_distribution_front, sLat) - (drag_lat / 2) + sideslip;
	rLat = -(sLat - fixmul(car_data->weight_distribution_front, sLat)) - (drag_lat / 2) - sideslip;
	rLon = -(sLon - fixmul(car_data->weight_distribution_front, sLon)) - (drag_lon / 2);
	fLon = -fixmul(car_data->weight_distribution_front, sLon) - (drag_lon / 2);

	// adjust steer sensitivity
	v59 = car_data->steer_angle;
	if (car_data->brake <= 200) {
		steering = v59 - fix15(fix8(car_data->speed_local_lon) * v59);
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
			if (force_Lat <= 0 ? -force_Lat : f_R_Lat + f_F_Lat > limit_tire_force) {
				if (force_Lat < 0) {
					force_Lat = -limit_tire_force;
				} else {
					force_Lat = limit_tire_force;
				}
			}
		}
	}

	// tire lateral force limit
	max_lateral_acc = car_data->road_grip_increment + car_specs->lateral_accel_cutoff;
	if (max_lateral_acc < 0)
		max_lateral_acc = 0;
	if (abs(force_Lat) > max_lateral_acc) {
		if (force_Lat < 0)
			force_Lat = -max_lateral_acc;
		else
			force_Lat = max_lateral_acc;
	}

	// calculate grip forces
	tr_weight = fixmul(force_Lon, car_data->weight_transfer_factor);

	car_data->tire_grip_front = fix8((car_data->front_friction_factor - tr_weight) * road_surface_type_array[4 * car_data->surface_type_a]);

	car_data->tire_grip_rear = fix8((tr_weight + car_data->rear_friction_factor) * road_surface_type_array[4 * car_data->surface_type_a]);

	tnfs_road_surface_modifier(car_data);

	// thrust force to acc (force/mass=acc?)
	force_lon_adj = fix8(car_specs->thrust_to_acc_factor * force_Lon);

	// suspension inclination
	car_data->susp_incl_lat = force_Lat;
	car_data->susp_incl_lon = force_lon_adj;

	// convert speeds to world scale (m/s >> 16)
	if (abs(car_data->speed_local_lon) + abs(car_data->speed_local_lat) >= 19660) {
		// car moving
		car_data->speed_local_lat += fixmul(car_data->delta_time, (drag_lat + force_Lat + car_data->slope_force_lat));
		car_data->speed_local_lon += fixmul(car_data->delta_time, (drag_lon + force_lon_adj + car_data->slope_force_lon));
	} else {
		// car stopped
		car_data->speed_local_lat += fixmul(car_data->delta_time, force_Lat);
		car_data->speed_local_lon += fixmul(car_data->delta_time, force_lon_adj);

		if (car_data->gear_selected == -1 || car_data->throttle == 0) {
			car_data->speed_local_lon = 0;
			car_data->speed_local_lat = 0;
		}
	}

	// rotate back to global frame of reference
	math_rotate_2d(car_data->speed_local_lat, car_data->speed_local_lon, car_data->angle_y, &car_data->speed_x, &car_data->speed_z);

	// move the car
	car_data->position.z += fixdiv(car_data->speed_z, car_data->delta_time);
	car_data->position.x -= fixdiv(car_data->speed_x, car_data->delta_time);

	// suspension body roll
	if (car_data->speed_local_lat + car_data->speed_local_lon <= 6553) {
		car_data->body_roll += -car_data->body_roll / 2;
		car_data->body_pitch += -car_data->body_pitch / 2;
	} else {
		car_data->body_roll += (-(fixmul(car_data->susp_incl_lat, car_specs->body_roll_factor)) - car_data->body_roll) / 2;
		car_data->body_pitch += (-(fixmul(car_data->susp_incl_lon, car_specs->body_pitch_factor)) - car_data->body_pitch) / 2;
	}

	// apply body rotation torque
	car_data->angular_speed += fixmul(car_data->delta_time, //
			fixmul(car_data->rear_yaw_factor, f_R_Lat) - fixmul(car_data->front_yaw_factor, f_F_Lat));

	if (abs(car_data->angular_speed) > 0x960000) {
		if (car_data->angular_speed >= 0)
			car_data->angular_speed = 0x960000;
		else
			car_data->angular_speed = -0x960000;
	}

	// rotate car body
	body_rotate = fixmul(car_data->delta_time, car_data->angular_speed);
	car_data->angle_y += fix2(body_rotate) + body_rotate;

	// further car turning due to road bank angle
	if (abs(car_data->angle_z) > 0x30000) {

		////track_heading_1 = *(DAT_80012DECC + 36 * (DAT_8001328E4 & car_data->road_segment_a) + 22) >> 16 << 10;
		track_heading_1 = track_data[car_data->road_segment_a].heading * 0x400;
		if (track_heading_1 > 0x800000)
			track_heading_1 -= 0x1000000;

		//if (DAT_8001328DC) {
		//	track_heading_2 = *(DAT_80012DECC + 36 * (DAT_8001328E4 & DAT_8001328E4 & (car_data->road_segment_a + 1)) + 22) >> 16 << 10;
		//} else {
		//	track_heading_2 = *(DAT_80012DECC + 36 * (DAT_8001328E4 & (car_data->road_segment_a + 1)) + 22) >> 16 << 10;
		//}
		track_heading_2 = track_data[car_data->road_segment_a + 1].heading * 0x400;
		if (track_heading_2 > 0x800000)
			track_heading_2 -= 0x1000000;

		body_roll_sine = math_sin_3(car_data->angle_z);// >> 14);
		track_turning = (car_data->speed_local_lon >> 8) * (track_heading_2 - track_heading_1);
		track_turning = math_mul(body_roll_sine, track_turning >> 8);
		car_data->angle_y -= fix7(track_turning);
	}

	// wrap angle
	car_data->angle_y = math_angle_wrap(car_data->angle_y);

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
			tnfs_replay_highlight_000502AB(87);
		if ((car_data->slide_front || car_data->slide_rear) //
		&& car_data->speed > 0x140000 //
		&& ((car_data->tire_skid_rear & 1) || (car_data->tire_skid_front & 1))) {
			tnfs_replay_highlight_000502AB(40);
		}
		if (DAT_800f62c0 == 0) {
			if (abs(car_data->speed_local_lon) > 3276) {
				tnfs_replay_highlight_000502AB(120);
				DAT_800f62c0 = g_game_time + 1;
			}
		}
	}

	if (stats_data_ptr[113] < abs(car_data->speed_local_lon)) {
		stats_data_ptr[113] = abs(car_data->speed_local_lon);
	}

	// Performance test - only PC version and Rusty Springs track
	if (!debug_mode_pc_version && selected_track == 3) {
		if (car_data->road_segment_a <= 97 || car_data->road_segment_a >= 465) {
			if (car_data->speed_local_lon < 13107) {
				stats_init_time = g_game_time;
				stats_init_road_segment = car_data->road_segment_b;
				stats_timer_a = 99999;
				stats_timer_b = 99999;
				stats_timer_c = 99999;
			}
			stats_elapsed_acc_time_1 = g_game_time - stats_init_time;
			if (car_data->throttle > 50 && stats_elapsed_acc_time_1 < 1500 && stats_elapsed_acc_time_1 > 100) {
				if (car_data->speed_local_lon > 1755447 && stats_timer_a > stats_elapsed_acc_time_1) {
					stats_timer_a = g_game_time - stats_init_time;
					tnfs_debug_00034309(stats_elapsed_acc_time_1, 0, 0, 0);
					if (stats_data_ptr[102] > stats_elapsed_acc_time_1)
						stats_data_ptr[102] = stats_elapsed_acc_time_1;
				}
				if (car_data->speed_local_lon > 2926182 && stats_timer_b > stats_elapsed_acc_time_1) {
					stats_timer_b = stats_elapsed_acc_time_1;
					tnfs_debug_00034309(0, stats_elapsed_acc_time_1, 0, 0);
					if (stats_data_ptr[103] > stats_elapsed_acc_time_1)
						stats_data_ptr[103] = stats_elapsed_acc_time_1;
				}
			}
			if (car_data->road_segment_b - stats_init_road_segment > 83) {
				//TODO DAT_800D8B00 = &unk_F423F;
				stats_elapsed_acc_time_2 = g_game_time - stats_init_time;
				if (stats_timer_c > g_game_time - stats_init_time && stats_elapsed_acc_time_2 < 1000) {
					if (stats_elapsed_acc_time_2 < stats_data_ptr[107]) {
						stats_data_ptr[107] = stats_elapsed_acc_time_2;
						stats_data_ptr[106] = car_data->speed_local_lon;
					}
					tnfs_debug_00034309(0, 0, stats_elapsed_acc_time_2, car_data->speed_local_lon);
					stats_timer_c = stats_elapsed_acc_time_2;
				}
			}
			if (car_data->brake > 50) {
				if (car_data->speed_local_lon > 2340290)
					stats_braking_init_time = g_game_time;
				if (car_data->speed_local_lon > 1755447)
					stats_braking_final_time = g_game_time;
				if (car_data->speed_local_lon < 6553 && stats_braking_final_time > 0) {
					stats_elapsed_brake_time_1 = g_game_time - stats_braking_final_time;

					printf("TUNING STATS : 60-0 in seconds %d = feet %d", 100 * (g_game_time - stats_braking_final_time) / 60, stats_elapsed_brake_time_1);
					tnfs_debug_000343C2(stats_elapsed_brake_time_1, 0);
					stats_braking_final_time = 0;
					if (stats_data_ptr[105] > stats_elapsed_brake_time_1)
						stats_data_ptr[105] = stats_elapsed_brake_time_1;
				}
				if (car_data->speed_local_lon < 6553 && stats_braking_init_time > 0) {
					stats_elapsed_brake_time_2 = g_game_time - stats_braking_init_time;

					printf("TUNING STATS : 80-0 in %d seconds = %d feet", 100 * (g_game_time - stats_braking_init_time) / 60, stats_elapsed_brake_time_2);
					tnfs_debug_000343C2(0, stats_elapsed_brake_time_2);
					stats_braking_init_time = 0;
					if (stats_data_ptr[104] > stats_elapsed_brake_time_2)
						stats_data_ptr[104] = stats_elapsed_brake_time_2;
				}
			}
		}
		if (car_data->tire_skid_rear) {
			if (abs(car_data->angular_speed) > 3276800)
				tnfs_replay_highlight_000502AB(87);
		}
	}

	debug_sum = car_data->position.z + car_data->position.y + car_data->position.x;
}

void tnfs_height_3B76F(tnfs_car_data *a1) {
  tnfs_vec3 v1;
  tnfs_vec3 v3;

  math_rotate_2d(0, a1->car_length, -a1->angle_dx, &v1.x, &v1.z);
  math_rotate_2d(-a1->car_width, 0, -a1->angle_dx, &v3.x, &v3.z);

  a1->front_edge.x = v1.x;
  a1->front_edge.y = 0;
  a1->front_edge.z = v1.z;
  a1->side_edge.x = v3.x;
  a1->side_edge.y = 0;
  a1->side_edge.z = v3.z;
}

void tnfs_height_3B6AB(tnfs_car_data *a1) {
  tnfs_vec3 v1;
  tnfs_vec3 v3;

  math_rotate_2d(0, a1->car_length, -a1->angle_y, &v1.x, &v1.z);
  math_rotate_2d(-a1->car_width, 0, -a1->angle_y, &v3.x, &v3.z);

  a1->front_edge.x = v1.x;
  a1->front_edge.y = 0;
  a1->front_edge.z = v1.z;
  a1->side_edge.x = v3.x;
  a1->side_edge.y = 0;
  a1->side_edge.z = v3.z;
}

void tnfs_height_road_position(tnfs_car_data *car_data, int mode) {
	tnfs_vec3 pC;
	tnfs_vec3 pB;
	tnfs_vec3 pA;
	tnfs_vec3 *pR;
	int node;

	if (mode) {
		tnfs_height_3B76F(car_data);
		pR = &car_data->world_position;
	} else {
		tnfs_height_3B6AB(car_data);
		pR = &car_data->road_ground_position;
	}

	pR->x = car_data->position.x;
	pR->y = car_data->position.y;
	pR->z = car_data->position.z;

	// get 3 surface points to triangulate surface position
	node = car_data->road_segment_a;
	pA.x = track_data[node].pos.x;
	pA.y = track_data[node].pos.y;
	pA.z = track_data[node].pos.z;

	pB.x = pA.x + (track_data[node].side_point.x >> 10) * 2;
	pB.y = pA.y + (track_data[node].side_point.y >> 10) * 2;
	pB.z = pA.z + (track_data[node].side_point.z >> 10) * 2;

	node += 1;
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

	// segment inclination transition
	dx = abs(fix2(angle_x - car->angle_x));
	dz = abs(fix2(angle_z - car->angle_z));

	aux = car->angle_x - angle_x;
	if (aux <= dx) {
		if (aux < -dx)
			car->angle_x += dx;
	} else {
		car->angle_x -= dx;
	}

	aux = car->angle_z - angle_z;
	if (aux <= dz) {
		if (aux < -dz)
			car->angle_z += dz;
	} else {
		car->angle_z -= dz;
	}
}

void tnfs_height_get_surface_inclination(tnfs_car_data *car, int *x, int *z) {
	*x = math_atan2(car->car_length, -car->front_edge.y);
	*z = math_atan2(car->car_width, -car->side_edge.y);
}

/*
 * Simulate gravity and position the car above the road.
 */
void tnfs_height_position(tnfs_car_data *car_data, int is_driving_mode) {
	int bounce;
	int angleX = 0;
	int angleZ = 0;
	int nHeight;

	tnfs_height_road_position(car_data, 0);

	g_car_road_segment = car_data->road_segment_a;

	/* apply gravity */
	car_data->speed_y -= 0x9cf5c * (car_data->delta_time >> 2) >> 14;
	car_data->speed_y -= 0x9cf5c * (car_data->delta_time >> 2) >> 15;

	// next frame falling height
	nHeight = (car_data->delta_time >> 4) * (((abs(car_data->speed_y) >> 2) + car_data->speed_y) >> 0xc) + car_data->position.y;

	if (!is_driving_mode) {
		// in crash mode
		car_data->position.y = car_data->road_ground_position.y;
		car_data->speed_y = 0;
		car_data->slope_force_lon = 0;
		car_data->slope_force_lat = 0;
		return;
	}

	if (car_data->road_ground_position.y >= nHeight - 0x40) {
		// hit the ground

		nHeight = car_data->road_ground_position.y;

		tnfs_height_car_inclination(car_data, 0, 0);
		tnfs_height_get_surface_inclination(car_data, &angleX, &angleZ);

		// vertical ramp speed
		car_data->speed_y = -fixmul(car_data->speed_local_lat, math_sin_3(angleZ)) - fixmul(car_data->speed_local_lon, math_sin_3(angleX));

		if (car_data->time_off_ground > 20) {
			if (DAT_8010d1c4 & 0x20)
				bounce = car_data->time_off_ground / 6;
			else
				bounce = car_data->time_off_ground;

			car_data->speed_y += (0x9cf5c * (car_data->delta_time >> 2) >> 14) * fix2(bounce);
			printf("Boink %d %d\n", car_data->speed_y >> 16, 0);
		}

		if (car_data->time_off_ground > 10) {
			tnfs_replay_highlight_000502AB(84);
			car_data->time_off_ground = -2;
			car_data->angle_x = angleX;
			car_data->angle_z = angleZ;
			if (car_data->unknown_flag_475 == 0)
				DAT_8010c478 = 5;
		}

		if (car_data->time_off_ground < 0)
			car_data->time_off_ground++;
		else
			car_data->time_off_ground = 0;

		if (car_data->time_off_ground == -1 && car_data->unknown_flag_475 == 0) {
			tnfs_sfx_play(-1, 4, 1, abs(car_data->speed_y), 1, 0x50000);
		}

		if (!car_data->wheels_on_ground && car_data->gear_selected != -1)
			car_data->is_gear_engaged = 1;

		car_data->wheels_on_ground = 1;

	} else {
		// wheels on air
		if (nHeight - car_data->road_ground_position.y >= 0x4000) {
			car_data->is_gear_engaged = 0;
			car_data->wheels_on_ground = 0;
			car_data->time_off_ground++;

			if (car_data->angle_z >= 0)
				tnfs_height_car_inclination(car_data, 983 * car_data->time_off_ground, -655 * car_data->time_off_ground);
			else
				tnfs_height_car_inclination(car_data, 983 * car_data->time_off_ground, 655 * car_data->time_off_ground);
		} else {
			tnfs_height_car_inclination(car_data, 0, 0);

			if (!car_data->wheels_on_ground && car_data->gear_selected != -1)
				car_data->is_gear_engaged = 1;

			car_data->wheels_on_ground = 1;
		}
	}

	car_data->slope_force_lon = (math_sin_3(car_data->angle_x) >> 8) * 0x9cf;
	car_data->unknown_flag_3DD = 0;
	car_data->slope_force_lat = 0;

	if (car_data->speed_y <= 0) {
		car_data->slope_force_lon = math_mul(0x10000, car_data->slope_force_lon);
	} else {
		car_data->slope_force_lon = math_mul(0x2666, car_data->slope_force_lon);
	}

	if (DAT_8010c478)
		DAT_8010c478--;

	car_data->position.y = nHeight;
}

/*
 * Main simulation loop
 */
void tnfs_driving_main() {
	tnfs_physics_update(&car_data);
	tnfs_road_segment_update(&car_data);
	tnfs_height_position(&car_data, 1);
}
