/*
 * AI driver
 */
#include "tnfs_math.h"
#include "tnfs_base.h"
#include "tnfs_collision_2d.h"

int DAT_000FDB8C = 0;
int DAT_000FDB94 = 0;
int DAT_001039D8 = 0;
int DAT_001039D4 = 0;
int DAT_001039DC = 0;
int DAT_0014DCC4 = 0;
int DAT_0014DCCC = 0xFFFF;
int DAT_00164FDC[20] = { 0, 0x100, 0x200, 0x300, 0x400, 0x500, 0x600, 0x700, 0x800, 0x900, 0xa00, 0xb00, 0xc00, 0xd00, 0xe00, 0xf00, 0x1000, 0x1100, 0x1200 };
tnfs_car_data* DAT_00165334;
int DAT_0016533C = 0;
int DAT_00165144 = 0;
int DAT_001651BC = 0;
int DAT_00165278 = 0xCCCC;
int DAT_001652CC = 0xCCCC;
int DAT_00165320 = 0x5999;
int DAT_0016533c = 0;
int DAT_00165340 = 0;
int DAT_0016709F = 1;
int DAT_001670AB = 0;
int DAT_001670B3 = 0;
int DAT_001670BB = 0;

tnfs_car_data* player_car_ptr;

int g_power_curve[] = { 0x5062, 0x4f1a, 0x4f1a, 0x55c2, 0x2937, 0x2c49, 0x28f5, 0x228f, 0x1df3, 0x19db, 0x7a14, 0x12f1, 0x11eb, //
		0xffd, 0xdf4, 0xc8b, 0xb43, 0x978, 0x7ae, 0x69e, 0x5e3, 0x560, 0x45a, 0x418, 0 };

void tnfs_ai_init() {
	int i;

	player_car_ptr = &player_car;

	for (i = 1; i < g_total_cars_in_scene; i++) {
    g_car_ptr_array[i]->field_174 = 0x1e4;
    //g_car_ptr_array[i]->field_174 |= 4; // run
    //g_car_ptr_array[i]->field_174 |= 8; //
    //g_car_ptr_array[i]->field_174 |= 0x404; //steer more
    //g_car_ptr_array[i]->field_174 |= 0x408; //
    //g_car_ptr_array[i]->field_174 |= 0x1000; //backwards
    //g_car_ptr_array[i]->field_174 |= 0x20000; //stopped

    for (i = 0; i <= 99; i++) {
      g_car_ptr_array[i]->power_curve[i] = g_power_curve[i >> 2];
    }
	}

	// globals
	player_car_ptr = &player_car;
}

void FUN_0044E11() {
	// stub
}

int FUN_00047121(tnfs_car_data *car) {
	return ((car->collision_data.speed).x >> 8) * ((car->road_heading).x >> 8) //
			+ ((car->road_heading).y >> 8) * ((car->collision_data.speed).y >> 8) //
			+ ((car->road_heading).z >> 8) * (-(car->collision_data.speed).z >> 8);
}

int FUN_0076FB9(int segment) {
	return 0;
}

void FUN_0077121(tnfs_car_data *car) {
	// stub
}

void FUN_00776EA() {
	// stub
}

int FUN_00077c58(tnfs_car_data *param_1) {
	return 0x136666;
}

int FUN_00080a75(tnfs_car_data *car, int speed) {
	return -0x136666;
}

int FUN_0007bbfe(tnfs_car_data *car) {
	return (-car->speed_x >> 8) * ((car->road_heading).x >> 8) //
			+ ((car->road_heading).y >> 8) * (car->speed_y >> 8) //
			+ ((car->road_heading).z >> 8) * (car->speed_z >> 8);
}

void FUN_007BC96(tnfs_car_data *car) {
	// stub
}

int FUN_007E87B(tnfs_car_data *car, int lane_grid, int param_3) {
	int iVar1;
	int iVar2;

	if ((car->field_174 & 4) == 0) {
		iVar2 = 0x33333;
	} else {
		iVar2 = 0x3cccc; // FIXME not a constant
	}
	if ((car->field_174 & 4) == 0) {
		iVar1 = 0x4000;
	} else {
		iVar1 = 0x4ccc; // FIXME not a constant
	}
	iVar1 = math_mul(iVar1, param_3);
	if (iVar2 <= iVar1) {
		iVar1 = iVar2;
	}
	if (lane_grid < 1) {
		iVar1 = -iVar1;
	}

	return iVar1;
}

int FUN_007D55E(tnfs_car_data *car) {
	return 1;
}

int FUN_00080c50(tnfs_car_data *car) {
	return 0x400000;
}

void tnfs_ai_update_vectors(tnfs_car_data *car) {
	int iVar1;
	int iVar5;
	int iVar6;
	int iVar7;
	int iVar12;
	int iVar13;
	int iVar14;

	iVar5 = track_data[car->road_segment_a].num_lanes;
	if (car->is_crashed == 0) {
		tnfs_track_update_vectors(car);
		math_matrix_from_pitch_yaw_roll(&car->matrix, car->angle_x, car->angle_y, car->angle_z);
	}

	car->center_line_distance = ((car->road_fence_normal).x >> 8) * (((car->position).x - track_data[car->road_segment_a].pos.x) >> 8) //
		+ ((car->road_fence_normal).y >> 8) * (((car->position).y - track_data[car->road_segment_a].pos.y) >> 8) //
		+ ((car->road_fence_normal).z >> 8) * (((car->position).z - track_data[car->road_segment_a].pos.z) >> 8);

	iVar14 = (car->road_fence_normal).x >> 8;
	iVar13 = (car->road_fence_normal).y >> 8;
	iVar12 = (car->road_fence_normal).z >> 8;

	iVar1 = iVar14 * ((car->matrix).ax >> 8) + iVar13 * ((car->matrix).ay >> 8) + iVar12 * ((car->matrix).az >> 8);
	if (iVar1 < 0) {
		iVar1 = -iVar1;
	}
	iVar6 = iVar14 * ((car->matrix).bx >> 8) + iVar13 * ((car->matrix).by >> 8) + iVar12 * ((car->matrix).bz >> 8);
	if (iVar6 < 0) {
		iVar6 = -iVar6;
	}
	iVar7 = iVar14 * ((car->matrix).cx >> 8) + iVar13 * ((car->matrix).cy >> 8) + iVar12 * ((car->matrix).cz >> 8);
	if (iVar7 < 0) {
		iVar7 = -iVar7;
	}

	car->side_width = (iVar1 >> 8) * ((car->collision_data.size).x >> 8) //
					+ (iVar6 >> 8) * ((car->collision_data.size).y >> 8) //
					+ (iVar7 >> 8) * ((car->collision_data.size).z >> 8);

	if ((car->field_4e1 < 0) || (g_number_of_players <= car->field_4e1)) {
		if ((car->field_174 & 0x1000) == 0) {
			if ((car->field_174 & 4) == 0) {
				iVar1 = (DAT_001651BC >> 1) + (player_car_ptr->car_road_speed >> 1);
				car->speed_target = iVar1;
				iVar1 = FUN_00080a75(car, iVar1);
				car->speed_target = iVar1;
			} else {
				car->speed_target = player_car_ptr->car_road_speed;
			}
			if ((car->field_174 & 0x404) == 0) {
				iVar5 = FUN_00080a75(car, ((car->road_segment_a >> 2) * 3 + DAT_000FDB8C + 2) << 0x10);
				if (iVar5 < car->speed_target) {
					car->speed_target = iVar5;
				}
			} else {
				if (car->speed_target < player_car_ptr->car_road_speed) {
					car->speed_target = player_car_ptr->car_road_speed;
				}
				iVar5 = car->road_segment_b - iVar5;
				if (((car->field_174 & 0x408) == 0x408)
						&& ((3 < iVar5 || (((-3 < iVar5 && (DAT_0016533c < 900)) && ((((car->road_segment_a >> 2) * 3 + DAT_000FDB8C + 2) << 0x10) < player_car_ptr->car_road_speed)))))) {
					car->speed_target = player_car_ptr->speed;
				}
			}
			iVar5 = FUN_00080c50(car);
			if ((iVar5 < car->speed_target) || ((car->field_174 & 4) != 0)) {
				car->speed_target = iVar5;
			}
		} else {
			iVar5 = FUN_00077c58(car);
			iVar5 = FUN_00080a75(car, -iVar5);
			car->speed_target = iVar5;
		}
	}

	iVar5 = car->field_4e1;
	if (iVar5 != 3) {
		if (iVar5 == 4) {
			iVar5 = FUN_00047121(car);
			//car->car_road_speed = iVar5;
		} else {
			iVar5 = FUN_0007bbfe(car);
			//car->car_road_speed = iVar5;
		}
	}

	if ((car->field_174 & 0x20000) != 0) {
		car->speed_target = 0;
	}
	car->field_174 = (car->field_174 & 0xffffbffc) | 0x1e0;
}

