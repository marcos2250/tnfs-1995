/*
 * AI driver
 */
#include "tnfs_math.h"
#include "tnfs_base.h"

tnfs_car_data *player_car_ptr;
int DAT_000fdb8c = 0;
int DAT_0014DCC4 = 0;
int DAT_0014dccc = 0;
int DAT_00164FDC[] = { 0, 0x100, 0x200, 0x300, 0x400, 0x500, 0x600, 0x700, 0x800, 0x900, 0xa000, 0xb00, 0xc00, 0xd00, 0xe00 };
int DAT_001651bc = 0;
int DAT_00165278 = 0x90000;
int DAT_001652CC = 0x90000;
int DAT_00165320 = 0x5999;
int DAT_0016533c = 0;
int DAT_00165340 = 0;
int DAT_0016709F = 1;
int DAT_001670af = 0;

void tnfs_ai_init() {
	player_car_ptr = &car_data;
	xman_car_data.speed_target = 0x300000;
	xman_car_data.field_174[0] = 0x1e4;
	xman_car_data.field_174[1] = 0x4f1a;
	xman_car_data.field_4e1 = 4;
	xman_car_data.field_088 = 0;
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
	return 1;
}

void FUN_0077121(tnfs_car_data *car) {
	// stub
}

void FUN_00776EA() {
	// stub
}

int FUN_00077c58(tnfs_car_data *param_1) {
	return 0x850A4; //0x2c51e-0x136666
}

int FUN_00080a75(tnfs_car_data *car, int speed) {
	return 0x24000; //0x8e666-0xd6666
}

int FUN_0007bbfe(tnfs_car_data *car) {
	return (-car->speed_x >> 8) * ((car->road_heading).x >> 8) //
			+ ((car->road_heading).y >> 8) * (car->speed_y >> 8) //
			+ ((car->road_heading).z >> 8) * (car->speed_z >> 8);
}

void FUN_007BC96(tnfs_car_data *car) {
	// stub
}

int FUN_007E87B(tnfs_car_data *car, int param_2, int param_3) {
	int iVar1;
	int iVar2;

	if ((car->field_174[0] & 4) == 0) {
		iVar2 = 0x33333;
	} else {
		iVar2 = 0x3cccc; // FIXME not a constant
	}
	if ((car->field_174[0] & 4) == 0) {
		iVar1 = 0x4000;
	} else {
		iVar1 = 0x4ccc; // FIXME not a constant
	}
	iVar1 = math_mul(iVar1, param_3);
	if (iVar2 <= iVar1) {
		iVar1 = iVar2;
	}
	if (param_2 < 1) {
		iVar1 = -iVar1;
	}
	if (iVar1 > 0x20000) iVar1 = 0x20000;
	if (iVar1 < -0x20000) iVar1 = -0x20000;
	return iVar1;
}

int FUN_007D55E(tnfs_car_data *car) {
	return 1;
}

int FUN_00080c50(tnfs_car_data *car) {
	return 0x300000;
}