void tnfs_ai_drive_car(tnfs_car_data *car, int curr_state) {
	int steer_speed;
	int lane;
	int iVar3;
	tnfs_vec3 local_position;
	tnfs_vec3 right_normal;
	tnfs_vec3 forward_vector;
	int iVar4;
	int iVar44;
	int next_state;
	int lane_change_speed;
	int accel;
	int curve_deccel;
	int speed;
	int iVar10;
	int iVar13;
	int centerline;
	int lateral_speed;
	int iVar14;
	int iVar15;
	int ground_height;
	char floor_it;
	int iVar18;
	int iVar19;
	char ai_type;

	ai_type = (car->field_174 & 4) != 0;

	iVar3 = abs(car->car_road_speed >> 0x10);
	iVar44 = iVar3 + 1;
	if (iVar3 > 99)
		iVar44 = 99;

	iVar14 = car->power_curve[iVar44];

	if (iVar44)
		iVar15 = car->power_curve[iVar44 - 1];
	else
		iVar15 = 0x640000;

	if (ai_type
			&& (player_car_ptr->speed >= 0x20000 || player_car_ptr->field_4e1 != 2)
			&& player_car_ptr->field_4e1 != 4) {

		iVar18 = (car->road_segment_b - player_car_ptr->road_segment_b) / 10 + 10;
		if (iVar18 < 0)
			iVar18 = 0;
		if (iVar18 > 20)
			iVar18 = 20;

		if (DAT_0016709F == 1)
			iVar19 = DAT_00165278;
		else
			iVar19 = DAT_001652CC;

		iVar14 = math_mul(iVar19, iVar14);
		iVar15 = math_mul(iVar19, iVar15);
	} else {
		iVar19 = 0x10000;
	}

	if (ai_type)
		iVar4 = DAT_00165320;
	else
		iVar4 = 0xb333;

	if (!car->wheels_on_ground) {
		iVar14 = 0;
		iVar4 = 0;
	}
	if (g_game_time < 310)
		iVar14 = 0;

	if (car->field_174 & 8) {
		next_state = 6;
		lane_change_speed = 0x2AAA;
	} else if (car->field_174 & 4) {
		next_state = 4;
		lane_change_speed = 0x4000;
	} else {
		next_state = 8;
		lane_change_speed = 0x2000;
	}
	accel = next_state - curr_state;

	// deccelerate a bit on curves
	if (abs(car->steer_angle - car->target_angle) > 0x60000 && car->car_road_speed > 0x70000) {
		if (ai_type) {
			curve_deccel = (abs(car->steer_angle - car->target_angle) >> 16) * 0xCCCC;
		} else {
			curve_deccel = (abs(car->steer_angle - car->target_angle) >> 16) * 0x1745;
		}
	} else {
		curve_deccel = 0;
	}

	// floor it or maintain speed
	floor_it = FUN_0076FB9(car->road_segment_a) == 0 //
		|| car->center_line_distance < track_data[car->road_segment_a].roadLeftFence * -0x2000 //
		|| car->center_line_distance > track_data[car->road_segment_a].roadRightFence * 0x2000 //
		|| car->collision_data.field_08c > 0;

	if (car->collision_data.field_08c > 0) {
		car->collision_data.field_08c--;
		if (car->collision_data.field_08c == 0) {
			car->collision_data.field_08c = -20;
		}
	}
	if (car->collision_data.field_08c < 0)
		car->collision_data.field_08c++;

	if ((car->field_174 & 0x1000) != 0) {
		if (car->speed_target <= car->car_road_speed) {
			if (iVar15) {
				if (floor_it) {
					car->car_road_speed -= accel * iVar14;
					if (car->car_road_speed < car->speed_target)
						car->car_road_speed = car->speed_target;
				}
			} else {
				car->car_road_speed += accel * iVar4;
			}
		} else {
			car->car_road_speed += accel * iVar4;
			if (car->car_road_speed > car->speed_target)
				car->car_road_speed = car->speed_target;
		}
	} else {
		if (car->speed_target >= car->car_road_speed) {
			if (iVar15) {
				if (floor_it) {
					car->car_road_speed += (accel * iVar14);

					if (car->car_road_speed > car->speed_target)
						car->car_road_speed = car->speed_target;
					if (ai_type) {
						car->brake -= 4;
						if (car->brake < 0)
							car->brake = 0;
					}
				}
			} else {
				car->car_road_speed -= accel * iVar4;
			}
		} else {
			car->car_road_speed -= accel * iVar4;
			if (car->car_road_speed <= car->speed_target)
				car->car_road_speed = car->speed_target;
			if (ai_type && car->car_data_ptr->field_4e1 == 3 && car->car_road_speed - 0xa0000 > car->speed_target) {
				car->brake = 0x11;
			}
		}
	}

	if (curve_deccel) {
		if (car->car_road_speed > 0) {
			car->car_road_speed -= curve_deccel;
		} else if (car->car_road_speed < 0) {
			car->car_road_speed += curve_deccel;
		} else {
			curve_deccel = 0;
		}
	}

	/* ????
	 if (car->car_road_speed > DAT_00165508[116 * *(&car->car_specs_ptr + 1)]
	 && (DAT_00FDB8C + 3 * (car->road_segment_a >> 2)) << 16 >= car->car_road_speed) {
	 DAT_00165508[116 * *(&car->car_specs_ptr + 1)] = car->car_road_speed;
	 }
	 */

	right_normal.x = car->road_fence_normal.x;
	right_normal.y = car->road_fence_normal.y;
	right_normal.z = car->road_fence_normal.z;

	if (car->collision_data.field_088 && (car->collision_data.field_088 != (ai_type ? 160 : 100) //
	|| abs(car->steer_angle - car->target_angle) < 0x20000)) {
		forward_vector.x = car->road_heading.x;
		forward_vector.y = car->road_heading.y;
		forward_vector.z = car->road_heading.z;
	} else {
		forward_vector.x = car->matrix.cx;
		forward_vector.y = car->matrix.cy;
		forward_vector.z = car->matrix.cz;
		if ((car->field_174 & 0x1000) != 0) {
			forward_vector.x = -forward_vector.x;
			forward_vector.y = -forward_vector.y;
			forward_vector.z = -forward_vector.z;
		}
	}

	speed = car->car_road_speed;
	forward_vector.x = fixmul(forward_vector.x, speed);
	forward_vector.y = fixmul(forward_vector.y, speed);
	forward_vector.z = fixmul(forward_vector.z, speed);

	//DAT_00165340 = DAT_00164FDC + ((track_data[car->road_segment_a].roadRightMargin >> 3) * 2) //
	//				+ (track_data[car->road_segment_a].num_lanes & 0xf) * 0x50;
	//DAT_00165340 = DAT_00165340 * 0x100;
	DAT_00165340 = (track_data[car->road_segment_a].roadRightMargin >> 3) * 0x10000;

	if (car->center_line_distance >= 3 * DAT_00165340) {
		lane = 7;
	} else if (car->center_line_distance >= 2 * DAT_00165340) {
		lane = 6;
	} else if (car->center_line_distance >= DAT_00165340) {
		lane = 5;
	} else if (car->center_line_distance >= 0) {
		lane = 4;
	} else if (car->center_line_distance >= -DAT_00165340) {
		lane = 3;
	} else if (car->center_line_distance >= -2 * DAT_00165340) {
		lane = 2;
	} else {
		lane = 1;
	}

	centerline = car->target_center_line - car->center_line_distance;

	if (3 - ((track_data[car->road_segment_a].num_lanes >> 4) & 0xF) == lane //
	|| ((track_data[car->road_segment_a].num_lanes & 0xF) + 4 == lane //
	|| (centerline && abs(car->steer_angle - car->target_angle) < 0x80000))) {

		iVar10 = FUN_007E87B(car, centerline, abs(speed));

		if (abs(next_state * iVar10) >> 5 > abs(centerline))
			iVar10 = fixmul(centerline, lane_change_speed) * 0x20;

		lateral_speed = car->speed_local_lat;
		if (car->speed_local_lat >= iVar10) {
			if (lateral_speed > iVar10) {
				if (lateral_speed >= 0) {
					iVar13 = 0xB333;
				} else {
					iVar13 = abs(iVar10) >> 4;
				}
				lateral_speed -= accel * iVar13;
				if (lateral_speed < iVar10)
					lateral_speed = iVar10;
			}
		} else {
			if (lateral_speed <= 0) {
				iVar13 = 0xB333;
			} else {
				iVar13 = abs(iVar10) >> 4;
			}
			lateral_speed += accel * iVar13;
			if (lateral_speed > iVar10)
				lateral_speed = iVar10;
		}
		car->speed_local_lat = lateral_speed;

		right_normal.x = math_mul(right_normal.x, car->speed_local_lat);
		right_normal.y = math_mul(right_normal.y, car->speed_local_lat);
		right_normal.z = math_mul(right_normal.z, car->speed_local_lat);

		if ((car->field_174 & 0x1000) == 0) {
			switch ((lateral_speed >> 0x10) / 5) {
			case 0:
				steer_speed = 0x10000;
				break;
			case 1:
				steer_speed = 0x18ccc;
				break;
			case 2:
			case 3:
				steer_speed = 0x15999;
				break;
			case 4:
			case 5:
				steer_speed = 0x11999;
				break;
			case 6:
			case 7:
				steer_speed = 0xcccc;
				break;
			case 8:
			case 9:
				steer_speed = 0x8000;
				break;
			case 10:
			case 0xb:
				steer_speed = 0x4ccc;
				break;
			default:
				steer_speed = 0;
			}
			car->target_angle = math_mul(car->speed_local_lat, steer_speed);
		} else {
			car->target_angle = 0;
		}

		forward_vector.x += right_normal.x;
		forward_vector.y += right_normal.y;
		forward_vector.z += right_normal.z;
	}

	local_position.x = car->position.x - car->road_position.x;
	local_position.y = car->position.y - car->road_position.y;
	local_position.z = car->position.z - car->road_position.z;

	// position car above road (again?)
	ground_height = math_vec3_dot(&local_position, &car->road_surface_normal);
	if (ground_height > 0x667
			&& FUN_007D55E(car)
			&& car->road_segment_b > 100 // wut??
			&& (car->field_174 & 4)
			&& car->car_road_speed > 0x1b0001) {
		if (car->wheels_on_ground)
			car->wheels_on_ground = 0;
	} else {
		if (!car->wheels_on_ground)
			car->wheels_on_ground = 1;
		car->position.x -= (car->road_surface_normal.x >> 8) * (ground_height >> 8);
		car->position.y -= (car->road_surface_normal.y >> 8) * (ground_height >> 8);
		car->position.z -= (car->road_surface_normal.z >> 8) * (ground_height >> 8);
	}

	// speed vector
	if (car->wheels_on_ground) {
		if (car->collision_data.field_088 && car->collision_data.field_088 != (ai_type ? 160 : 100)) {
			car->speed_y = forward_vector.y;
		} else {
			car->speed_x = forward_vector.x;
			car->speed_y = forward_vector.y;
			car->speed_z = forward_vector.z;
			car->speed_x = -car->speed_x;
		}
	}

	car->collision_data.crash_time_ai_state = next_state;
}

void tnfs_ai_main(tnfs_car_data *car) {
	signed int ground_height;
	tnfs_vec3 local_position;
	int lane;
	int max_angular_speed;
	int *angular_speed;
	int rotation;
	int max_steer;
	tnfs_vec9 rot_matrix;
	char ai_flag_4;
	int state;
	int speed_x;
	int speed_z;

	ai_flag_4 = (car->field_174 & 4) != 0;
	state = car->collision_data.crash_time_ai_state;

	if (ai_flag_4) {
		speed_x = abs(car->speed_x);
		speed_z = abs(car->speed_z);
		if (speed_x <= speed_z)
			car->speed = (speed_x >> 2) + speed_z;
		else
			car->speed = (speed_z >> 2) + speed_x;
	}

	car->is_crashed = 1;
	if (!ai_flag_4 || car->road_segment_b < 2)
		FUN_0077121(car);

	// segment change
	if (tnfs_road_segment_update(car)) {
		tnfs_track_update_vectors(car);
		// body rotation matrix
		if (car->field_174 & 0x1000) {
			car->matrix.ax = -car->road_fence_normal.x;
			car->matrix.ay = -car->road_fence_normal.y;
			car->matrix.az = -car->road_fence_normal.z;
			car->matrix.bx = car->road_surface_normal.x;
			car->matrix.by = car->road_surface_normal.y;
			car->matrix.bz = car->road_surface_normal.z;
			car->matrix.cx = -car->road_heading.x;
			car->matrix.cy = -car->road_heading.y;
			car->matrix.cz = -car->road_heading.z;
		} else {
			memcpy(&car->matrix, &car->road_fence_normal, sizeof(car->matrix));
		}
		// turning angle, when changing lanes
		if (car->steer_angle) {
			math_matrix_set_rot_Y(&rot_matrix, car->steer_angle);
			math_matrix_multiply(&car->matrix, &rot_matrix, &car->matrix);
		}
		car->collision_data.crash_time_ai_state = 0;
	}

	if (car->steer_angle) {
		car->slide_rear = 0;
		car->slide_front = 0;

		if (!FUN_0076FB9(car->road_segment_a)) {
			car->steer_angle = 0;
			car->target_angle = 0;
			car->angular_speed = 0;
		}

		if (abs(car->steer_angle - car->target_angle) > 0x90000 && abs(car->car_road_speed) > 0x90000) {

			//DAT_00165340 = DAT_00164FDC
			//		+ ((track_data[car->road_segment_a].num_lanes & 0xF) * 0x50) //
			//		+ (2 * (track_data[car->road_segment_a].roadRightMargin >> 3));
			DAT_00165340 = (track_data[car->road_segment_a].roadRightMargin >> 3) * 0x10000;

			// FIXME not sure - lane ids
			if (car->center_line_distance > DAT_00165340 * 0x300) {
				lane = 7;
			} else if (car->center_line_distance > DAT_00165340 * 0x200) {
				lane = 6;
			} else if (car->center_line_distance > DAT_00165340 * 0x100) {
				lane = 5;
			} else if (car->center_line_distance > 0) {
				lane = 4;
			} else if (car->center_line_distance > DAT_00165340 * -0x100) {
				lane = 3;
			} else if (car->center_line_distance > DAT_00165340 * -0x200) {
				lane = 2;
			} else if (car->center_line_distance > DAT_00165340 * -0x300) {
				lane = 1;
			} else {
				lane = 0;
			}

			if (lane != 3 - (track_data[car->road_segment_a].num_lanes >> 4)) {
				if ((track_data[car->road_segment_a].num_lanes & 0xf) + 4 <= lane) {
					car->slide_rear = 0x401;
				} else {
					car->slide_rear = 0x408;
				}
				car->slide_front = abs(car->steer_angle - car->target_angle) * 0x14;
			} else {
				car->slide_rear = 0;
				car->slide_front = 0;
			}
		}

		if (car->field_174 & 0x404)
			max_steer = 0x200000;
		else
			max_steer = 0x160000;

		if (abs(car->steer_angle) > max_steer && (car->field_174 & 0x200000) == 0) {
			FUN_0044E11(car->car_data_ptr);
			if (car->field_174 & 8) {
				if (FUN_0076FB9(car->road_segment_a))
					tnfs_replay_highlight_record(0x52);
				FUN_00776EA(0);
			}
		}

	} else {
		car->slide_rear = 0;
		car->slide_front = 0;
	}

	if (car->steer_angle != car->target_angle && abs(car->steer_angle) < 0xF0000 && car->wheels_on_ground) {
		if (car->collision_data.field_088)
			car->collision_data.field_088--;
	} else {
		car->collision_data.field_088 = ai_flag_4 ? 160 : 100;
	}

	if (car->road_segment_b < 150) {
		car->angular_speed = 0;
		car->steer_angle = 0;
		car->target_angle = 0;
	}

	// auto steer
	if (car->angular_speed) {
		if (abs(car->angular_speed) > 0x12C0000 || abs(car->steer_angle) > 0x12C0000) {
			car->steer_angle = 0;
			car->angular_speed = car->steer_angle;
		}
		rotation = math_mul(car->angular_speed, 0x444);
		car->steer_angle += rotation;
		math_matrix_set_rot_Y(&rot_matrix, rotation);
		math_matrix_multiply(&car->matrix, &rot_matrix, &car->matrix);
	}
	if (car->steer_angle >= car->target_angle) {
		if (car->steer_angle <= car->target_angle) {
			if (abs(car->steer_angle) < abs(car->target_angle) + 0x20000) {
				car->angular_speed = 0;
				car->steer_angle = car->target_angle;
			}
		} else if (car->angular_speed <= 0) {
			if (abs(car->car_road_speed) > 0x30000) {
				car->angular_speed = 9 * car->steer_angle;
				car->angular_speed = -car->angular_speed;
				if (car->target_angle + 0x30000 > car->steer_angle) {
					car->steer_angle = car->target_angle;
					car->angular_speed = 0;
				}
			}
		} else {
			angular_speed = &car->angular_speed;
			if (ai_flag_4)
				max_angular_speed = 0x23333;
			else
				max_angular_speed = 0x13333;
			*angular_speed -= max_angular_speed;
		}
	} else if (car->angular_speed >= 0) {
		if (abs(car->car_road_speed) > 0x30000) {
			car->angular_speed = 9 * car->steer_angle;
			car->angular_speed = -car->angular_speed;
			if (car->target_angle - 0x30000 < car->steer_angle) {
				car->steer_angle = car->target_angle;
				car->angular_speed = 0;
			}
		}
	} else {
		angular_speed = &car->angular_speed;
		if (ai_flag_4)
			max_angular_speed = 0x23333;
		else
			max_angular_speed = 0x13333;
		*angular_speed += max_angular_speed;
	}

	if (!car->collision_data.crash_time_ai_state)
		tnfs_ai_drive_car(car, car->collision_data.crash_time_ai_state);

	car->collision_data.crash_time_ai_state--;

	if (!DAT_0014DCC4 && FUN_0076FB9(car->road_segment_a)) {

		//DAT_00165340 = DAT_00164FDC
		//		+ ((track_data[car->road_segment_a].num_lanes & 0xF) * 0x50)
		//		+ (2 * track_data[car->road_segment_a].roadRightMargin);
		DAT_00165340 = (track_data[car->road_segment_a].roadRightMargin >> 3) * 0x10000;

		// FIXME not sure
		if (car->center_line_distance > 3 * DAT_00165340) {
			lane = 7;
		} else if (car->center_line_distance > 2 * DAT_00165340) {
			lane = 6;
		} else if (car->center_line_distance > DAT_00165340) {
			lane = 5;
		} else if (car->center_line_distance > 0) {
			lane = 4;
		} else if (car->center_line_distance > -DAT_00165340) {
			lane = 3;
		} else if (car->center_line_distance > -2 * DAT_00165340) {
			lane = 2;
		} else {
			lane = 1;
		}

		if (3 - (track_data[car->road_segment_a].num_lanes >> 4) != lane) {

			//DAT_00165340 = DAT_00164FDC
			//		+ ((track_data[car->road_segment_a].num_lanes & 0xF) * 0x50)
			//		+ (2 * track_data[car->road_segment_a].roadRightMargin);

			if (car->center_line_distance > DAT_00165340 * 0x300) {
				lane = 7;
			} else if (car->center_line_distance > DAT_00165340 * 0x200) {
				lane = 6;
			} else if (car->center_line_distance > DAT_00165340 * 0x100) {
				lane = 5;
			} else if (car->center_line_distance > 0) {
				lane = 4;
			} else if (car->center_line_distance > DAT_00165340 * -0x100) {
				lane = 3;
			} else if (car->center_line_distance > DAT_00165340 * -0x200) {
				lane = 2;
			} else if (car->center_line_distance > DAT_00165340 * -0x300) {
				lane = 1;
			}

			if ((track_data[car->road_segment_a].num_lanes & 0xf) + 4 != lane) {
				if (car->road_segment_b > 70)
					car->car_road_speed = 255 * (car->car_road_speed >> 8);
			}
		}

	}

	// stop the car
	if (car->center_line_distance < (int)track_data[car->road_segment_a].roadLeftFence * -0x2000 //
		|| car->center_line_distance > (int)track_data[car->road_segment_a].roadLeftFence * 0x2000) {
		if (FUN_0076FB9(car->road_segment_a)) {
			if (car->field_33c >= -0x40000) {
				FUN_007BC96(car);
			} else {
				car->speed_target = 0;
				car->car_road_speed = 0;
				car->speed_x = 0;
				car->speed_y = 0;
				car->speed_z = 0;
			}
		} else {
			car->target_angle = 0;
			car->steer_angle = 0;
		}
	}


	// apply gravity
	if (!car->wheels_on_ground) {
		car->speed_y -= 0x6838;
	}

	// move car
	car->position.x -= (car->speed_x >> 9) + (car->speed_x >> 5);
	car->position.y += (car->speed_y >> 9) + (car->speed_y >> 5);
	car->position.z += (car->speed_z >> 9) + (car->speed_z >> 5);
	car->position.x -= (car->speed_x >> 11) + (car->speed_x >> 7);
	car->position.y += (car->speed_y >> 7) + (car->speed_y >> 11);
	car->position.z += (car->speed_z >> 7) + (car->speed_z >> 11);

	// position car above road
	if (car->collision_data.field_088 && car->collision_data.field_088 != (ai_flag_4 ? 160 : 100)) {
		local_position.x = car->position.x - car->road_position.x;
		local_position.y = car->position.y - car->road_position.y;
		local_position.z = car->position.z - car->road_position.z;
		ground_height = math_vec3_dot(&local_position, &car->road_surface_normal);
		car->position.x -= fixmul(car->road_surface_normal.x, ground_height);
		car->position.y -= fixmul(car->road_surface_normal.y, ground_height);
		car->position.z -= fixmul(car->road_surface_normal.z, ground_height);
	}
}