void tnfs_ai_update_vectors(tnfs_car_data *car) {
	int iVar1;
	int iVar5;
	int iVar6;
	int iVar7;
	int iVar8;
	int iVar9;
	int iVar10;
	int iVar12;
	int iVar13;
	int iVar14;

	iVar5 = track_data[car->road_segment_a].num_lanes;
	iVar6 = track_data[car->road_segment_a].pos.x;
	iVar8 = track_data[car->road_segment_a].pos.y;
	iVar10 = track_data[car->road_segment_a].pos.z;
	iVar1 = (car->position).x;
	iVar7 = (car->position).y;
	iVar9 = (car->position).z;
	if (car->is_crashed == 0) {
		tnfs_track_update_vectors(car);
		math_matrix_from_pitch_yaw_roll(&car->matrix, car->angle_x, car->angle_z, car->angle_y);
	}
	iVar14 = (car->road_fence_normal).x >> 8;
	iVar13 = (car->road_fence_normal).y >> 8;
	iVar12 = (car->road_fence_normal).z >> 8;

	car->center_line_distance = ((car->road_fence_normal).x >> 8) * ((iVar1 - iVar6) >> 8) //
		+ ((car->road_fence_normal).y >> 8) * ((iVar7 - iVar8) >> 8) //
		+ ((car->road_fence_normal).z >> 8) * ((iVar9 - iVar10) >> 8);

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

	car->angle_084 = (iVar1 >> 8) * ((car->collision_data.size).x >> 8) //
					+ (iVar6 >> 8) * ((car->collision_data.size).y >> 8) //
					+ (iVar7 >> 8) * ((car->collision_data.size).z >> 8);

	if ((car->field444_0x520 < 0) || (DAT_001670af <= car->field444_0x520)) {
		if ((car->field_174[0] & 0x1000) == 0) {
			if ((car->field_174[0] & 4) == 0) {
				iVar1 = (DAT_001651bc >> 1) + (player_car_ptr->car_road_speed >> 1);
				car->speed_target = iVar1;
				iVar1 = FUN_00080a75(car, iVar1);
				car->speed_target = iVar1;
			} else {
				car->speed_target = player_car_ptr->car_road_speed;
			}
			if ((car->field_174[0] & 0x404) == 0) {
				iVar5 = FUN_00080a75(car, ((car->road_segment_a >> 2) * 3 + DAT_000fdb8c + 2) << 0x10);
				if (iVar5 < car->speed_target) {
					car->speed_target = iVar5;
				}
			} else {
				if (car->speed_target < player_car_ptr->car_road_speed) {
					car->speed_target = player_car_ptr->car_road_speed;
				}
				iVar5 = car->road_segment_b - iVar5;
				if (((car->field_174[0] & 0x408) == 0x408)
						&& ((3 < iVar5 || (((-3 < iVar5 && (DAT_0016533c < 900)) && ((((car->road_segment_a >> 2) * 3 + DAT_000fdb8c + 2) << 0x10) < player_car_ptr->car_road_speed)))))) {
					car->speed_target = player_car_ptr->speed;
				}
			}
			iVar5 = FUN_00080c50(car);
			if ((iVar5 < car->speed_target) || ((car->field_174[0] & 4) != 0)) {
				car->speed_target = iVar5;
			}
		} else {
			iVar5 = FUN_00077c58(car);
			iVar5 = FUN_00080a75(car, -iVar5);
			car->speed_target = iVar5;
		}
	}

	iVar5 = car->field444_0x520;
	if (iVar5 != 3) {
		if (iVar5 == 4) {
			iVar5 = FUN_00047121(car);
			car->car_road_speed = iVar5;
		} else {
			iVar5 = FUN_0007bbfe(car);
			car->car_road_speed = iVar5;
		}
	}

	if ((car->field_174[0] & 0x20000) != 0) {
		car->speed_target = 0;
	}
	car->field_174[0] = (car->field_174[0] & 0xffffbffc) | 0x1e0;
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
	int steer;
	int speed;
	int iVar10;
	int iVar13;
	int centerline;
	int lateral_speed;
	int iVar14;
	int iVar15;
	int ground_height;
	char bVar1;
	int iVar18;
	int iVar19;
	char ai_type;

	ai_type = (car->field_174[0] & 4) != 0;

	iVar3 = abs(car->car_road_speed >> 0x10);
	iVar44 = iVar3 + 1;
	if (iVar3 > 99)
		iVar44 = 99;

	iVar14 = car->field_174[1]; //car->field_174[4 * iVar44];

	if (iVar44)
		iVar15 = car->field_174[1]; //car->field_174[iVar44];
	else
		iVar15 = 0x640000;

	if (ai_type && (player_car_ptr->speed >= 0x20000 || player_car_ptr->field_4e1 != 2) && player_car_ptr->field_4e1 != 4) {
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

	if (car->field_174[0] & 8) {
		next_state = 6;
		lane_change_speed = 0x2AAA;
	} else if (car->field_174[0] & 4) {
		next_state = 4;
		lane_change_speed = 0x4000;
	} else {
		next_state = 8;
		lane_change_speed = 0x2000;
	}
	accel = next_state - curr_state;

	if (abs(car->steer_angle - car->field_1c) > 0x60000 && car->car_road_speed > 0x70000) {
		if (ai_type) {
			steer = (abs(car->steer_angle - car->field_1c) >> 16) * 0xCCCC;
		} else {
			steer = (abs(car->steer_angle - car->field_1c) >> 16) * 0x1745;
		}
	} else {
		steer = 0;
	}

	bVar1 = FUN_0076FB9(car->road_segment_a) == 0 //
	|| car->center_line_distance < track_data[car->road_segment_a].roadLeftFence * -0x2000 //
	|| car->center_line_distance > track_data[car->road_segment_a].roadRightFence * 0x2000 //
	|| car->field_08c > 0;

	if (car->field_08c > 0) {
		car->field_08c--;
		if (car->field_08c == 0) {
			car->field_08c = -20;
		}
	}
	if (car->field_08c < 0)
		car->field_08c++;

	if ((car->field_174[0] & 0x1000) == 0) {
		if (car->speed_target <= car->car_road_speed) {
			if (iVar15) {
				if (bVar1) {
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
	} else if (car->speed_target >= car->car_road_speed) {
		if (iVar15) {
			if (bVar1) {
				car->car_road_speed += accel * iVar14;
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

	if (steer) {
		if (abs(car->car_road_speed) >= steer) {
			if (car->car_road_speed >= 0) {
				if (car->car_road_speed > 0)
					car->car_road_speed -= steer;
			} else {
				car->car_road_speed += steer;
			}
		} else {
			car->car_road_speed = 0;
			steer = car->car_road_speed;
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

	if (car->field_088 && (car->field_088 != (ai_type ? 100 : 160) //
	|| abs(car->steer_angle - car->field_1c) < 0x20000)) {
		forward_vector.x = car->road_heading.x;
		forward_vector.y = car->road_heading.y;
		forward_vector.z = car->road_heading.z;
	} else {
		forward_vector.x = car->matrix.cx;
		forward_vector.y = car->matrix.cy;
		forward_vector.z = car->matrix.cz;
		if ((car->field_174[0] & 0x1000) != 0) {
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
	|| (centerline && abs(car->steer_angle - car->field_1c) < 0x80000))) {

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

		if ((car->field_174[0] & 0x1000) == 0) {
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
			car->field_1c = math_mul(car->speed_local_lat, steer_speed);
		} else {
			car->field_1c = 0;
		}

		forward_vector.x += right_normal.x;
		forward_vector.y += right_normal.y;
		forward_vector.z += right_normal.z;
	}

	local_position.x = car->position.x - car->road_position.x;
	local_position.y = car->position.y - car->road_position.y;
	local_position.z = car->position.z - car->road_position.z;
	ground_height = math_vec3_dot(&local_position, &car->road_surface_normal);
	if (ground_height > 0x667 && FUN_007D55E(car) && car->road_segment_b > 100 && (car->field_174[0] & 4) && car->car_road_speed > 0x1b0001) {
		if (car->wheels_on_ground)
			car->wheels_on_ground = 0;
	} else {
		if (!car->wheels_on_ground)
			car->wheels_on_ground = 1;
		car->position.x -= (car->road_surface_normal.x >> 8) * (ground_height >> 8);
		car->position.y -= (car->road_surface_normal.y >> 8) * (ground_height >> 8);
		car->position.z -= (car->road_surface_normal.z >> 8) * (ground_height >> 8);
	}
	if (car->wheels_on_ground) {
		if (car->field_088 && car->field_088 != (ai_type ? 100 : 160)) {
			car->speed_y = forward_vector.y;
		} else {
			car->speed_x = forward_vector.x;
			car->speed_y = forward_vector.y;
			car->speed_z = forward_vector.z;
			car->speed_x = -car->speed_x;
		}
	}

	car->ai_state = next_state;
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
	int aux;

	ai_flag_4 = (car->field_174[0] & 4) != 0;
	state = car->ai_state;

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
		if (car->field_174[0] & 0x1000) {
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
		if (car->steer_angle) {
			math_matrix_set_rot_Y(&rot_matrix, car->steer_angle);
			math_matrix_multiply(&car->matrix, &rot_matrix, &car->matrix);
		}
		car->ai_state = 0;
	}

	if (car->steer_angle) {
		car->slide_rear = 0;
		car->slide_front = 0;

		if (!FUN_0076FB9(car->road_segment_a)) {
			car->steer_angle = 0;
			car->field_1c = 0;
			car->angular_speed = 0;
		}

		if (abs(car->steer_angle - car->field_1c) > 0x90000 && abs(car->car_road_speed) > 0x90000) {

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
				car->slide_front = abs(car->steer_angle - car->field_1c) * 0x14;
			} else {
				car->slide_rear = 0;
				car->slide_front = 0;
			}
		}

		if (car->field_174[0] & 0x404)
			max_steer = 0x200000;
		else
			max_steer = 0x160000;

		if (abs(car->steer_angle) > max_steer && (car->field_174[0] & 0x200000) == 0) {
			FUN_0044E11(car->car_data_ptr);
			if (car->field_174[0] & 8) {
				if (FUN_0076FB9(car->road_segment_a))
					tnfs_replay_highlight_record(0x52);
				FUN_00776EA(0);
			}
		}

	} else {
		car->slide_rear = 0;
		car->slide_front = 0;
	}

	if (car->steer_angle != car->field_1c && abs(car->steer_angle) < 0xF0000 && car->wheels_on_ground) {
		if (car->field_088)
			car->field_088--;
	} else {
		car->field_088 = ai_flag_4 ? 100 : 160;
	}
	if (car->road_segment_b < 150) {
		car->angular_speed = 0;
		car->steer_angle = 0;
		car->field_1c = 0;
	}
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
	if (car->steer_angle >= car->field_1c) {
		if (car->steer_angle <= car->field_1c) {
			if (abs(car->steer_angle) < abs(car->field_1c) + 0x20000) {
				car->angular_speed = 0;
				car->steer_angle = car->field_1c;
			}
		} else if (car->angular_speed <= 0) {
			if (abs(car->car_road_speed) > 0x30000) {
				car->angular_speed = 9 * car->steer_angle;
				car->angular_speed = -car->angular_speed;
				if (car->field_1c + 0x30000 > car->steer_angle) {
					car->steer_angle = car->field_1c;
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
			if (car->field_1c - 0x30000 < car->steer_angle) {
				car->steer_angle = car->field_1c;
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

	if (!car->ai_state)
		tnfs_ai_drive_car(car, car->ai_state);

	car->ai_state--;

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
			car->field_1c = 0;
			car->steer_angle = 0;
		}
	}

	if (!car->wheels_on_ground) {
		car->speed_y -= 0x5361 + 0x14d7;
	}
	car->position.x -= (car->speed_x >> 9) + (car->speed_x >> 5);
	car->position.y += (car->speed_y >> 9) + (car->speed_y >> 5);
	car->position.z += (car->speed_z >> 9) + (car->speed_z >> 5);
	car->position.x -= (car->speed_x >> 11) + (car->speed_x >> 7);
	car->position.y += (car->speed_y >> 7) + (car->speed_y >> 11);
	car->position.z += (car->speed_z >> 7) + (car->speed_z >> 11);

	if (car->field_088) {
		if (ai_flag_4)
			aux = 100;
		else
			aux = 160;
		if (car->field_088 != aux) {
			local_position.x = car->position.x - car->road_position.x;
			local_position.y = car->position.y - car->road_position.y;
			local_position.z = car->position.z - car->road_position.z;
			ground_height = math_vec3_dot(&local_position, &car->road_surface_normal);
			car->position.x -= fixmul(car->road_surface_normal.x, ground_height);
			car->position.y -= fixmul(car->road_surface_normal.y, ground_height);
			car->position.z -= fixmul(car->road_surface_normal.z, ground_height);
		}
	}
}