void FUN_00078545(tnfs_car_data *car1, char lane_grid[8], int lane, int param_4, tnfs_vec3 *speedA, tnfs_vec3 *speedB, tnfs_car_data *car2) {
	int lVar2;
	int local_a4;
	int speed_diff;
	int seg_distance;
	signed int local_64;
	int carId;
	tnfs_car_data *car;
	int local_1c;
	int i, j;
	int aiCarID;

	aiCarID = 0;
	for (i = 0; i < g_total_cars_in_scene; ++i) {
		if (car1 == g_car_ptr_array[i]) {
			aiCarID = i;
		}
	}

	if (car1->field_174 & 0x1000) {
		j = g_total_cars_in_scene;
		local_1c = 0;
		while (local_1c != 3 && j > 0) {
			carId = (j + aiCarID) % g_total_cars_in_scene;
			car = g_car_ptr_array[carId];
			if ((car1 != car) && (car->field_4e9 & 4) && (car->field_4e1 != 6)) {
				seg_distance = car->road_segment_a - car1->road_segment_a;
		        if ((seg_distance < 0) && (0 < car->road_segment_b - car1->road_segment_b)) {
		        	seg_distance = -seg_distance;
		        }
				if (seg_distance <= 0 && -seg_distance < 27) {
					seg_distance = seg_distance + 2;
					if (((1 << lane) & lane_grid[carId]) //
					&& ((param_4 & 1 << (lane & 0x1f)) == 0)) {
						speedB->y = 0x1b6db * seg_distance + car->car_road_speed;
						speedA->y = 0x50000 * seg_distance + car->car_road_speed;
						param_4 |= 1 << lane;
						//car2->position.y = car;
						local_1c++;
					}
					if (lane > 0 //
					&& ((1 << (lane - 1)) & lane_grid[carId]) //
							&& ((param_4 & 1 << ((lane - 1) & 0x1f)) == 0)) {
						speedB->x = 0x1b6db * seg_distance + car->car_road_speed;
						speedA->x = 0x50000 * seg_distance + car->car_road_speed;
						param_4 |= 1 << (lane - 1);
						//car2->position.x = car;
						local_1c++;
					}
					if (lane < 7 //
					&& ((1 << (lane + 1)) & lane_grid[carId]) //
							&& ((param_4 & 1 << ((lane + 1) & 0x1f)) == 0)) {
						speedB->z = 0x1b6db * seg_distance + car->car_road_speed;
						speedA->z = 0x50000 * seg_distance + car->car_road_speed;
						param_4 |= 1 << (lane + 1);
						//car2->position.z = car;
						local_1c++;
					}
				}
			}
			--j;
		}
		if ((car2->position.y != 0) //
				&& (DAT_001670B3 + DAT_001670AB <= car1->field_4e5) //
				&& (DAT_001670B3 + DAT_001670AB + DAT_001670BB > car1->field_4e5) //
				&& ((car1->road_segment_a - car2->road_segment_a) < 3)) {
			speedA->y >>= 1;
			speedB->y >>= 1;
			if (car2->speed_target <= car1->speed_target) {
				speedA->x >>= 1;
				speedB->x >>= 1;
				speedA->z >>= 1;
				speedB->z >>= 1;
			}
		}
	} else {
		i = 1;
		local_1c = 0;
		while (local_1c != 3 && i < g_total_cars_in_scene) {
			carId = (i + aiCarID) % g_total_cars_in_scene;
			car = g_car_ptr_array[carId];
			if (car1 != car //
			&& (car->field_4e9 & 4) != 0 //
			&& car->field_4e1 != 6) {
				seg_distance = car->road_segment_a - car1->road_segment_a;

				if (abs(seg_distance) > 1 //
				|| ((car1->road_heading.z >> 8) * ((car->position.z - car1->position.z) >> 8) + ((car->position.x - car1->position.x) >> 8) * (car1->road_heading.x >> 8) > 0x4ffff)) {

			        if ((seg_distance < 0) && (0 < car->road_segment_b - car1->road_segment_b)) {
			        	seg_distance = -seg_distance;
			        }
					if ((car1->field_174 & 4) && (car->field_174 & 0x1000)) {
						local_64 = g_car_ptr_array[car1->field_4e5]->speed_local_lon;
					} else {
						local_64 = 27;
					}
					if (seg_distance > 0 && seg_distance < local_64) {
						seg_distance = seg_distance - 2;
						if ((car1->field_174 & 0x404) && seg_distance < 1) {
							seg_distance = 1;
						}
						if (((1 << lane) & lane_grid[carId]) //
						&& ((param_4 & 1 << (lane & 0x1f)) == 0)) {
							speedB->y = 0x1b6db * seg_distance + car->car_road_speed;
							speedA->y = 0x50000 * seg_distance + car->car_road_speed;
							param_4 |= 1 << lane;
							//car2->position.y = car;
							local_1c++;
						}
						if (lane > 0 //
						&& ((1 << (lane - 1)) & lane_grid[carId]) //
								&& ((param_4 & 1 << ((lane - 1) & 0x1f)) == 0)) {
							speedB->x = 0x1b6db * seg_distance + car->car_road_speed;
							speedA->x = 0x50000 * seg_distance + car->car_road_speed;
							param_4 |= 1 << (lane - 1);
							//car2->position.x = car;
							local_1c++;
						}
						if (lane < 7 //
						&& ((1 << (lane + 1)) & lane_grid[carId]) //
								&& ((param_4 & 1 << ((lane + 1) & 0x1f)) == 0)) {
							speedB->z = 0x1b6db * seg_distance + car->car_road_speed;
							speedA->z = 0x50000 * seg_distance + car->car_road_speed;
							param_4 |= 1 << (lane + 1);
							//car2->position.z = car;
							local_1c++;
						}
					}
				}
			}
			++i;
		}

		if ((DAT_001670B3 + DAT_001670AB <= car1->field_4e5) //
				&& (DAT_001670B3 + DAT_001670AB + DAT_001670BB > car1->field_4e5) //
				&& (car2->position.y) //
				&& (car2->road_segment_a - car1->road_segment_a < 3)) {
			speedA->y >>= 1;
			speedB->y >>= 1;
			if (car2->speed_target >= car1->speed_target) {
				speedA->x >>= 1;
				speedB->x >>= 1;
				speedA->z >>= 1;
				speedB->z >>= 1;
			}
		}
		if ((car1->field_4e5 >= 0 && (car1->field_4e5 < DAT_001670AB)) //
				&& (car2->position.y != 0) //
				&& ((car2->road_segment_a - car1->road_segment_a) < 2) //
				&& (car->car_road_speed > 0x140000)) {
			speed_diff = car->car_road_speed - (car->car_road_speed >> 3);
			if (speed_diff < 0) {
				speed_diff = 0x10000;
			}
			speedA->y = speed_diff;
			speedB->y = speed_diff;
		}
		if ((lane + 1) == ((track_data[DAT_0014DCCC & car1->road_segment_a].num_lanes & 0xF) + 4)) {
			speedA->z >>= 1;
			speedB->z >>= 1;
		} else if ((lane - 1) == (3 - (track_data[DAT_0014DCCC & car1->road_segment_a].num_lanes >> 4))) {
			speedA->x >>= 1;
			speedB->x >>= 1;
		}
	}


	if ((car1->car_road_speed > 0xa0000) //
			&& (car2->position.y != 0) //
			&& (car2->field_4e5 >= 0) //
			&& (car2->field_4e5 < DAT_001670AB)
			&& (speedA->y < speedA->x) //
			&& (speedA->y < speedA->z)) {

		seg_distance = abs(car1->road_segment_a - car2->road_segment_a);
		speed_diff = car1->car_road_speed - car2->car_road_speed;
		if (seg_distance > 2 && seg_distance < 15 && speed_diff > 0) {
			local_a4 = 0x33333; //FUN_0007e87b(car1, 1, car1->car_road_speed);
			local_a4 = abs(local_a4);
			local_a4 = math_mul(seg_distance * 0x60000, local_a4);
			lVar2 = math_mul(math_mul(0x50000, local_a4), 0x20000);
			if (local_a4 < lVar2) {
				speedA->y = 0;
				speedB->y = 0;
			}
		}
	}
}

void FUN_0007820e(tnfs_car_data *car, tnfs_vec3 *s0, tnfs_vec3 *s1, tnfs_vec3 *result) {
	int target;
	int speed;
	int i;

	if ((car->field_174 & 0x1000) == 0) {
		target = car->speed_target;
		speed = car->car_road_speed;

		for (i = 0; i < 3; i++) {
			if ((&s0->x)[i] < target) {
				(&result->x)[i] += ((target - (&s0->x)[i]) >> 8) * -0xcd;
			}
			if ((&s1->x)[i] < target) {
				(&result->x)[i] += ((target - (&result->x)[i]) >> 8) * -0x80;
			}
			if ((&s0->x)[i] < speed) {
				(&result->x)[i] += ((speed - (&s0->x)[i]) >> 8) * -0x100;
			}
			if ((&s1->x)[i] < speed) {
				(&result->x)[i] += ((speed - (&s1->x)[i]) >> 8) * -0x1a;
			}
		}
		if (s0->y < target) {
			car->speed_target = s0->y;
		}
	} else {
		target = car->speed_target;
		speed = car->car_road_speed;

		for (i = 0; i < 3; i++) {
			if (target < (&s0->x)[i]) {
				(&result->x)[i] += (((&s0->x)[i] - target) >> 8) * -0xcd;
			}
			if (target < (&s1->x)[i]) {
				(&result->x)[i] += (((&s1->x)[i] - target) >> 8) * -0x80;
			}
			if (speed < (&s0->x)[i]) {
				(&result->x)[i] += (((&s0->x)[i] - speed) >> 8) * -0x100;
			}
			if (speed < (&s1->x)[i]) {
				(&result->x)[i] += (((&s1->x)[i] - speed) >> 8) * -0x1a;
			}
		}
		if (target < s0->y) {
			car->speed_target = s0->y;
		}
	}
	//if (abs(car->speed_target) > 0x14ccc) { //???
	//	car->speed_target = 0x14ccc;
	//}
}



void tnfs_ai_lane_change() {
	int local_c0;
	tnfs_vec3 car_speed_b;
	tnfs_vec3 car_speed_a;
	unsigned int local_118;
	int segment;
	int local_c4;
	tnfs_vec3 change_lane_vector;
	int lane;
	int local_bc;
	int lane_right;
	int distance;
	int lane_left;
	int local_6c;
	char lane_grid[8];
	int i;
	tnfs_car_data *car;
	int iVar3;

	if (!DAT_000FDB94) {
		DAT_0016533C += 4;
		//sub_77350();

		// if a 2 player game, get the winning car
		local_6c = 0;
		for (i = 0; i < g_number_of_players; i++) {
			if (g_car_ptr_array[i]->road_segment_b > local_6c) {
				local_6c = g_car_ptr_array[i]->road_segment_b;
				DAT_00165334 = g_car_ptr_array[i];
				DAT_00165144 = i;
			}
		}
		//if (!(g_selected_cheat & 4)) {
		//	FUN_00078f5d();
		//}

		/* Populate the "lane_grid" array for all cars */

		for (i = 0; i < 8; i++) {
			lane_grid[i] = 0;
		}
		for(i = 0; i < g_total_cars_in_scene; i++) {
			car = g_car_ptr_array[i];

			if (car->field_4e1 != 6) {
				tnfs_ai_update_vectors(car);

				// left side
				distance = car->center_line_distance - car->side_width + 0x8000;
				//DAT_00165340 = DAT_00164FDC[(track_data[car->road_segment_a].roadRightMargin >> 3) + ((track_data[car->road_segment_a].num_lanes & 0xf) >> 4)];
				DAT_00165340 = (track_data[car->road_segment_a].roadRightMargin >> 3) * 0x10000;

				if (distance >= 3 * DAT_00165340) {
					lane_left = 7;
				} else if (distance >= 2 * DAT_00165340) {
					lane_left = 6;
				} else if (distance >= DAT_00165340) {
					lane_left = 5;
				} else if (distance >= 0) {
					lane_left = 4;
				} else if (distance >= -DAT_00165340) {
					lane_left = 3;
				} else if (distance >= -2 * DAT_00165340) {
					lane_left = 2;
				} else {
					lane_left = 1;
				}

				// right side
				distance = car->side_width + car->center_line_distance - 0x8000;
				//DAT_00165340 = DAT_00164FDC[(track_data[car->road_segment_a].roadRightMargin >> 3) + ((track_data[car->road_segment_a].num_lanes & 0xf) >> 4)];
				DAT_00165340 = (track_data[car->road_segment_a].roadRightMargin >> 3) * 0x10000;
				if (distance >= 3 * DAT_00165340) {
					lane_right = 7;
				} else if (distance >= 2 * DAT_00165340) {
					lane_right = 6;
				} else if (distance >= DAT_00165340) {
					lane_right = 5;
				} else if (distance >= 0) {
					lane_right = 4;
				} else if (distance >= -DAT_00165340) {
					lane_right = 3;
				} else if (distance >= -2 * DAT_00165340) {
					lane_right = 2;
				} else {
					lane_right = 1;
				}

				// each bit represents a blocked lane (8 bits = 8 lanes)
				if (lane_right == lane_left) {
					lane_grid[i] = 1 << lane_left;
					lane_grid[lane_right]++;
				} else if (lane_left + 1 == lane_right) {
					lane_grid[lane_right]++;
					lane_grid[lane_left]++;
					lane_grid[i] = 3 << lane_left;
				} else {
					lane_grid[lane_right]++;
					lane_grid[lane_left]++;
					lane_grid[lane_left + 1]++;
					lane_grid[i] = 7 << lane_left;
				}
			}
		}

		player_car_ptr->field_174 &= 0xF700;
		player_car_ptr->field_174 &= 0xFB00;

		// do lane changes
		i = 0;
		for(i = 0; i < g_total_cars_in_scene; i++) {
			car = g_car_ptr_array[i];

			if (car->field_4e1 != 6) {
				local_bc = 1; //FUN_00076fb9(car->road_segment_a);
				lane = 0;
				local_c4 = 0;

				if (((car->field_4e9 & 4) != 0) //
					&& (car->field_4e5 < 0 || car->field_4e5 <= g_number_of_players) //
					&& (car->field_4e1 != 4)) {

						// current lane
						//DAT_00165340 = DAT_000164FDC[(track_data[car->road_segment_a].roadRightMargin >> 3) + ((track_data[car->road_segment_a].num_lanes & 0xf) >> 4)] * 0x100;
						DAT_00165340 = (track_data[car->road_segment_a].roadRightMargin >> 3) * 0x10000;
						if (car->center_line_distance >= 3 * DAT_00165340) {
							lane = 7;
						} else if (car->center_line_distance >= 2 * DAT_00165340) {
							lane = 6;
						} else if (car->center_line_distance >= DAT_00165340) {
							lane = 5;
						} else if (car->center_line_distance >= 0) {
							lane = 4;
						} else if (car->center_line_distance >= -DAT_00165340) {
							lane = 3;
						} else if (car->center_line_distance >= -2 * DAT_00165340) {
							lane = 2;
						} else {
							lane = 1;
						}

						if (car->field_174 & 0x1000) {
							change_lane_vector.x = 0x10000;
							change_lane_vector.y = 0;
							change_lane_vector.z = -0x10000;
						} else {
							change_lane_vector.x = -0x10000;
							change_lane_vector.y = 0;
							change_lane_vector.z = 0;

							if ((DAT_0016709F > 1) && (car->road_segment_b < 100)) {
								if (//*(int *)(&DAT_00167179 + (int)_car->unknown_475 * 4) * 5 + 0x20) <= car->road_segment_b // ???
									((3 - ((track_data[car->road_segment_a].num_lanes >> 4) & 0xF) != lane)
									&& (track_data[car->road_segment_a].num_lanes & 0xF) + 4 != lane)) {
									change_lane_vector.y = 0x460000;
								}
							} else if ((DAT_0016709F == 1) && (car->field_174 & 4) && (car->road_segment_a < 60)) {
								if ((abs(car->road_segment_a - g_car_ptr_array[0]->road_segment_a) < 5) //
										&& (3 - ((track_data[car->road_segment_a].num_lanes >> 4) & 0xF) != lane) //
										&& ((track_data[car->road_segment_a].num_lanes & 0xF) + 4 != lane)) {
									change_lane_vector.y = 0x460000;
								}
							}
						}

						local_c4 = 0;

						//if (car->field_174 & 8) {
						//  FUN_00079235(car, local_d4);
						//}
						//if (car->field_174 & 4) {
						//  FUN_00079af9(car,lane);
						//}

						if ((car != player_car_ptr) //
								&& ((track_data[car->road_segment_a].num_lanes & 0xF) == (track_data[(car->road_segment_a + 11)].num_lanes & 0xF))
								&& lane_grid[lane] == 1 //
								&& (((car->field_174 & 0x1000) && lane < 4) || (((car->field_174 & 0x1000) != 0) && lane >= 4))
								&& (3 - ((track_data[car->road_segment_a].num_lanes >> 4) & 0xF) >= lane)
								&& (3 - ((track_data[car->road_segment_a].num_lanes >> 4) & 0xF) <= lane)
								&& (track_data[car->road_segment_a].num_lanes & 0xF) + 4 <= lane
								&& (track_data[car->road_segment_a].num_lanes & 0xF) + 4 >= lane
								&& (!(car->field_174 & 0x404) || (car->field_174 & 0x20000)) //
							) {
							car->target_center_line = ((lane - 4)
									* DAT_00164FDC[(track_data[car->road_segment_a].roadRightMargin >> 3) + ((track_data[car->road_segment_a].num_lanes & 0xf) >> 4)] * 0x100)
									+ ((DAT_00164FDC[(track_data[car->road_segment_a].roadRightMargin >> 3) + ((track_data[car->road_segment_a].num_lanes & 0xf) >> 4)] * 0x100)
											>> 1);
						} else {
							if (DAT_0016709F == 1) {
								if (car->field_174 & 0x1000) {
									if (lane - 1 >= 4)
										change_lane_vector.x -= 0x50000;
									if (lane >= 4)
										change_lane_vector.y -= 0x50000;
									if (lane + 1 >= 4)
										change_lane_vector.z -= 0x50000;
								} else {
									if (lane - 1 < 4)
										change_lane_vector.x -= 0x30000;
									if (lane < 4)
										change_lane_vector.y -= 0x30000;
									if (lane + 1 < 4)
										change_lane_vector.z -= 0x30000;
								}
							}
							if ((car->field_174 & 0x1000)
									|| (track_data[car->road_segment_a].num_lanes & 0xF) <= (track_data[(car->road_segment_a + 11)].num_lanes & 0xF)) {
								if ((car->field_174 & 0x1000) == 0 //
										&& car->road_segment_a > 11 //
										&& car->collision_data.field16_0x90 < 0x10000 //
										&& (track_data[car->road_segment_a].num_lanes & 0xF) > (track_data[(car->road_segment_a - 11)].num_lanes & 0xF)) {
									if (lane + 1 != 3 - ((track_data[car->road_segment_a].num_lanes >> 4) & 0xF)
											&& lane + 1 != (track_data[car->road_segment_a].num_lanes & 0xF) + 4) {
										change_lane_vector.z += 0x280000;
									} else {
										change_lane_vector.y += 0x280000;
									}
								}
							} else if (lane + 1 != 3 - ((track_data[car->road_segment_a].num_lanes >> 4) & 0xF)
									&& lane + 1 != (track_data[car->road_segment_a].num_lanes & 0xF) + 4) {
								if (lane + 2 == 3 - ((track_data[car->road_segment_a].num_lanes >> 4) & 0xF)
										|| lane + 2 == (track_data[car->road_segment_a].num_lanes & 0xF) + 4) {
									change_lane_vector.y += 0x280000;
								}
							} else {
								change_lane_vector.x += 0x280000;
							}
							segment = car->road_segment_a;
							if (car->field_174 & 4)
								local_118 = -0xf0000;
							else
								local_118 = -0x260000;
							if (lane - 1 == 3 - ((track_data[segment].num_lanes >> 4) & 0xF) || lane - 1 == (track_data[segment].num_lanes & 0xF) + 4) {
								change_lane_vector.x += local_118;
								if ((track_data[car->road_segment_a].roadLeftFence << 13) - (track_data[car->road_segment_a].roadLeftMargin << 13) < car->collision_data.size.x)
									change_lane_vector.x -= 0x640000;
							}
							if (3 - ((track_data[segment].num_lanes >> 4) & 0xF) == lane || (track_data[segment].num_lanes & 0xF) + 4 == lane) {
								change_lane_vector.y += local_118;
								if (lane >= 0) {
									if ((track_data[car->road_segment_a].roadRightFence << 13) - (track_data[car->road_segment_a].roadRightMargin << 13) < car->collision_data.size.x)
										change_lane_vector.y -= 0x640000;
								} else if ((track_data[car->road_segment_a].roadLeftFence << 13) - (track_data[car->road_segment_a].roadLeftMargin << 13)
										< car->collision_data.size.x) {
									change_lane_vector.y -= 0x640000;
								}
							}
							if (lane + 1 == 3 - ((track_data[segment].num_lanes >> 4) & 0xF) || lane + 1 == (track_data[segment].num_lanes & 0xF) + 4) {
								change_lane_vector.z += local_118;
								if ((track_data[car->road_segment_a].roadRightFence << 13) - (track_data[car->road_segment_a].roadRightMargin << 13) < car->collision_data.size.x)
									change_lane_vector.z -= 0x640000;
							}
							if (lane - 1 < 3 - ((track_data[segment].num_lanes >> 4) & 0xF) || lane - 1 > (track_data[segment].num_lanes & 0xF) + 4) {
								change_lane_vector.x -= 0x3E80000;
							}
							if (3 - ((track_data[segment].num_lanes >> 4) & 0xF) > lane || (track_data[segment].num_lanes & 0xF) + 4 < lane) {
								change_lane_vector.y -= 0x3E80000;
							}
							if (lane + 1 < 3 - ((track_data[segment].num_lanes >> 4) & 0xF) || lane + 1 > (track_data[segment].num_lanes & 0xF) + 4) {
								change_lane_vector.z -= 0x3E80000;
							}

							car_speed_a.x = car->speed_target;
							car_speed_a.y = car->speed_target;
							car_speed_a.z = car->speed_target;
							car_speed_b = car_speed_a;

							// functions used to avoid collision to other cars
							FUN_00078545(car, lane_grid, lane, local_c4, &car_speed_a, &car_speed_b, player_car_ptr);
							FUN_0007820e(car, &car_speed_a, &car_speed_b, &change_lane_vector);

							if ((car->field_174 & 0x1000) //
									&& (car != player_car_ptr) //
									&& (player_car_ptr->car_road_speed > 0xa0000)) {
								change_lane_vector.z -= 0x1e0000;
							}
							if (!(car->field_174 & 0x404) && !(car->field_174 & 8)) {
								if (car->field_174 & 0x1000) {
									if (local_bc) {
										if (car_speed_b.y > -196608) {
											DAT_001039D8 = DAT_001039DC * DAT_001039D4;
											DAT_001039D4 = DAT_001039DC * DAT_001039D4;
											//if ((((DAT_0001039D8 & 0xFFFF00u) >> 8) & 0xFF) < 35 && car->field_15c && !DAT_000F9BB0 )
											//FUN_00082DA5((&car->field_4e5), &car->field_461);
										}
									}
								}
							}

							// change target lane
							local_c0 = change_lane_vector.y;
							if (change_lane_vector.x > change_lane_vector.y) {
								lane = lane - 1;
								local_c0 = change_lane_vector.x;
							}
							if (change_lane_vector.z > local_c0)
								lane = lane + 1;

							car->target_center_line = ((lane - 4)
									* DAT_00164FDC[(track_data[car->road_segment_a].roadRightMargin >> 3) + ((track_data[car->road_segment_a].num_lanes & 0xf) >> 4)] * 0x100)
									+ ((DAT_00164FDC[(track_data[car->road_segment_a].roadRightMargin >> 3) + ((track_data[car->road_segment_a].num_lanes & 0xf) >> 4)] * 0x100)
											>> 1);

							// limit center line
							iVar3 = track_data[car->road_segment_a].roadRightFence * 0x2000 - (car->collision_data.size).x;
							if (iVar3 < car->target_center_line) {
								car->target_center_line = iVar3;
							}
							iVar3 = track_data[car->road_segment_a].roadLeftFence * -0x2000 + (car->collision_data.size).x;
							if (car->target_center_line < iVar3) {
								car->target_center_line = iVar3;
							}

							if (car->field_174 & 0x1000)
								car->target_center_line += car->field_33c;
							else
								car->target_center_line -= car->field_33c;

				            //if ((DAT_00165148 != 0) && (local_c0 != lane)) {
				            //  FUN_00077a05(car,&player_car, local_c0, &car_speed_a);
				            //}
				            //if (((car->field_174 & 4) != 0) && (car->road_segment_b < 0x41)) {
				            //  car->speed_target = ((0x10000 + 2016 * (65 - car->road_segment_b)) * car->speed_target + 0x8000) >> 16;
				            //}

					}
				}
			}
		}
	}
}


/*
 * minimal routines for the X-man driver
 */
void tnfs_ai_driver_update(tnfs_car_data *car) {
	tnfs_ai_update_vectors(car);
	tnfs_ai_lane_change();
	tnfs_ai_main(car);
	tnfs_track_fence_collision(car);
}
