/*
 * AI driver
 */
#include "tnfs_math.h"
#include "tnfs_base.h"
#include "tnfs_collision_3d.h"
#include "tnfs_files.h"

int g_ai_frame_counter = 4; // 001449C8 800EC504
int g_is_playing = 1; //FD8E0

tnfs_car_specs g_ai_car_specs;

int g_power_curve[] = { 0x5062, 0x4f1a, 0x4f1a, 0x55c2, 0x2937, 0x2c49, 0x28f5, 0x228f, 0x1df3, 0x19db, 0x7a14, 0x12f1, 0x11eb, //
		0xffd, 0xdf4, 0xc8b, 0xb43, 0x978, 0x7ae, 0x69e, 0x5e3, 0x560, 0x45a, 0x418, 0 };

int DAT_00164FDC[20] = { 0, 0x100, 0x200, 0x300, 0x400, 0x500, 0x600, 0x700, 0x800, 0x900, 0xa00, 0xb00, 0xc00, 0xd00, 0xe00, 0xf00, 0x1000, 0x1100, 0x1200 };

int g_opponent_skill = 1; //=0,1,2,3 // 0016709F

int g_lead_player_id = 0; //00165144
tnfs_car_data *g_lead_player = 0; //DAT_00165334

int DAT_000fdbe8[4] = { 0x20, 0x40, 0x60, 0x80 };
int g_cop_park_position = 0; //000fdbf8
int g_tri_lap_chunks = 0x208;
int DAT_0016532c = 0;
int DAT_00165324 = 0;
int DAT_00165328 = 0;
int DAT_00165330 = 0;
int g_random_direction[2] = { 0, 0 }; //DAT_00164fd8-00164FDC
tnfs_random_struct g_random_struct[2]; //DAT_0016511E-00165134;

int g_ai_opp_speed_factors[8] = { 0xf5c2, 0x10000, 0xfae1, 0xfae1, 0x10000, 0xfae1, 0xf333, 0xf333 };

void tnfs_ai_load_car(tnfs_car_data *car, int newCarModelId);

void tnfs_ai_init() {
	int i, j;
    tnfs_car_data *car;

	g_lead_player = &g_car_array[0];

	for (i = g_number_of_players; i < g_total_cars_in_scene; i++) {

		// car model
		if (i < g_racer_cars_in_scene) {
			g_car_array[i].car_model_id = i - 1; //featured cars, ids 0-7
		} else if (i < g_racer_cars_in_scene + g_number_of_cops) {
			g_car_array[i].car_model_id = 10; //cop Mustang
		} else {
			g_car_array[i].car_model_id = 11 + i; //traffic cars, id>11
		}

		car = &g_car_array[i];
		g_car_ptr_array[i] = &g_car_array[i];
		g_car_array[i].car_data_ptr = &g_car_array[i];
		g_car_array[i].car_specs_ptr = &g_ai_car_specs;
		g_car_array[i].car_id = i;
		g_car_array[i].car_id2 = i;

		// starting grid
		g_car_array[i].track_slice = 0x10;
		tnfs_initial_position(&g_car_array[i]);

		// ai states
		if (i < g_racer_cars_in_scene) {
			g_car_array[i].ai_state = 0x1e4; //opponents
			g_car_array[i].field_158 = 0;
			g_car_array[i].field_4e9 = 7;
		} else if (i < g_racer_cars_in_scene + g_number_of_cops) {
			g_car_array[i].ai_state = 0x1e8; //cop
			g_cop_car_ptr = g_car_ptr_array[i];
			g_car_array[i].field_158 = 1;
			g_car_array[i].field_4e9 = 0;
			g_car_array[i].collision_data.field_084 = 1;
		} else {
			g_car_array[i].ai_state = 0x1e0; //traffic
			g_car_array[i].field_158 = 1;
			g_car_array[i].field_4e9 = 0;
			g_car_array[i].collision_data.field_084 = 1;
		}

		g_car_array[i].track_slice_lap = g_car_array[i].track_slice;

		// load car basic specs
		tnfs_ai_load_car(car, car->car_model_id);
	}

	// init random generator
	g_random_struct[0].id = 1;
	g_random_struct[1].id = 0;

	for (i = 0; i < 2; i++) {
		j = g_car_array[i].track_slice_lap;
		g_random_struct[i].c = j;
		g_random_struct[i].d = j + 0x2b;
		g_random_struct[i].e = j - 0xc;
		g_random_struct[i].a = g_random_struct[i].d + 11;
		g_random_struct[i].b = g_random_struct[i].e - 8;
	}
}

void tnfs_ai_load_car(tnfs_car_data *car, int newCarModelId) {
	int i;

	car->car_model_id = newCarModelId;

	// load PDN specs file
	if (!read_pdn_file("carspecs.pdn", car)) {

		// if file not found
		car->collision_data.size.x = 0x1020c; //PDN 0
		car->collision_data.size.y = 0x94fd;
		car->collision_data.size.z = 0x246e9;
		car->collision_data.moment_of_inertia = 0x1b333; //PDN 0xC
		car->collision_data.mass = 0x10000; //PDN 0x10
		car->field_168 = 0x120000; //PDN 0x14
		car->field_170 = 0x10000; //PDN 0x18

		for (i = 0; i < 100; i++) { //PDN 0x1c
			car->power_curve[i] = g_power_curve[i >> 2];
		}

		car->top_speed_per_gear[0] = 0; //PDN 0x1AC
		car->top_speed_per_gear[1] = 0x1a382c;
		car->top_speed_per_gear[2] = 0x278d24;
		car->top_speed_per_gear[3] = 0x35c5a4;
		car->top_speed_per_gear[4] = 0x446fe8;
		car->top_speed_per_gear[5] = 0x59c4a8;

		car->pdn_max_rpm = 0x1c840000; //PDN 0x1c4
		car->pdn_number_of_gears = 5; //PDN 0x1c8
	}

	// derived specs values
	car->field_16c = math_div(0x10000, car->field_168);
	car->collision_data.angular_acc_factor = math_inverse_value(car->collision_data.moment_of_inertia);
	car->collision_data.linear_acc_factor = math_inverse_value(car->collision_data.mass);
	car->collision_height_offset = car->collision_data.size.y;
	car->collision_data.edge_length = math_vec3_length(&car->collision_data.size);

	// gear ratios
	for (i = 0; i < 6; i++) {
		if (car->top_speed_per_gear[i] != 0) {
			car->ai_gear_ratios[i] = 0x100000000 / car->top_speed_per_gear[i];
		}
	}

	tnfs_reset_car(car);
}

int FUN_00077c58(tnfs_car_data *car) {
	int iVar1;
	iVar1 = g_car_array[g_random_struct[car->field_158].id].car_road_speed;
	if (iVar1 < 90439) {
		return 1271398;
	}
	if (iVar1 < 1821900) {
		return 910950;
	}
	if (iVar1 < 2732851) {
		return 728104;
	}
	if (iVar1 < 3637248) {
		return 452198;
	}
	if (iVar1 < 4548198) {
		return 272629;
	}
	return 181534;
}

int tnfs_ai_traffic_speed(tnfs_car_data *car, int speed) {
	int result;
	if (speed <= 0) {
		result = math_mul(speed, car->collision_data.traffic_speed_factor);
	} else {
		result = math_mul(speed, car->collision_data.traffic_speed_factor);
		if (result < speed - 0xd6666) {
			result = speed - 0xd6666;
		}
		if (result < 0x8e666) {
			result = 0x8e666;
		}
	}
	return result;
}

void tnfs_ai_police_reset_state(int flag) {
	if (g_number_of_cops == 1) {
		if (flag != 0) {
			g_cop_car_ptr->ai_state &= 0xfff7ffff; //disable 0x80000
		}
		g_cop_car_ptr->ai_state &= 0xff86dbff; //disable 0x2400
		g_police_on_chase = 0;
	}
}


void tnfs_ai_random_generator() {
	int i;
	int seed_value;
	int iVar2, iVar3, iVar4, iVar6, iVar8;

	if (g_racer_cars_in_scene > 2) {
		return;
	}

	for (i = 0; i < 2; i++) {
		seed_value = g_car_ptr_array[g_random_struct[i].id]->track_slice_lap;
		g_random_struct[i].c = seed_value;
		g_random_struct[i].d = seed_value + 0x2b;
		g_random_struct[i].e = seed_value - 0xc;
	}

    iVar6 = 0;
	do {
		iVar8 = 0;
		iVar3 = iVar6;
		iVar2 = iVar3 + 1;
		while ((iVar2 < 2) && (g_random_struct[iVar3].e < g_random_struct[iVar2].d)) {
			iVar4 = g_random_struct[iVar3].e + (iVar8 >> 1);
			g_random_struct[iVar3].e = iVar4;
			iVar8 = iVar8 + (g_random_struct[iVar2].d - (iVar4 * 0x10000 >> 0x10));
			g_random_struct[iVar2].e = g_random_struct[iVar2].e - (iVar8 >> 1);
			iVar3 = iVar2;
			iVar2++;
		}
		g_random_struct[iVar6].d = g_random_struct[iVar6].d + (iVar8 >> 1);
		iVar8 = 0;
		iVar6 = iVar3 + 1;
	} while (iVar3 + 2 < 2);

	for (i = 0; i < 2; i++) {
		g_random_struct[i].a = g_random_struct[i].d + 11;
		g_random_struct[i].b = g_random_struct[i].e - 8;
	}

	for (i = 0; i < 2; i++) {
		if (g_random_struct[i].c < 0) {
			g_random_struct[i].c = 0;
		}
		if (g_random_struct[i].d < 0) {
			g_random_struct[i].d = 0;
		}
		if (g_random_struct[i].e < 0) {
			g_random_struct[i].e = 0;
		}
		if (g_random_struct[i].a < 0) {
			g_random_struct[i].a = 0;
		}
		if (g_random_struct[i].b < 0) {
			g_random_struct[i].b = 0;
		}
		g_random_struct[i].d = g_random_struct[i].d & g_slice_mask;
		g_random_struct[i].e = g_random_struct[i].e & g_slice_mask;
		g_random_struct[i].a = g_random_struct[i].a & g_slice_mask;
		g_random_struct[i].b = g_random_struct[i].b & g_slice_mask;
		g_random_struct[i].c = g_random_struct[i].c & g_slice_mask;
	}
}

char tnfs_ai_random_boolean_a(int param_1, int param_2) {
	int local_34;
	int local_1c;

	local_34 = g_random_struct[param_2].b;
	local_1c = g_random_struct[param_2].a;
	local_1c -= local_34;

	if (local_1c < 0) {
		local_1c += g_tri_num_chunks << 2;
	}
	if (local_34 < param_1) {
		local_34 = param_1 - local_34;
	} else {
		local_34 = (g_tri_num_chunks << 2) + param_1 - local_34;
	}
	return local_34 < local_1c;
}

int tnfs_util_random_value_a(int param_1) {
	int i;
	param_1 = param_1 & g_slice_mask;
	for (i = 0; i < 2; i++) {
		if (tnfs_ai_random_boolean_a(param_1, i)) {
			return i + 1;
		}
	}
	return 0;
}

char tnfs_ai_random_boolean_b(int param_1, int param_2) {
	int local_34;
	int local_1c;

	local_34 = g_random_struct[param_2].e;
	local_1c = g_random_struct[param_2].d;
	local_1c -= local_34;

	if (local_1c < 0) {
		local_1c += g_tri_num_chunks << 2;
	}
	if (local_34 < param_1) {
		local_34 = param_1 - local_34;
	} else {
		local_34 = (g_tri_num_chunks << 2) + param_1 - local_34;
	}
	return local_34 < local_1c;
}

int tnfs_util_random_value_b(int param_1) {
	int i;
	param_1 = param_1 & g_slice_mask;
	if (g_racer_cars_in_scene < 3) {
		for (i = 0; i < 2; i++) {
			if (tnfs_ai_random_boolean_b(param_1, i)) {
				return i + 1;
			}
		}
		return 0;
	}
	return 1;
}

void tnfs_ai_respawn_location(tnfs_car_data *car, int *direction, int *node) {
	int piVar1;
	char bVar3;
	int local_28;

	piVar1 = car->field_158;
	bVar3 = track_data[car->track_slice & g_slice_mask].num_lanes >> 4 == 0;

	if (((car->ai_state & 0x2400) != 0) //
	|| (((car->ai_state & 8) != 0 && (bVar3)))) {
		*direction = 1;
		if ((car->ai_state & 8) == 0) {
			local_28 = 12;
		} else {
			local_28 = 18;
		}
		*node = player_car_ptr->track_slice - local_28;
		if (*node < 0) {
			*node = 0;
		}
	} else {
		if ((car->ai_state & 8) == 0) {
			g_lcg_random_nbr = g_lcg_random_mod * g_lcg_random_seed;
			g_lcg_random_mod = g_lcg_random_nbr & 0xffff;
			*direction = 0x7fff < (g_lcg_random_nbr & 0xffff00) >> 8;
			if (bVar3) {
				*direction = 1;
			}
			if (((car->ai_state & 8) == 0) && ((car->ai_state & 4) == 0)) {
				if ((car->ai_state & 0x1000) == 0) {
					g_random_direction[1]--;
					if (g_random_direction[1] < 0) {
						g_random_direction[1] = 0;
					}
				} else {
					g_random_direction[0]--;
					if (g_random_direction[0] < 0) {
						g_random_direction[0] = 0;
					}
				}
				if ((g_number_of_traffic_cars - 1 <= g_random_direction[*direction]) && (!bVar3)) {
					*direction = (*direction == 0);
				}
				g_random_direction[*direction] = g_random_direction[*direction] + 1;
			}
		} else {
			*direction = 0;
		}
		if ((g_car_ptr_array[g_random_struct[piVar1].id]->speed < 0x10000) //
				&& (*direction == 1) //
				&& (car->ai_state & 4) == 0) {
			g_lcg_random_nbr = g_lcg_random_mod * g_lcg_random_seed;
			g_lcg_random_mod = g_lcg_random_nbr & 0xffff;
			*node = (g_random_struct[piVar1].e) - ((g_lcg_random_nbr & 0xffff00) >> 8 & 0xf);
			if (*node < 0) {
				*node = 0;
			}
		} else {
			g_lcg_random_nbr = g_lcg_random_mod * g_lcg_random_seed;
			g_lcg_random_mod = g_lcg_random_nbr & 0xffff;
			*node = (g_random_struct[piVar1].d) + (((g_lcg_random_nbr & 0xffff00) >> 8) * 0xb >> 0x10);
			if (*node < 0) {
				*node = 0;
			}
		}
	}
	*node = *node & g_slice_mask;
}

void FUN_0007da53(tnfs_car_data *car) {
	car->ai_state = car->ai_state & 0xfffd7fff;
	if ((((car->ai_state & 0x2000) == 0) //
			&& (((car->ai_state & 4) == 0 || (player_car_ptr->track_slice <= car->track_slice)))) //
			&& (((car->ai_state & 8) == 0 || (track_data[car->track_slice & g_slice_mask].num_lanes >> 4 != 0)))) {
		if ((car->ai_state & 8) != 0) {
			g_police_on_chase = 0;
		}
	} else if (((car->ai_state) & 0x2000) == 0) {
		if ((car->ai_state & 8) != 0) {
			g_police_on_chase = 0;
		}
	} else {
		car->ai_state = (car->ai_state & 0xffffdfff) | 0x400; //disable cop chase
	}
	if ((car->ai_state & 0x408) == 8) {
		// chasing cop turn around
		car->field_4e9 &= 0xfb; //disable flag 4, disable collision, enable cop park respawn
	}
}

void FUN_0007db5a(tnfs_car_data *car, int side) {
	if ((side == 0) && ((car->ai_state & 4) == 0)) {
		car->car_road_speed = FUN_00077c58(car);
		car->car_road_speed = -car->car_road_speed;
		car->speed_target = car->car_road_speed;
		car->ai_state = (car->ai_state & 0xfef) | 0x1000;
	} else {
		car->speed_target = (g_ai_skill_cfg.opp_desired_speed_c >> 1) //
				+ (g_car_array[g_random_struct[car->field_158].id].speed >> 1);
		car->speed_target = tnfs_ai_traffic_speed(car, car->speed_target);
		if ((car->ai_state & 4) == 0) {
			if ((car->ai_state & 0x404) == 0) {
				car->car_road_speed = car->speed_target;
			} else {
				printf("Setting cop speed to zone basis speed %d (car0 is %d)\n", car->speed_target, player_car_ptr->car_road_speed);
				car->car_road_speed = player_car_ptr->car_road_speed;
				car->speed_target = car->car_road_speed;
			}
		} else {
			car->car_road_speed = 0;
		}
		car->ai_state = car->ai_state & 0xefef;
	}
}

void tnfs_ai_respawn_reset(tnfs_car_data *car) {
	tnfs_track_update_vectors(car);

	car->position.x = (car->road_position).x;
	car->position.y = (car->road_position).y;
	car->position.z = (car->road_position).z;
	car->position.x += fixmul(car->target_center_line, car->road_fence_normal.x);
	car->position.y += fixmul(car->target_center_line, car->road_fence_normal.y);
	car->position.z += fixmul(car->target_center_line, car->road_fence_normal.z);

	if (tnfs_track_node_update(car)) {
		tnfs_track_update_vectors(car);
	}

	// ...
	tnfs_collision_data_set(car);
}

void tnfs_ai_respawn_do(tnfs_car_data *car, int node, int side, int centerline, int flag) {

	//FIXME added quirks ***
	//never respawn outside track boundaries
	if (node <= 0 || node >= g_road_node_count) {
		return;
	}
	//never respawn racers
	if (car->ai_state & 4) {
		return;
	}
	// log respawn events
	printf("Respawn car %d, node %d, direction %d, state=%x, f084=%d, f158=%x f4e9=%x\n", //
			car->car_id, node, side, car->ai_state, car->collision_data.field_084, car->field_158, car->field_4e9);
	// ***


	if (node <= 0) {
		node = 0;
	}

	car->track_slice = node;
	car->track_slice_lap = node;

	tnfs_ai_get_speed_factor(car);
	tnfs_ai_get_lane_slack(car);

	car->collision_data.field_08c = 0;
	car->collision_data.field_084 = 0;
	car->collision_data.field_088 = (car->ai_state & 4) ? 160 : 100;
	car->target_angle = 0;
	car->steer_angle = 0;
	car->collision_data.state_timer = 0;
	car->angular_speed = 0;

	//if ((((car->ai_state & 8) != 0) && (g_is_playing != 1)) && (g_restore_camera == 6)) {
	//	g_restore_camera = 4;
	//	FUN_00041ea7(&selected_camera, &g_car_ptr_array[g_lead_player_id], 4);
	//}

	FUN_0007da53(car);
	//FUN_00064e46();
	FUN_0007db5a(car, side);
	car->target_center_line = centerline;
	tnfs_ai_respawn_reset(car);
	tnfs_ai_get_speed_factor(car);

	// for traffic cars
	if (((flag != 0) && ((car->ai_state & 4) == 0)) && ((car->ai_state & 8) == 0)) {
		if ((car->car_id < 0) || (car->car_id >= g_number_of_players)) {
			g_lcg_random_nbr = g_lcg_random_mod * g_lcg_random_seed;
			car->collision_data.field_084 = ((((g_lcg_random_nbr & 0xFFFF00) >> 8) * g_ai_skill_cfg.traffic_density) >> 16) + 1;
			g_lcg_random_mod = g_lcg_random_nbr & 0xffff;
			//car->crash_state = 6; //???
			if ((car->ai_state & 4) == 0) {
				DAT_0016532c = DAT_0016532c + 1;
			}
		}
	}
}

void tnfs_ai_respawn_0007e2ee(tnfs_car_data *car, int flag) {
	int direction;
	int node;

	if (car->ai_state & 8) {
		g_cop_car_ptr->field_158 = player_car_ptr->field_158;
	}

	car->slide_front = 0;
	car->slide_rear = 0;

	if (car == player_car_ptr) {
		player_car_ptr->ai_state = player_car_ptr->ai_state & 0xfffedbff;
	}

	tnfs_ai_respawn_location(car, &direction, &node);

	if ((direction == 0) && ((car->ai_state & 4) == 0)) {
		g_lcg_random_nbr = g_lcg_random_mod * g_lcg_random_seed;
		g_lcg_random_mod = g_lcg_random_nbr & 0xffff;
		/*
		car->target_center_line = ((((DAT_001039d8 & 0xffff00) >> 8) * (track_data[node & DAT_0014dccc].num_lanes >> 4) >> 0x10) + 1)
				* ((&DAT_00164FDC + ((track_data[node & DAT_0014dccc].roadRightMargin << 0xd) >> 0x10) * 2
						+ (track_data[node & DAT_0014dccc].num_lanes & 0xf) * 0x50 + 2) >> 0x10) * -0x100
				+ (((&DAT_00164FDC + ((track_data[node & DAT_0014dccc].roadRightMargin << 0xd) >> 0x10) * 2
						+ (track_data[node & DAT_0014dccc].num_lanes & 0xf) * 0x50 + 2) >> 0x10) * 0x100) / 2;
		 */
		car->target_center_line = (track_data[node].num_lanes >> 4) * (track_data[node].roadLeftMargin >> 2) * -0x100;
	} else {
		g_lcg_random_nbr = g_lcg_random_mod * g_lcg_random_seed;
		g_lcg_random_mod = g_lcg_random_nbr & 0xffff;
		/*
		car->target_center_line = (((track_data[node & DAT_0014dccc].num_lanes & 0xf) * ((DAT_001039d8 & 0xffff00) >> 8) >> 0x10) + 1)
				* ((&DAT_00164FDC + ((track_data[node & DAT_0014dccc].roadRightMargin << 0xd) >> 0x10) * 2
						+ (track_data[node & DAT_0014dccc].num_lanes & 0xf) * 0x50 + 2) >> 0x10) * 0x100
				- (((&DAT_00164FDC + ((track_data[node & DAT_0014dccc].roadRightMargin << 0xd) >> 0x10) * 2
						+ (track_data[node & DAT_0014dccc].num_lanes & 0xf) * 0x50 + 2) >> 0x10) * 0x100) / 2;
		*/
		car->target_center_line = (track_data[node].num_lanes & 0xf) * (track_data[node].roadRightMargin >> 2) * 0x100;
	}
	car->target_center_line = car->target_center_line + car->lane_slack;
	//tnfs_ai_respawn_racer_0007df87(car, &node);
	tnfs_ai_respawn_do(car, node, direction, car->target_center_line, flag);
}

void tnfs_ai_respawn_0007702c(tnfs_car_data *car) {
	int iVar3;
	int i;

	// only head to head
	if (g_racer_cars_in_scene > 2) {
		return;
	}

	if ((car->ai_state & 8) == 0) {
		// traffic
		for (i = car->field_158; i < 2; i++) {
			tnfs_ai_respawn_0007e2ee(car, 0);
			iVar3 = tnfs_util_random_value_b(car->track_slice);
			if (iVar3 == 0)
				break;
			car->field_158++;
		}
		if (car->field_158 > 1) {
			car->field_158 = 0;
			tnfs_ai_respawn_0007e2ee(car, 1);
		}
	} else {
		// cop car
		iVar3 = (car->ai_state >> 20) + 1; //???
		if (iVar3 < 2) {
			car->ai_state = (iVar3 * 0x100000) | (car->ai_state & 0xff8fffff);
			g_cop_car_ptr->field_158 = player_car_ptr->field_158;
			printf("Reinitializing cop\n");
			tnfs_ai_respawn_0007e2ee(car, 0);
		} else {
			tnfs_ai_police_reset_state(1);
		}
	}
}

int tnfs_ai_respawn_0077121(tnfs_car_data *car) {
	int uVar1;

	if (car->track_slice_lap < 2) {
		if ((car->ai_state & 4) == 0) {
			car->field_158 = 0;
		}
		tnfs_ai_respawn_0007e2ee(car, 1);
		uVar1 = 0;
	} else {
		uVar1 = 0;
		if (g_racer_cars_in_scene < 3) {
			if (tnfs_util_random_value_a(car->track_slice) == 0) {
				car->field_158++; //???
				tnfs_ai_respawn_0007702c(car);
				uVar1 = 0;
			} else {
				uVar1 = 1;
			}
		}
	}
	return uVar1;
}

int FUN_007E87B(tnfs_car_data *car, int centerline, int speed) {
	int iVar1;
	int iVar2;

	if ((car->ai_state & 4) == 0) {
		iVar2 = 0x33333;
	} else {
		iVar2 = g_ai_opp_data[car->car_id2].opp_lane_change_speeds;
	}
	if ((car->ai_state & 4) == 0) {
		iVar1 = 0x4000;
	} else {
		iVar1 = g_ai_opp_data[car->car_id2].field_0x69;
	}
	iVar1 = math_mul(iVar1, speed);
	if (iVar2 <= iVar1) {
		iVar1 = iVar2;
	}
	if (centerline < 1) {
		iVar1 = -iVar1;
	}

	return iVar1;
}

char FUN_0007d4b1(tnfs_car_data *car1, tnfs_car_data *car2) {
	int uVar1;
	int local_34;
	int local_1c;

	uVar1 = (car1->track_slice - 0x14) & g_slice_mask;
	local_1c = ((car1->track_slice + 0x36) & g_slice_mask) - uVar1;
	if (local_1c < 0) {
		local_1c = local_1c + g_tri_num_chunks * 4;
	}
	if ((int) uVar1 < car2->track_slice) {
		local_34 = car2->track_slice - uVar1;
	} else {
		local_34 = (car2->track_slice - uVar1) + g_tri_num_chunks * 4;
	}
	return local_34 < local_1c;
}

int FUN_007D55E(tnfs_car_data *car) {
	for (int i = 0; i < g_number_of_players; i++) {
		if (g_number_of_players <= i) {
			return 0;
		}
		if (FUN_0007d4b1(g_car_ptr_array[i], car)) {
			break;
		}
	}
	return 1;
}

int FUN_000779b7(int car_id, int param_2) {
	return g_race_positions[car_id] / 2 << 1 <= param_2;
}

int tnfs_ai_racer_speed(tnfs_car_data *car) {
	int top_speed;
	int uVar8;
	int uVar9;
	int local_74;
	int local_70;
	int local_30;
	int local_28;
	int local_24;
	int result_speed;

	uVar8 = g_game_settings & 4;

	if ((car->ai_state & 4) == 0) {
		result_speed = g_track_speed[car->track_slice >> 2].top_speed << 0x10;
		if (player_car_ptr->car_model_id < 8) { // featured cars
			result_speed = math_mul(result_speed, g_ai_opp_speed_factors[player_car_ptr->car_model_id]);
		}
		if (player_car_ptr->is_wrecked) {
			result_speed = 0;
		}
		return result_speed;
	}

	top_speed = g_track_speed[car->track_slice >> 2].top_speed * 0x10000;

	local_30 = car->field_170;
	if (uVar8 == 0) {
		local_30 = math_mul(g_ai_opp_data[car->car_id2].field_0x55, car->field_170);
	}

	DAT_00165328 = track_data[car->track_slice & g_slice_mask].heading * 0x400 - track_data[(car->track_slice + 1) & g_slice_mask].heading * -0x400;
	DAT_00165328 = abs(DAT_00165328);
	if (DAT_00165328 >= 0x800000) {
		DAT_00165328 = 0x1000000 - DAT_00165328;
	}

	uVar9 = math_mul(car->field_168 - DAT_00165328, car->field_16c);
	DAT_00165324 = (uVar9 >> 3) + (uVar9 >> 4) + (uVar9 >> 6);
	if (uVar8 == 0) {
		DAT_00165324 = math_mul(DAT_00165324, g_ai_opp_data[car->car_id2].field_0x59);
	}

	DAT_00165330 = (car->track_slice_lap - g_lead_player->track_slice_lap) / 10 + 10;
	if (DAT_00165330 < 0) {
		DAT_00165330 = 0;
	}
	if (DAT_00165330 > 0x14) {
		DAT_00165330 = 0x14;
	}
	if (uVar8 == 0) {
		local_70 = g_ai_opp_data[car->car_id2].opponent_glue_factors[DAT_00165330];
	} else {
		local_70 = 0x10000;
	}
	if (g_opponent_skill == 1) {
		local_74 = g_ai_skill_cfg.opponent_glue_0[DAT_00165330];
	} else {
		local_74 = g_ai_skill_cfg.opponent_glue_1[DAT_00165330];
	}
	local_28 = math_mul(local_74, local_70);

	if (((g_lead_player->speed < 0x20000) && (g_lead_player->crash_state == 2)) //
			|| ((car->track_slice_lap < g_lead_player->track_slice_lap) //
			&& (g_lead_player->crash_state == 4) //
			&& (0x10000 < local_28))) {
		local_28 = 0x10000;
	}

	local_24 = math_mul(math_mul((DAT_00165324 + 0xcccc), local_28), local_30);
	if (FUN_007D55E(car)) {
		if (local_24 > 0x13333) {
			local_24 = 0x13333;
		}
		if (local_24 < 0xcccc) {
			local_24 = 0xcccc;
		}
	}
	if (car->track_slice_lap < 0x41) {
		local_24 = math_mul(((0x41 - car->track_slice_lap) * 0x3f0 + 0x10000), local_24);
	}
	result_speed = math_mul(top_speed, local_24);
	if (tnfs_racer_crossed_finish_line(car) == 2) {
		if (FUN_000779b7(car->car_id2, tnfs_racer_crossed_finish_line(car))) {
			result_speed = 0;
		}
	}
	return result_speed;
}

void tnfs_ai_update_speed_targets(tnfs_car_data *car) {
	int iVar1;
	int iVar5;
	int iVar6;
	int iVar7;
	int iVar12;
	int iVar13;
	int iVar14;
	int safe_speed;

	if (car->is_crashed == 0) {
		tnfs_track_update_vectors(car);
		math_matrix_from_pitch_yaw_roll(&car->matrix, car->angle.x, car->angle.y, car->angle.z);
	}

	car->center_line_distance = ((car->road_fence_normal).x >> 8) * (((car->position).x - track_data[car->track_slice].pos.x) >> 8) //
		+ ((car->road_fence_normal).y >> 8) * (((car->position).y - track_data[car->track_slice].pos.y) >> 8) //
		+ ((car->road_fence_normal).z >> 8) * (((car->position).z - track_data[car->track_slice].pos.z) >> 8);

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

	/* determine target speed */
	if ((car->car_id < 0) || (car->car_id >= g_number_of_players)) {
		if ((car->ai_state & 0x1000) == 0) {

			// driving forward
			if ((car->ai_state & 4) == 0) {
				// cruise speed
				iVar1 = (g_ai_skill_cfg.traffic_base_speed >> 1) + (g_lead_player->car_road_speed >> 1);
				car->speed_target = iVar1;
				iVar1 = tnfs_ai_traffic_speed(car, iVar1);
				car->speed_target = iVar1;
			} else {
				// opponent car
				car->speed_target = (g_ai_skill_cfg.opp_desired_ahead >> 1) + (g_lead_player->car_road_speed >> 1);
			}

			safe_speed = (int) g_track_speed[car->track_slice >> 2].safe_speed << 0x10;

			if ((car->ai_state & 0x404) == 0) {
				// not opponent (traffic car or police cruise)
				iVar5 = tnfs_ai_traffic_speed(car, safe_speed);
				if (iVar5 < car->speed_target) {
					car->speed_target = iVar5;
				}
			} else {
				// opponents or police chase
				iVar5 = g_ai_skill_cfg.opp_desired_ahead + g_lead_player->car_road_speed;
				if (car->speed_target < iVar5) {
					car->speed_target = iVar5;
				}
				if ((car->ai_state & 8) != 0) {
					// police pull over
					iVar5 = car->track_slice_lap - player_car_ptr->track_slice_lap;
					if ((3 < iVar5 || (((-3 < iVar5 && (g_police_chase_time < 900)) //
						&& (safe_speed < player_car_ptr->car_road_speed))))) {
						car->speed_target = player_car_ptr->speed;
					}
				}
			}

			iVar5 = tnfs_ai_racer_speed(car);
			if ((iVar5 < car->speed_target) || ((car->ai_state & 4) != 0)) {
				car->speed_target = iVar5;
			}

		} else {
			// oncoming traffic
			iVar5 = FUN_00077c58(car);
			iVar5 = tnfs_ai_traffic_speed(car, -iVar5);
			car->speed_target = iVar5;
		}
	}

	if (car->crash_state != 3) {
		if (car->crash_state == 4) {
			car->car_road_speed = tnfs_car_road_speed_2(car);
		} else {
			car->car_road_speed = tnfs_car_road_speed(car);
		}
	}

	if ((car->ai_state & 0x20000) != 0) {
		car->speed_target = 0;
	}

	car->ai_state = (car->ai_state & 0xffffbffc) | 0x1e0;
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
	char is_a_racer;

	is_a_racer = (car->ai_state & 4) != 0;

	iVar3 = abs(car->car_road_speed >> 0x10);
	iVar44 = iVar3 + 1;
	if (iVar3 > 99)
		iVar44 = 99;

	iVar14 = car->power_curve[iVar44];

	if (iVar44)
		iVar15 = car->power_curve[iVar44 - 1];
	else
		iVar15 = 0x640000;

	if (is_a_racer
			&& (player_car_ptr->speed >= 0x20000 || player_car_ptr->crash_state != 2)
			&& player_car_ptr->crash_state != 4) {

		iVar18 = (car->track_slice_lap - player_car_ptr->track_slice_lap) / 10 + 10;
		if (iVar18 < 0)
			iVar18 = 0;
		if (iVar18 > 20)
			iVar18 = 20;

		if (g_opponent_skill == 1)
			iVar19 = g_ai_skill_cfg.opponent_glue_2[iVar18];
		else
			iVar19 = g_ai_skill_cfg.opponent_glue_3[iVar18];

		iVar14 = math_mul(iVar19, iVar14);
		iVar15 = math_mul(iVar19, iVar15);
	} else {
		iVar19 = 0x10000;
	}

	if (is_a_racer)
		iVar4 = 0x5999;
	else
		iVar4 = 0xb333;

	if (!car->wheels_on_ground) {
		iVar14 = 0;
		iVar4 = 0;
	}
	if (iSimTimeClock < 310)
		iVar14 = 0;

	if (car->ai_state & 8) {
		next_state = 6;
		lane_change_speed = 0x2AAA;
	} else if (car->ai_state & 4) {
		next_state = 4;
		lane_change_speed = 0x4000;
	} else {
		next_state = 8;
		lane_change_speed = 0x2000;
	}
	accel = next_state - curr_state;

	// deccelerate a bit on curves
	if (abs(car->steer_angle - car->target_angle) > 0x60000 && car->car_road_speed > 0x70000) {
		if (is_a_racer) {
			curve_deccel = (abs(car->steer_angle - car->target_angle) >> 16) * 0xCCCC;
		} else {
			curve_deccel = (abs(car->steer_angle - car->target_angle) >> 16) * 0x1745;
		}
	} else {
		curve_deccel = 0;
	}

	// floor it or maintain speed
	floor_it = tnfs_util_random_value_b(car->track_slice) == 0 //
		|| car->center_line_distance < track_data[car->track_slice].roadLeftFence * -0x2000 //
		|| car->center_line_distance > track_data[car->track_slice].roadRightFence * 0x2000 //
		|| car->collision_data.field_08c <= 0;

	if (car->collision_data.field_08c > 0) {
		car->collision_data.field_08c--;
		if (car->collision_data.field_08c == 0) {
			car->collision_data.field_08c = -20;
		}
	}
	if (car->collision_data.field_08c < 0)
		car->collision_data.field_08c++;

	if ((car->ai_state & 0x1000) != 0) {
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

					if (is_a_racer) {
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
			if (is_a_racer && car->crash_state == 3 && car->car_road_speed - 0xa0000 > car->speed_target) {
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

	if (car->car_road_speed > g_stats_data[car->car_id2].top_speed_2
		&& (g_track_speed[car->track_slice >> 2].top_speed << 16) >= car->car_road_speed) {
		g_stats_data[car->car_id2].top_speed_2 = car->car_road_speed;
	}

	right_normal.x = car->road_fence_normal.x;
	right_normal.y = car->road_fence_normal.y;
	right_normal.z = car->road_fence_normal.z;

	if (car->collision_data.field_088 != 0 //
			&& (car->collision_data.field_088 != (is_a_racer ? 160 : 100) //
	|| abs(car->steer_angle - car->target_angle) < 0x20000)) {
		forward_vector.x = car->road_heading.x;
		forward_vector.y = car->road_heading.y;
		forward_vector.z = car->road_heading.z;
	} else {
		forward_vector.x = car->matrix.cx;
		forward_vector.y = car->matrix.cy;
		forward_vector.z = car->matrix.cz;
		if ((car->ai_state & 0x1000) != 0) {
			forward_vector.x = -forward_vector.x;
			forward_vector.y = -forward_vector.y;
			forward_vector.z = -forward_vector.z;
		}
	}

	speed = car->car_road_speed;
	forward_vector.x = fixmul(forward_vector.x, speed);
	forward_vector.y = fixmul(forward_vector.y, speed);
	forward_vector.z = fixmul(forward_vector.z, speed);

	/*
	//DAT_00165340 = DAT_00164FDC + ((track_data[car->track_slice].roadRightMargin >> 3) * 2) //
	//				+ (track_data[car->track_slice].num_lanes & 0xf) * 0x50;
	//DAT_00165340 = DAT_00165340 * 0x100;
	DAT_00165340 = (track_data[car->track_slice].roadRightMargin >> 3) * 0x10000;

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
	*/
	//3do version
	lane = (((car->center_line_distance >> 8) * 0x33) >> 0x10) + 4;
	if (lane < 0) lane = 0;
	if (lane > 7) lane = 7;

	centerline = car->target_center_line - car->center_line_distance;

	if (3 - ((track_data[car->track_slice].num_lanes >> 4) & 0xF) == lane //
	|| ((track_data[car->track_slice].num_lanes & 0xF) + 4 == lane //
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

		if ((car->ai_state & 0x1000) == 0) {
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
			&& car->track_slice_lap > 100 // wut??
			&& (car->ai_state & 4)
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
		if (car->collision_data.field_088 && car->collision_data.field_088 != (is_a_racer ? 160 : 100)) {
			car->speed_y = forward_vector.y;
		} else {
			car->speed_x = forward_vector.x;
			car->speed_y = forward_vector.y;
			car->speed_z = forward_vector.z;
			car->speed_x = -car->speed_x;
		}
	}

	car->collision_data.state_timer = next_state;
}

void FUN_0044E11(tnfs_car_data *car) {
	tnfs_collision_data_set(car);
	car->crash_state = 4;
	car->collision_data.state_timer = 0x3c;
	if (g_car_ptr_array[g_player_id] == car) {
		tnfs_collision_on();
	}
}

void tnfs_ai_fence_collision(tnfs_car_data *car) {
	int normalX;
	int normalZ;
	int iVar5;
	int reboundX;
	int reboundZ;
	int local_20;

	normalX = -(car->road_fence_normal).x;
	normalZ = (car->road_fence_normal).z;

	local_20 = math_mul(fixmul(car->speed_x, normalX) + fixmul(normalZ, car->speed_z), 0x14000);

	if (((car->center_line_distance > 0) && (local_20 < 0)) ||
	    ((car->center_line_distance < 0) && (local_20 > 0))) {
		local_20 = 0;
	}

	reboundX = car->speed_x - math_mul(normalX, local_20);
	reboundZ = car->speed_z - math_mul(normalZ, local_20);

	iVar5 = fixmul(normalX, reboundX) + fixmul(reboundZ, normalZ);

	if ((car->center_line_distance < 0) && (iVar5 < 0x10000)) {
		reboundX = math_mul(normalX, 0x10000);
		reboundZ = math_mul(normalZ, 0x10000);
	} else if ((car->center_line_distance > 0) && (iVar5 > -0x10000)) {
		reboundX = math_mul(normalX, -0x10000);
		reboundZ = math_mul(normalZ, -0x10000);
	}

	car->speed_x = reboundX;
	car->speed_z = reboundZ;
}

void tnfs_ai_driving_main(tnfs_car_data *car) {
	signed int ground_height;
	tnfs_vec3 local_position;
	int lane;
	int max_angular_speed;
	int *angular_speed;
	int rotation;
	int max_steer;
	tnfs_vec9 rot_matrix;
	char is_a_racer;
	int speed_x;
	int speed_z;

	is_a_racer = (car->ai_state & 4) != 0;

	if (is_a_racer) {
		speed_x = abs(car->speed_x);
		speed_z = abs(car->speed_z);
		if (speed_x <= speed_z)
			car->speed = (speed_x >> 2) + speed_z;
		else
			car->speed = (speed_z >> 2) + speed_x;
	}

	car->is_crashed = 1;

	if (!is_a_racer || car->track_slice_lap < 2)
		tnfs_ai_respawn_0077121(car);

	// track slice change
	if (tnfs_track_node_update(car)) {
		tnfs_track_update_vectors(car);
		// body rotation matrix
		if (car->ai_state & 0x1000) {
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
		car->collision_data.state_timer = 0;
	}

	if (car->steer_angle) {
		car->slide_rear = 0;
		car->slide_front = 0;

		if (!tnfs_util_random_value_b(car->track_slice)) {
			car->steer_angle = 0;
			car->target_angle = 0;
			car->angular_speed = 0;
		}

		if (abs(car->steer_angle - car->target_angle) > 0x90000 && abs(car->car_road_speed) > 0x90000) {

			/*
			//DAT_00165340 = DAT_00164FDC
			//		+ ((track_data[car->track_slice].num_lanes & 0xF) * 0x50) //
			//		+ (2 * (track_data[car->track_slice].roadRightMargin >> 3));
			DAT_00165340 = (track_data[car->track_slice].roadRightMargin >> 3) * 0x10000;
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
			*/
			//3do
			lane = (((car->center_line_distance >> 8) * 0x33) >> 0x10) + 4;
			if (lane < 0) lane = 0;
			if (lane > 7) lane = 7;

			if (lane != 3 - (track_data[car->track_slice].num_lanes >> 4)) {
				if ((track_data[car->track_slice].num_lanes & 0xf) + 4 <= lane) {
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

		if (car->ai_state & 0x404)
			max_steer = 0x200000;
		else
			max_steer = 0x160000;

		if (abs(car->steer_angle) > max_steer && (car->ai_state & 0x200000) == 0) {
			FUN_0044E11(car);
			if (car->ai_state & 8) {
				if (tnfs_util_random_value_b(car->track_slice))
					tnfs_replay_highlight_record(0x52);
				tnfs_ai_police_reset_state(0);
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
		car->collision_data.field_088 = is_a_racer ? 160 : 100;
	}

	if (car->track_slice_lap < 150) {
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
			if (is_a_racer)
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
		if (is_a_racer)
			max_angular_speed = 0x23333;
		else
			max_angular_speed = 0x13333;
		*angular_speed += max_angular_speed;
	}

	if (car->collision_data.state_timer == 0)
		tnfs_ai_drive_car(car, car->collision_data.state_timer);

	car->collision_data.state_timer--;

	if (!g_is_closed_track && tnfs_util_random_value_b(car->track_slice)) {

		/*
		//DAT_00165340 = DAT_00164FDC
		//		+ ((track_data[car->track_slice].num_lanes & 0xF) * 0x50)
		//		+ (2 * track_data[car->track_slice].roadRightMargin);
		DAT_00165340 = DAT_00164FDC[(track_data[car->track_slice].roadRightMargin >> 3) + ((track_data[car->track_slice].num_lanes & 0xf) >> 4)] * 0x100;
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
		*/
		//3do
		lane = (((car->center_line_distance >> 8) * 0x33) >> 0x10) + 4;
		if (lane < 0) lane = 0;
		if (lane > 7) lane = 7;

		if (3 - (track_data[car->track_slice].num_lanes >> 4) != lane) {

			/*
			//DAT_00165340 = DAT_00164FDC
			//		+ ((track_data[car->track_slice].num_lanes & 0xF) * 0x50)
			//		+ (2 * track_data[car->track_slice].roadRightMargin);
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
			*/

			/*
			// FIXME ???
			if ((track_data[car->track_slice].num_lanes & 0xf) + 4 != lane) {
				if (car->track_slice_lap > 70)
					car->car_road_speed = 255 * (car->car_road_speed >> 8);
			}
			*/
		}

	}

	// fence collision
	if (car->center_line_distance < (int)track_data[car->track_slice & g_slice_mask].roadLeftFence * -0x2000 //
		|| car->center_line_distance > (int)track_data[car->track_slice & g_slice_mask].roadRightFence * 0x2000) {
		if (tnfs_util_random_value_b(car->track_slice)) { //???
			if (car->lane_slack >= -0x40000) {
				tnfs_ai_fence_collision(car);
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
	if (car->collision_data.field_088 != 0 //
			&& car->collision_data.field_088 != (is_a_racer ? 160 : 100)) {
		local_position.x = car->position.x - car->road_position.x;
		local_position.y = car->position.y - car->road_position.y;
		local_position.z = car->position.z - car->road_position.z;
		ground_height = math_vec3_dot(&local_position, &car->road_surface_normal);
		car->position.x -= fixmul(car->road_surface_normal.x, ground_height);
		car->position.y -= fixmul(car->road_surface_normal.y, ground_height);
		car->position.z -= fixmul(car->road_surface_normal.z, ground_height);
	}
}


void tnfs_ai_traffic_swerve(tnfs_car_data *car1, int *lane_grid, int lane, int *resultLane, tnfs_vec3 *speedA, tnfs_vec3 *speedB, tnfs_car_data **others) {
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

	if (car1->ai_state & 0x1000) {
		j = g_total_cars_in_scene;
		local_1c = 0;
		while (local_1c != 3 && j > 0) {
			carId = (j + aiCarID) % g_total_cars_in_scene;
			car = g_car_ptr_array[carId];
			if ((car1 != car) && (car->field_4e9 & 4) && (car->crash_state != 6)) {
				seg_distance = car->track_slice - car1->track_slice;
				if ((seg_distance < 0) && (0 < car->track_slice_lap - car1->track_slice_lap)) {
					seg_distance = -seg_distance;
				}
				if (seg_distance <= 0 && -seg_distance < 27) {
					seg_distance = seg_distance + 2;
					if (((1 << lane) & lane_grid[carId]) //
					&& ((*resultLane & 1 << (lane & 0x1f)) == 0)) {
						speedB->y = 0x1b6db * seg_distance + car->car_road_speed;
						speedA->y = 0x50000 * seg_distance + car->car_road_speed;
						*resultLane |= 1 << lane;
						others[1] = car;
						local_1c++;
					}
					if (lane > 0 //
					&& ((1 << (lane - 1)) & lane_grid[carId]) //
							&& ((*resultLane & 1 << ((lane - 1) & 0x1f)) == 0)) {
						speedB->x = 0x1b6db * seg_distance + car->car_road_speed;
						speedA->x = 0x50000 * seg_distance + car->car_road_speed;
						*resultLane |= 1 << (lane - 1);
						others[0] = car;
						local_1c++;
					}
					if (lane < 7 //
					&& ((1 << (lane + 1)) & lane_grid[carId]) //
							&& ((*resultLane & 1 << ((lane + 1) & 0x1f)) == 0)) {
						speedB->z = 0x1b6db * seg_distance + car->car_road_speed;
						speedA->z = 0x50000 * seg_distance + car->car_road_speed;
						*resultLane |= 1 << (lane + 1);
						others[2] = car;
						local_1c++;
					}
				}
			}
			--j;
		}
		if ((others[1] != 0) //
				&& (g_number_of_cops + g_racer_cars_in_scene <= car1->car_id) //
				&& (g_number_of_cops + g_racer_cars_in_scene + g_number_of_traffic_cars > car1->car_id) //
				&& ((car1->track_slice - others[1]->track_slice) < 3)) {
			speedA->y >>= 1;
			speedB->y >>= 1;
			if (others[1]->speed_target <= car1->speed_target) {
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
			&& car->crash_state != 6) {
				seg_distance = car->track_slice - car1->track_slice;

				if (abs(seg_distance) > 1 //
				|| (fixmul(car1->road_heading.z, (car->position.z - car1->position.z)) //
				+ fixmul(car1->road_heading.x, (car->position.x - car1->position.x)) > 0x4ffff)) {

					if ((seg_distance < 0) && (0 < car->track_slice_lap - car1->track_slice_lap)) {
						seg_distance = -seg_distance;
					}
					if ((car1->ai_state & 4) && (car->ai_state & 0x1000)) {
						local_64 = g_ai_opp_data[car->car_id].opp_oncoming_look_ahead;
					} else {
						local_64 = 27;
					}
					if (seg_distance > 0 && seg_distance < local_64) {
						seg_distance = seg_distance - 2;
						if ((car1->ai_state & 0x404) && seg_distance < 1) {
							seg_distance = 1;
						}
						if (((1 << lane) & lane_grid[carId]) //
						&& ((*resultLane & 1 << (lane & 0x1f)) == 0)) {
							speedB->y = 0x1b6db * seg_distance + car->car_road_speed;
							speedA->y = 0x50000 * seg_distance + car->car_road_speed;
							*resultLane |= 1 << lane;
							others[1] = car;
							local_1c++;
						}
						if (lane > 0 //
						&& ((1 << (lane - 1)) & lane_grid[carId]) //
								&& ((*resultLane & 1 << ((lane - 1) & 0x1f)) == 0)) {
							speedB->x = 0x1b6db * seg_distance + car->car_road_speed;
							speedA->x = 0x50000 * seg_distance + car->car_road_speed;
							*resultLane |= 1 << (lane - 1);
							others[0] = car;
							local_1c++;
						}
						if (lane < 7 //
						&& ((1 << (lane + 1)) & lane_grid[carId]) //
								&& ((*resultLane & 1 << ((lane + 1) & 0x1f)) == 0)) {
							speedB->z = 0x1b6db * seg_distance + car->car_road_speed;
							speedA->z = 0x50000 * seg_distance + car->car_road_speed;
							*resultLane |= 1 << (lane + 1);
							others[2] = car;
							local_1c++;
						}
					}
				}
			}
			++i;

			if ((g_number_of_cops + g_racer_cars_in_scene <= car1->car_id) //
					&& (g_number_of_cops + g_racer_cars_in_scene + g_number_of_traffic_cars > car1->car_id) //
					&& (others[1] != 0) //
					&& (others[1]->track_slice - car1->track_slice < 3)) {
				speedA->y >>= 1;
				speedB->y >>= 1;
				if (others[1]->speed_target >= car1->speed_target) {
					speedA->x >>= 1;
					speedB->x >>= 1;
					speedA->z >>= 1;
					speedB->z >>= 1;
				}
			}
		}

		if ((car1->car_id >= 0 && (car1->car_id < g_racer_cars_in_scene)) //
				&& (others[1] != 0) //
				&& ((others[1]->track_slice - car1->track_slice) < 2) //
				&& (car->car_road_speed > 0x140000)) {
			speed_diff = car->car_road_speed - (car->car_road_speed >> 3);
			if (speed_diff < 0) {
				speed_diff = 0x10000;
			}
			speedA->y = speed_diff;
			speedB->y = speed_diff;
		}
		if ((lane + 1) == ((track_data[g_slice_mask & car1->track_slice].num_lanes & 0xF) + 4)) {
			speedA->z >>= 1;
			speedB->z >>= 1;
		} else if ((lane - 1) == (3 - (track_data[g_slice_mask & car1->track_slice].num_lanes >> 4))) {
			speedA->x >>= 1;
			speedB->x >>= 1;
		}
	}

	if ((car1->car_road_speed > 0xa0000) //
			&& (others[1] != 0) //
			&& (others[1]->car_id >= 0) //
			&& (others[1]->car_id < g_racer_cars_in_scene) //
			&& (speedA->y < speedA->x) //
			&& (speedA->y < speedA->z)) {

		seg_distance = abs(car1->track_slice - others[1]->track_slice);
		speed_diff = car1->car_road_speed - others[1]->car_road_speed;
		if (seg_distance > 2 && seg_distance < 15 && speed_diff > 0) {
			local_a4 = FUN_007E87B(car1, 1, car1->car_road_speed);
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

void tnfs_ai_change_vector_combine(tnfs_car_data *car, tnfs_vec3 *s0, tnfs_vec3 *s1, tnfs_vec3 *result) {
	int target;
	int speed;
	int i;

	if ((car->ai_state & 0x1000) == 0) {
		target = car->speed_target;
		speed = car->car_road_speed;

		for (i = 0; i < 3; i++) {
			if ((&s0->x)[i] < target) {
				(&result->x)[i] += ((target - (&s0->x)[i]) >> 8) * -0xcd;
			}
			if ((&s1->x)[i] < target) {
				(&result->x)[i] += ((target - (&s1->x)[i]) >> 8) * -0x80;
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
		if (car->speed_target < -0x14ccc) {
			car->speed_target = -0x14ccc;
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
		if (car->speed_target > 0x14ccc) {
			car->speed_target = 0x14ccc;
		}
	}
}

int FUN_0004796f(tnfs_car_data *car, int node) {
	return 1; //stub
}

void tnfs_ai_police_busted() {
	if (g_police_speeding_ticket) {
		printf("Cop: Speeding Ticket!");
	} else {
		printf("Cop: Warning Ticket!");
	}
	printf(" Penalties %d - Warnings %d\n", g_stats_data[0].penalty_count, g_stats_data[0].warning_count);
	g_police_speeding_ticket = 0;
	//stub - just reset player car
	//tnfs_reset_car(player_car_ptr);
}

void FUN_000811c2(tnfs_car_data *car, int param_2) {
	tnfs_car_data *ptVar1;
	ptVar1 = car->car_data_ptr;
	car->collision_data.field_084 = param_2;
	ptVar1->crash_state = 1;
	if ((ptVar1 == g_car_ptr_array[g_player_id]) && (ptVar1->crash_state == 4)) {
		tnfs_collision_off();
	}
}

void tnfs_car_stop_0007d5c1(tnfs_car_data *car) {
	car->angular_speed = 0;
	car->steer_angle = 0;
	car->target_angle = 0;
	memcpy(&car->matrix, &car->road_fence_normal, sizeof(car->matrix));
	car->speed_local_lat = 0;
	car->speed_local_vert = 0;
	car->speed_local_lon = 0;
	car->car_road_speed = 0;
	car->wheels_on_ground = 1;
}


void tnfs_ai_respawn_0007d647() {
	tnfs_car_data *car;
	int iVar1;
	int i;

	tnfs_ai_police_reset_state(1);

	for (i = g_number_of_players; i < g_total_cars_in_scene; i++) {
		car = g_car_ptr_array[i];
		iVar1 = tnfs_util_random_value_b(car->track_slice);
		car->ai_state &= 0xfff7dbff;
		g_police_on_chase = 0;
		if ((car->crash_state == 5) //
				|| (car->crash_state == 4) //
				|| ((car->ai_state & 8) != 0) //
				|| (iVar1 == 0)) {
			if ((car->ai_state & 4) == 0) {
				if ((car->car_id < 0) || (g_racer_cars_in_scene <= car->car_id)) {
					if (car->crash_state == 6) {
						DAT_0016532c--;
					}
					car->crash_state = 3;
					tnfs_ai_respawn_0007e2ee(car, 1);
				}
			}
			if (car->crash_state != 3) {
				if (car->crash_state == 4) {
					if ((-car->center_line_distance == track_data[car->track_slice & g_slice_mask].roadLeftFence * 0x2000
							|| (track_data[car->track_slice & g_slice_mask].roadLeftFence * -0x2000) < car->center_line_distance)
							&& (car->center_line_distance <= (track_data[car->track_slice & g_slice_mask].roadRightFence * 0x2000))) {
						tnfs_collision_data_get(car, 3);
						tnfs_car_stop_0007d5c1(car);
					}
				}
				if (((car->crash_state == 5)
						&& (-car->center_line_distance == track_data[car->track_slice & g_slice_mask].roadLeftFence * 0x2000
								|| (track_data[car->track_slice & g_slice_mask].roadLeftFence * -0x2000) < car->center_line_distance))
						&& (car->center_line_distance <= (track_data[car->track_slice & g_slice_mask].roadRightFence * 0x2000))) {
					car->crash_state = 3;
					tnfs_car_stop_0007d5c1(car);
				}
			}
		}
	}
}

void FUN_0007b78f(tnfs_car_data *car, int lane) {
	int iVar12;
	int local_1c;
	int local_18;
	int right_margin;
	int num_Lanes;

	right_margin = track_data[car->track_slice & g_slice_mask].roadRightMargin;
	num_Lanes = track_data[car->track_slice & g_slice_mask].num_lanes;

	if (lane == 0) {
		local_18 = (g_random_direction[(num_Lanes & 0xf) * 0x28 + (right_margin >> 3)] * -0x100 * (num_Lanes >> 4)) //
				- (g_random_direction[(num_Lanes & 0xf) * 0x28 + (right_margin >> 3)] * 0x100 >> 1);

		if (local_18 < ((car->collision_data.size).x + track_data[car->track_slice & g_slice_mask].roadLeftFence * -0x2000)) {
			local_18 = (car->collision_data.size).x + track_data[car->track_slice & g_slice_mask].roadLeftFence * -0x2000;
		}
	} else {
			local_18 = g_random_direction[(num_Lanes & 0xf) * 0x28 + (right_margin >> 3)] * 0x100 * (num_Lanes & 0xf)
					+ (g_random_direction[(num_Lanes & 0xf) * 0x28 + (right_margin >> 3)] * 0x100 >> 1);

		if (local_18 > (track_data[car->track_slice & g_slice_mask].roadRightFence * 0x2000 - (car->collision_data.size).x)) {
			local_18 = track_data[car->track_slice & g_slice_mask].roadRightFence * 0x2000 - (car->collision_data.size).x;
		}
	}

	memcpy(&car->matrix, &car->road_fence_normal, 0x24u);

	car->angular_speed = 0;

	local_1c = ((int)track_data[car->track_slice & g_slice_mask].slope) * 0x400;
	if (local_1c > 0x800000) {
		local_1c -= 0x1000000;
	}
	car->angle.x = -local_1c;

	car->angle.y = ((int) track_data[car->track_slice & g_slice_mask].heading) * 0x400;

	local_1c = ((int) track_data[car->track_slice & g_slice_mask].slant) * 0x400;
	if (local_1c > 0x800000) {
		local_1c -= 0x1000000;
	}
	car->angle.z = -local_1c;

	car->speed_local_lon = 0;
	car->speed_local_lat = 0;
	car->speed_z = car->speed_local_lat;
	car->speed_y = car->speed_z;
	car->speed_x = car->speed_y;

	iVar12 = local_18 -
			 (fixmul((car->position).x - (car->road_position).x, (car->road_fence_normal).x) //
			+ fixmul((car->position).y - (car->road_position).y, (car->road_fence_normal).y) //
			- fixmul((car->position).z - (car->road_position).z, (car->road_fence_normal).z));

	(car->position).x += fixmul((car->road_fence_normal).x, iVar12);
	(car->position).y += fixmul((car->road_fence_normal).y, iVar12);
	(car->position).z += fixmul((car->road_fence_normal).z, iVar12);
}



void tnfs_ai_police_chase(tnfs_car_data *car, int lane, tnfs_vec3 *direction) {
	int node;
	char bVar2;
	int playerNode;
	int iVar4;
	int segDistance;

	playerNode = player_car_ptr->track_slice;
	segDistance = car->track_slice - player_car_ptr->track_slice;
	if (((segDistance < 0) && ((car->ai_state & 0x2400) != 0)) && ((car->ai_state & 0x80000) == 0)) {
		car->ai_state |= 0x80000;
	}

	if ((g_police_chase_time >= g_ai_skill_cfg.cop_warning_time) //
			&& (g_police_chase_time - 4 < g_ai_skill_cfg.cop_warning_time) //
			&& ((car->ai_state & 0x24) != 0) //
			&& (g_is_playing == 1)) {
		g_police_speeding_ticket = 1; //++
	}

	if (((car->ai_state & 0x400) != 0) && (player_car_ptr->is_wrecked == 0)) {

		if ((car->car_road_speed < 0x10000) //
				&& (player_car_ptr->speed < 0x10000) //
				&& (player_car_ptr->ai_state & 0x10000) != 0) {

			if (g_police_chase_time < g_ai_skill_cfg.cop_warning_time) {
				if (g_is_playing == 1) {
					g_stats_data[g_car_ptr_array[0]->car_id].warning_count++;
				}
			} else {
				if (g_is_playing == 1) {
					g_stats_data[g_car_ptr_array[0]->car_id].penalty_count++;
				}
			}
			tnfs_ai_police_busted();
			FUN_000811c2(g_car_ptr_array[0], g_ai_skill_cfg.field_028);

			// reset flags
			car->ai_state &= 0xfff7fbff;
			player_car_ptr->ai_state &= 0xfffeffff; //player released
			g_police_on_chase = 0;
			g_police_chase_time = 0;

			tnfs_ai_respawn_0007d647();
			FUN_0007b78f(g_car_ptr_array[0], (lane >= 0));

			if (g_stats_data[g_car_ptr_array[0]->car_id].penalty_count > 1 && (g_is_playing == 1)) {
				DAT_00153B24 = 4;
				DAT_00153B20 = 4;
				g_stats_data[g_car_ptr_array[0]->car_id].prev_lap_time = 0;
				g_stats_data[g_car_ptr_array[0]->car_id].lap_time = 0;
				//FUN_0008635b();
			}
		} else if ((segDistance > 2) //
				|| (((segDistance >= 0 && (player_car_ptr->speed < 0x30000)) && (car->car_road_speed < 0x30000)))) {
			// pull over player / acc lock
			player_car_ptr->ai_state |= 0x10000;
			printf("pull over!\n");
		}
	}

	if (((car->ai_state & 0x8000) != 0) || ((player_car_ptr->ai_state & 0x10000) != 0)) {
		node = car->track_slice & g_slice_mask;
		iVar4 = FUN_0004796f(car, node);

		if (iVar4 == 0 //
				&& FUN_0004796f(car, node + 1) == 0 //
				&& FUN_0004796f(car, node + 2) == 0) {
			bVar2 = 0;
		} else {
			bVar2 = 1;
		}

		if ((track_data[node].num_lanes >> 4 == 0) && ((lane * 2) < (track_data[node].num_lanes & 0xf))) {
			if (!bVar2) {
				if (lane * 2 + 1 == (track_data[node].num_lanes & 0xf)) {
					direction->x = direction->x + 0x2e0000;
					direction->z = direction->z + 0x2e0000;
				} else {
					direction->x = direction->x + 0x2e0000;
					direction->z = direction->z - 0x2e0000;
				}
			}
		} else if (!bVar2) {
			direction->x = direction->x - 0x2e0000;
			direction->z = direction->z + 0x2e0000;
		}
	}

	if (segDistance < 1) {
		segDistance = -segDistance;
	}
	if ((car->ai_state & 0x2400) != 0) {
		player_car_ptr->ai_state |= 0x400;
	}
	if ((segDistance < 10) && ((car->ai_state & 0x8000) == 0)) {
		player_car_ptr->ai_state |= 0x800;

		if ((segDistance < 5) //
				&& (g_police_on_chase == 0) //
				&& ((g_track_speed[playerNode >> 2].legal_speed << 0x10) < player_car_ptr->car_road_speed)) {

			// engage chase
			g_police_chase_time = 0;
			g_police_on_chase = 1;
			printf("Police chasing!\n");
		}
		if (((g_police_on_chase != 0) //
				&& (car->track_slice < playerNode)) //
				&& ((car->ai_state & 0x20000) != 0)) {
			car->ai_state &= 0xfffdffff; // disable 0x20000 (unpark car)
		}

		if ((g_police_on_chase != 0) && ((car->ai_state & 4) == 0)) {
			if (((car->ai_state & 0x1000) == 0) //
					&& (car->track_slice < playerNode)) {
				car->ai_state |= 0x400;
			} else {
				car->ai_state |= 0x2000;
			}
		}
	}
}

void tnfs_ai_police_parked_respawn() {
	int iVar2;
	int node;
	int local_30;
	int i;
	int uVar2;

	tnfs_car_data *car = g_cop_car_ptr;

	if ((car != 0) && (g_ai_skill_cfg.number_of_traffic_cars != 1) && (g_number_of_players < 2)) {
		iVar2 = g_car_ptr_array[0]->track_slice;
		if (g_track_slice < car->track_slice) {
			tnfs_ai_police_reset_state(1);
		}

		if ((iVar2 > g_cop_park_position) //
			&& ((car->field_4e9 & 4) == 0)) {

			for (i = g_cop_park_position + 1; i <= iVar2; i++) {

				if (((g_tri_num_chunks << 2) <= (i + 0x32)) //
					|| (((track_data[(i + 0x32) & g_slice_mask].side_normal_x) & 1) == 0)) {

					node = i + 0x32;
					g_lcg_random_nbr = g_lcg_random_mod * g_lcg_random_seed;
					g_lcg_random_mod = g_lcg_random_nbr & 0xffff;

					if ((DAT_000fdbe8[((track_data[node & g_slice_mask].side_normal_x & 0x18) >> 3)] << 16) //
							< (((g_lcg_random_nbr & 0xffff00) >> 8 & 0x7f00) << 8)) {
						g_cop_park_position = iVar2;
						return;
					}

					if ((track_data[node & g_slice_mask].side_normal_x & 2) == 0) {
						local_30 = track_data[node & g_slice_mask].roadRightFence * 0x2000 - car->collision_data.size.x;
					} else {
						local_30 = track_data[node & g_slice_mask].roadLeftFence * -0x2000 + car->collision_data.size.x;
					}

					uVar2 = 1;
					if (((track_data[node & g_slice_mask].num_lanes >> 4) & 0xF) //
						|| (track_data[node & g_slice_mask].side_normal_x & 2)) {
						uVar2 = 0;
					}

					car->ai_state = car->ai_state & 0xff87dbff;
					g_police_on_chase = 0;

					tnfs_ai_respawn_do(car, node, uVar2, local_30, 0);
					car->field_4e9 = car->field_4e9 | 4; //re-enable flag 4
					if ((track_data[node & g_slice_mask].side_normal_x & 4U) == 0) {
						car->ai_state = car->ai_state | 0x20000; //parked
					}
					g_cop_park_position = iVar2;
					return;
				}
			}
		}
	}
}

int DAT_000fdcfc = 0;
int DAT_000fdcf8 = 0;
int DAT_000fdd00 = 0;

int FUN_000bbac0() {
	return 1; //stub
}

int FUN_0006588f(int param_1) {
	int uVar1;
	uVar1 = FUN_000bbac0();
	return (int) ((uVar1 & 0xffff) * param_1) >> 0x10;
}

void FUN_00082DA5(int a, int b) {
	if (DAT_000fdcfc == 0) {
		DAT_000fdcf8 = b;
		DAT_000fdd00 = a;
		if ((-1 < a) && (b < g_racer_cars_in_scene)) {
			DAT_000fdd00 = 1;
			DAT_000fdcf8 = 1;
		}
		DAT_000fdcfc = FUN_0006588f(0xf);
		DAT_000fdcfc += 5;
	}
}

void FUN_00077a05(tnfs_car_data *car, tnfs_car_data *others[3], int lane, int laneR, tnfs_vec3 *speed) {
	long lVar1;
	long lVar2;
	int local_4c;
	int local_48;
	int local_44;
	int local_40;
	int local_28;
	int local_18;

	if ((lane == laneR) || (others[1] == 0) || (others[1]->crash_state == 4)) {
		local_18 = speed->y;
	} else {
		local_28 = car->track_slice_lap - others[1]->track_slice_lap;
		if (local_28 < 0) {
			local_28 = local_28 + g_tri_num_chunks * 4;
		}
		local_4c = abs(local_28);

		if (car->car_road_speed == others[1]->car_road_speed || car->car_road_speed - others[1]->car_road_speed < 0) {
			local_48 = -(car->car_road_speed - others[1]->car_road_speed);
		} else {
			local_48 = car->car_road_speed - others[1]->car_road_speed;
		}
		if (car->target_center_line == car->center_line_distance || car->target_center_line - car->center_line_distance < 0) {
			local_44 = -(car->target_center_line - car->center_line_distance);
		} else {
			local_44 = car->target_center_line - car->center_line_distance;
		}
		local_40 = abs(FUN_007E87B(car, 1, car->car_road_speed));

		lVar1 = math_mul(local_44, local_48);
		lVar2 = math_mul(local_4c * 0x60000, local_40);
		lVar1 = math_mul(lVar1, 0x13333);

		if (lVar1 < lVar2) {
			if (laneR < lane) {
				local_18 = speed->z;
			} else {
				local_18 = speed->x;
			}
		} else {
			local_18 = speed->y;
		}
	}

	if ((car->ai_state & 0x1000) == 0) {
		if (local_18 < car->speed_target) {
			car->speed_target = local_18;
		}
		if (car->speed_target < -0x14ccc) {
			car->speed_target = -0x14ccc;
		}
	} else {
		if (car->speed_target < local_18) {
			car->speed_target = local_18;
		}
		if (0x14ccc < car->speed_target) {
			car->speed_target = 0x14ccc;
		}
	}
}

void tnfs_ai_block_opponent_behind(tnfs_car_data *car, int lane, tnfs_vec3 *direction, int nodeA, int nodeB) {
	int i;
	unsigned int uVar2;
	int uVar4;
	tnfs_car_data *car1;

	for (i = 0; i < g_number_of_players; i++) {
		car1 = g_car_ptr_array[i];

		/*
		iVar3 = g_random_direction[(track_data[car1->track_slice & DAT_0014dccc].num_lanes & 0xf * 0x28)];
		iVar3 += track_data[car1->track_slice & DAT_0014dccc].roadRightMargin >> 3;
		DAT_00165340 = iVar3 * 0x100;
		if (car1->center_line_distance < iVar3 * -0x300) {
			uVar4 = 0;
		} else if (car1->center_line_distance < iVar3 * -0x200) {
			uVar4 = 1;
		} else if (car1->center_line_distance < iVar3 * -0x100) {
			uVar4 = 2;
		} else if (car1->center_line_distance < 0) {
			uVar4 = 3;
		} else if (car1->center_line_distance < iVar3 * 0x100) {
			uVar4 = 4;
		} else if (car1->center_line_distance < iVar3 * 0x200) {
			uVar4 = 5;
		} else if (car1->center_line_distance < iVar3 * 0x200) {
			uVar4 = 6;
		} else {
			uVar4 = 7;
		}
		*/
		//FIXME simplified version
		uVar4 = (((car1->center_line_distance >> 8) * 0x33) >> 0x10) + 4;
		if (uVar4 < 0) uVar4 = 0;
		if (uVar4 > 7) uVar4 = 7;

		// block opponent from behind
		if ((car1->track_slice < nodeA) //
			&& (nodeA < car1->track_slice + g_ai_opp_data[car->car_id].opp_block_look_behind) //
			&& (nodeB > 0x86) //
			&& (g_ai_opp_data[car->car_id].opp_block_behind_distance //
					<= -(fixmul(car1->position.x - car->position.x, car->road_heading.x) + fixmul(car1->position.z - car->position.z, car->road_heading.z)))
			) {
			if (uVar4 < lane) {
				direction->x = direction->x + 0x70000;
			} else if (uVar4 == lane) {
				direction->y = direction->y + 0x70000;
			} else {
				direction->z = direction->z + 0x70000;
			}
		}

		if ((car->track_slice < car1->track_slice) && (uVar4 == lane)) {
			uVar4 = g_lead_player->car_road_speed;
			if (uVar4 < 0)
				uVar4 = -uVar4;
			uVar2 = math_mul_floor(0x35555, (car1->track_slice - car->track_slice) * 0x60000);
			if (uVar2 < uVar4) {
				g_lcg_random_nbr = g_lcg_random_mod * g_lcg_random_seed;
				g_lcg_random_mod = g_lcg_random_nbr & 0xffff;
				if (((g_lcg_random_nbr & 0xffff00) >> 8) * 10 >> 0x10 < car->collision_data.field_084) {
					FUN_00082DA5(car->car_id, 0);
					car->collision_data.field_084 = 0;
				}
			}
		} else {
			car->collision_data.field_084 = 0;
		}
		if ((car->track_slice == car1->track_slice - 5) //
				|| ((car->track_slice == player_car_ptr->track_slice - 2) //
						&& (car->car_road_speed > 0xf0000) //
				&& (car1->crash_state == 1))) {
			FUN_00082DA5(car->car_id, 0);
		}
	}
}

int DAT_000fdbb8[8] = { 3, 3, 4, 5, 4, 5, 4, 3 };

void tnfs_ai_opp_engine_and_cornering(tnfs_car_data *car, int lane, tnfs_vec3 *direction) {
	int node;
	int iVar5;
	char bVar9;
	int iVar10;
	int local_44;
	int local_40;
	int local_24;
	int local_18;
	int local_14;

	node = car->track_slice;
	if (node == 0) node = 1;

	iVar10 = tnfs_racer_crossed_finish_line(car);
	if (iVar10 == 2) {
		if ((3 - (track_data[car->track_slice & g_slice_mask].num_lanes >> 4) == lane) //
				|| ((track_data[car->track_slice & g_slice_mask].num_lanes & 0xf) + 4 == lane)) {
			bVar9 = 0;
		} else {
			bVar9 = 1;
		}
		if (bVar9) {
			iVar10 = tnfs_racer_crossed_finish_line(car);
			iVar10 = FUN_000779b7(car->car_id2, iVar10 + 10);
			if (iVar10 != 0) {
				if ((g_race_positions[car->car_id2] & 1) == 0) {
					direction->x = direction->x + 0x640000;
				} else {
					direction->z = direction->z + 0x640000;
				}
			}
		}
	}
	for (local_18 = 0; local_18 < 5; local_18++) {
		iVar10 = car->car_road_speed - car->top_speed_per_gear[local_18];
		if ((iVar10 > 0) //
			&& (iVar10 < 0x8000) //
			&& (car->car_road_speed >> 0x10 != 0) //
			&& (car->collision_data.field_08c == 0)) {
			car->collision_data.field_08c = DAT_000fdbb8[car->car_model_id];
		}
		if (car->car_road_speed < car->top_speed_per_gear[local_18])
			break;
	}

	// AI car engine RPM
	if (car->collision_data.field_08c < 1) {
		local_14 = math_mul(car->car_road_speed, car->ai_gear_ratios[local_18]);
		local_14 = math_mul(local_14, car->pdn_max_rpm);
		local_14 = (local_14 >> 0x11) + (local_14 >> 0x12);
	} else {
		local_14 = 3000;
	}
	local_14 = local_14 - car->rpm_engine;
	car->rpm_engine += fix3(local_14);
	if ((car->pdn_max_rpm >> 0x10) - 200 < car->rpm_engine) {
		car->rpm_engine = car->pdn_max_rpm >> 0x10;
	}

	// only 3DO and PC
	iVar10 = ((int) track_data[(node - 1) & g_slice_mask].slant << 8) | track_data[(node - 1) & g_slice_mask].heading;
	iVar10 >>= 0x10;
	if (iVar10 * 0x400 < 0x800001) {
		local_40 = iVar10 << 10;
	} else {
		local_40 = iVar10 * 0x400 + -0x1000000;
	}
	iVar5 = ((int) track_data[node & g_slice_mask].slant << 8) | track_data[node & g_slice_mask].heading;
	iVar5 >>= 0x10;
	if (iVar5 * 0x400 < 0x800001) {
		local_44 = iVar5 << 10;
	} else {
		local_44 = iVar5 * 0x400 + -0x1000000;
	}
	local_24 = local_44 - local_40;

	if ((lane != 0) //
		&& ((lane + -1 == 3 - (track_data[node & g_slice_mask].num_lanes >> 4) //
		|| (lane + -1 == (track_data[node & g_slice_mask].num_lanes & 0xf) + 4)))) {
		local_24 = 0;
	}
	if (local_24 > 0) {
		direction->x += local_24 * g_ai_opp_data[car->car_id].opp_oncoming_corner_swerve;
	}
	if (local_24 < 0) {
		direction->x += local_24 * -g_ai_opp_data[car->car_id].opp_cut_corners;
	}

	tnfs_ai_block_opponent_behind(car, lane, direction, node, car->track_slice_lap);
}

void tnfs_ai_lane_change() {
	int local_c0;
	tnfs_vec3 car_speed_b;
	tnfs_vec3 car_speed_a;
	unsigned int local_118;
	int node;
	int local_c4;
	tnfs_vec3 change_lane_vector;
	int lane;
	int local_bc;
	int lane_right;
	int distance;
	int lane_left;
	int local_6c;
	int lane_grid[8];
	int i;
	tnfs_car_data *car;
	int iVar3;
	tnfs_car_data *others[3];

	if (!DAT_000FDB94) {
		g_police_chase_time += 4;
		tnfs_ai_random_generator();

		// if a 2 player game, get the winning player car
		local_6c = 0;
		for (i = 0; i < g_number_of_players; i++) {
			if (g_car_ptr_array[i]->track_slice_lap > local_6c) {
				local_6c = g_car_ptr_array[i]->track_slice_lap;
				g_lead_player = g_car_ptr_array[i];
				g_lead_player_id = i;
			}
		}
		if (!(g_game_settings & 4)) {
		      tnfs_ai_police_parked_respawn();
		}

		/* Populate the "lane_grid" array for all cars */

		for (i = 0; i < 8; i++) {
			lane_grid[i] = 0;
		}
		for (i = 0; i < g_total_cars_in_scene; i++) {
			car = g_car_ptr_array[i];

			if (car->crash_state != 6) {
				tnfs_ai_update_speed_targets(car);

				// left side
				distance = car->center_line_distance - car->side_width + 0x8000;
				/*
				 DAT_00165340 = g_random_direction[(track_data[car->track_slice].roadRightMargin >> 3) + ((track_data[car->track_slice].num_lanes & 0xf) >> 4)];
				 DAT_00165340 = (track_data[car->track_slice].roadRightMargin >> 3) * 0x10000;

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
				*/
				//3do version
				lane_left = (((distance >> 8) * 0x33) >> 0x10) + 4;
				if (lane_left < 0) lane_left = 0;
				if (lane_left > 7) lane_left = 7;

				// right side
				distance = car->side_width + car->center_line_distance - 0x8000;
				/*
				 //DAT_00165340 = DAT_00164FDC[(track_data[car->track_slice].roadRightMargin >> 3) + ((track_data[car->track_slice].num_lanes & 0xf) >> 4)];
				 DAT_00165340 = (track_data[car->track_slice].roadRightMargin >> 3) * 0x10000;
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
				*/
				//3do version
				lane_right = (((distance >> 8) * 0x33) >> 0x10) + 4;
				if (lane_right < 0) lane_right = 0;
				if (lane_right > 7) lane_right = 7;

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

		//player_car_ptr->ai_state &= 0xF700; //???
		//player_car_ptr->ai_state &= 0xFB00;

		// do lane changes
		for (i = 0; i < g_total_cars_in_scene; i++) {
			car = g_car_ptr_array[i];

			if (car->crash_state != 6) {
				local_bc = tnfs_util_random_value_b(car->track_slice);
				lane = 0;
				local_c4 = 0;

				if (((car->field_4e9 & 4) != 0) //
						&& (car->car_id < 0 || car->car_id >= g_number_of_players) //
						&& (car->crash_state != 4)) {

					/*
					 // current lane
					 //DAT_00165340 = DAT_000164FDC[(track_data[car->track_slice].roadRightMargin >> 3) + ((track_data[car->track_slice].num_lanes & 0xf) >> 4)] * 0x100;
					 DAT_00165340 = (track_data[car->track_slice].roadRightMargin >> 3) * 0x10000;
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
						*/
					//3do version
					lane = (((car->center_line_distance >> 8) * 0x33) >> 0x10) + 4;
					if (lane < 0) lane = 0;
					if (lane > 7) lane = 7;

					if (car->ai_state & 0x1000) {
						change_lane_vector.x = 0x10000;
						change_lane_vector.y = 0;
						change_lane_vector.z = -0x10000;
					} else {
						change_lane_vector.x = -0x10000;
						change_lane_vector.y = 0;
						change_lane_vector.z = 0;

						if ((g_opponent_skill > 1) && (car->track_slice_lap < 100)) {
							if (((g_race_positions[car->car_id2] * 5 + 100) <= car->track_slice_lap) //
								&& ((3 - ((track_data[car->track_slice].num_lanes >> 4) & 0xF) != lane) //
								&& (track_data[car->track_slice].num_lanes & 0xF) + 4 != lane)) {
								change_lane_vector.y = 0x460000;
							}
						} else if ((g_opponent_skill == 1) && (car->ai_state & 4) && (car->track_slice < 60)) {
							if ((abs(car->track_slice - g_car_ptr_array[0]->track_slice) < 5) //
									&& (3 - ((track_data[car->track_slice].num_lanes >> 4) & 0xF) != lane) //
									&& ((track_data[car->track_slice].num_lanes & 0xF) + 4 != lane)) {
								change_lane_vector.y = 0x460000;
							}
						}
					}

					local_c4 = 0;

					others[2] = 0;
					others[1] = 0;
					others[0] = 0;

					if (car->ai_state & 8) { // is cop car
						tnfs_ai_police_chase(car, lane, &change_lane_vector);
					}

					if (car->ai_state & 4) { // is opponent
						tnfs_ai_opp_engine_and_cornering(car, lane, &change_lane_vector);
					}

					if ((car != others[1]) //
							&& ((track_data[car->track_slice].num_lanes & 0xF) == (track_data[(car->track_slice + 11)].num_lanes & 0xF)) //
							&& lane_grid[lane] == 1 //
							&& (((car->ai_state & 0x1000) && lane < 4) || (((car->ai_state & 0x1000) != 0) && lane >= 4)) //
							&& (3 - ((track_data[car->track_slice].num_lanes >> 4) & 0xF) >= lane) //
							&& (3 - ((track_data[car->track_slice].num_lanes >> 4) & 0xF) <= lane) //
							&& (track_data[car->track_slice].num_lanes & 0xF) + 4 <= lane //
							&& (track_data[car->track_slice].num_lanes & 0xF) + 4 >= lane //
							&& (!(car->ai_state & 0x404) || (car->ai_state & 0x20000)) //
							) {
						/*
						car->target_center_line = ((lane - 4)
								* DAT_00164FDC[(track_data[car->track_slice].roadRightMargin >> 3) + ((track_data[car->track_slice].num_lanes & 0xf) >> 4)] * 0x100)
								+ ((DAT_00164FDC[(track_data[car->track_slice].roadRightMargin >> 3) + ((track_data[car->track_slice].num_lanes & 0xf) >> 4)] * 0x100) >> 1);
						*/
						//3do version
						car->target_center_line = lane * 0x50000 - 0x118000;
					} else {
						if (g_opponent_skill == 1) {
							if (car->ai_state & 0x1000) {
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
						if ((car->ai_state & 0x1000) //
						|| (track_data[car->track_slice].num_lanes & 0xF) <= (track_data[(car->track_slice + 11)].num_lanes & 0xF)) {
							if ((car->ai_state & 0x1000) == 0 //
							&& car->track_slice > 11 //
							&& car->collision_data.traffic_speed_factor < 0x10000 //
							&& (track_data[car->track_slice].num_lanes & 0xF) > (track_data[(car->track_slice - 11)].num_lanes & 0xF)) {
								if (lane + 1 != 3 - ((track_data[car->track_slice].num_lanes >> 4) & 0xF) //
								&& lane + 1 != (track_data[car->track_slice].num_lanes & 0xF) + 4) {
									change_lane_vector.z += 0x280000;
								} else {
									change_lane_vector.y += 0x280000;
								}
							}
						} else if (lane + 1 != 3 - ((track_data[car->track_slice].num_lanes >> 4) & 0xF) //
								&& lane + 1 != (track_data[car->track_slice].num_lanes & 0xF) + 4) {
							if (lane + 2 == 3 - ((track_data[car->track_slice].num_lanes >> 4) & 0xF) //
							|| lane + 2 == (track_data[car->track_slice].num_lanes & 0xF) + 4) {
								change_lane_vector.y += 0x280000;
							}
						} else {
							change_lane_vector.x += 0x280000;
						}
						node = car->track_slice;
						if (car->ai_state & 4)
							local_118 = -0xf0000;
						else
							local_118 = -0x260000;
						if (lane - 1 == 3 - ((track_data[node].num_lanes >> 4) & 0xF) || lane - 1 == (track_data[node].num_lanes & 0xF) + 4) {
							change_lane_vector.x += local_118;
							if ((track_data[car->track_slice].roadLeftFence << 13) - (track_data[car->track_slice].roadLeftMargin << 13) < car->collision_data.size.x)
								change_lane_vector.x -= 0x640000;
						}
						if (3 - ((track_data[node].num_lanes >> 4) & 0xF) == lane || (track_data[node].num_lanes & 0xF) + 4 == lane) {
							change_lane_vector.y += local_118;
							if (lane >= 0) {
								if ((track_data[car->track_slice].roadRightFence << 13) - (track_data[car->track_slice].roadRightMargin << 13) < car->collision_data.size.x)
									change_lane_vector.y -= 0x640000;
							} else if ((track_data[car->track_slice].roadLeftFence << 13) - (track_data[car->track_slice].roadLeftMargin << 13)
									< car->collision_data.size.x) {
								change_lane_vector.y -= 0x640000;
							}
						}
						if (lane + 1 == 3 - ((track_data[node].num_lanes >> 4) & 0xF) || lane + 1 == (track_data[node].num_lanes & 0xF) + 4) {
							change_lane_vector.z += local_118;
							if ((track_data[car->track_slice].roadRightFence << 13) - (track_data[car->track_slice].roadRightMargin << 13) < car->collision_data.size.x)
								change_lane_vector.z -= 0x640000;
						}
						if (lane - 1 < 3 - ((track_data[node].num_lanes >> 4) & 0xF) || lane - 1 > (track_data[node].num_lanes & 0xF) + 4) {
							change_lane_vector.x -= 0x3E80000;
						}
						if (3 - ((track_data[node].num_lanes >> 4) & 0xF) > lane || (track_data[node].num_lanes & 0xF) + 4 < lane) {
							change_lane_vector.y -= 0x3E80000;
						}
						if (lane + 1 < 3 - ((track_data[node].num_lanes >> 4) & 0xF) || lane + 1 > (track_data[node].num_lanes & 0xF) + 4) {
							change_lane_vector.z -= 0x3E80000;
						}

						car_speed_a.x = car->speed_target;
						car_speed_a.y = car->speed_target;
						car_speed_a.z = car->speed_target;
						car_speed_b.x = car_speed_a.x;
						car_speed_b.y = car_speed_a.y;
						car_speed_b.z = car_speed_a.z;

						// functions used to avoid collision to other cars
						tnfs_ai_traffic_swerve(car, &lane_grid[0], lane, &local_c4, &car_speed_a, &car_speed_b, &others[0]);
						tnfs_ai_change_vector_combine(car, &car_speed_a, &car_speed_b, &change_lane_vector);

						if ((car->ai_state & 0x1000) //
								&& (others[1] != 0) //
								&& (car != others[1]) //
								&& (others[1]->car_road_speed > 0xa0000)) {
							change_lane_vector.z -= 0x1e0000;
						}
						if (!(car->ai_state & 0x404) && !(car->ai_state & 8)) {
							if (car->ai_state & 0x1000) {
								if (local_bc) {
									if (car_speed_a.y > -0x30000) {
										g_lcg_random_nbr = g_lcg_random_seed * g_lcg_random_mod;
										g_lcg_random_mod = g_lcg_random_seed * g_lcg_random_mod;
										if ((((g_lcg_random_nbr & 0xFFFF00u) >> 8) & 0xFF) < 35 && car->car_road_speed && !DAT_000F9BB0) {
											FUN_00082DA5(car->car_id, car->field_461);
										}
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

						/*
						car->target_center_line = ((lane - 4)
								* DAT_00164FDC[(track_data[car->track_slice].roadRightMargin >> 3) + ((track_data[car->track_slice].num_lanes & 0xf) >> 4)] * 0x100)
								+ ((DAT_00164FDC[(track_data[car->track_slice].roadRightMargin >> 3) + ((track_data[car->track_slice].num_lanes & 0xf) >> 4)] * 0x100)
										>> 1);
						*/
						//3do version
						car->target_center_line = lane * 0x50000 - 0x118000;

						// limit center line
						iVar3 = track_data[car->track_slice].roadRightFence * 0x2000 - (car->collision_data.size).x;
						if (iVar3 < car->target_center_line) {
							car->target_center_line = iVar3;
						}
						iVar3 = track_data[car->track_slice].roadLeftFence * -0x2000 + (car->collision_data.size).x;
						if (car->target_center_line < iVar3) {
							car->target_center_line = iVar3;
						}

						if (car->ai_state & 0x1000)
							car->target_center_line += car->lane_slack;
						else
							car->target_center_line -= car->lane_slack;

						if ((DAT_00165148 != 0) && (local_c0 != lane)) {
							FUN_00077a05(car, others, lane, local_c0, &car_speed_a);
						}
						if (((car->ai_state & 4) != 0) && (car->track_slice_lap < 65)) {
							car->speed_target = math_mul(car->speed_target, (65 - car->track_slice_lap) * 0x1f80 + 0x10000);
						}
					}
				}
			}
		}
	}
}


void tnfs_player_pull_over(tnfs_car_data *car) {
	int angle;
	int steer;

	if ((car->ai_state & 0x10000) != 0) {
		angle = math_atan2((player_car_ptr->position).z - (car->position).z,
						   (player_car_ptr->position).x - (car->position).x);
		angle = (angle - car->angle.y) & 0xffffff;
		if (0x800000 < angle) {
			angle = angle - 0x1000000;
		}
		steer = (car->steer_angle >> 1) + (angle >> 1);
		car->steer_angle = steer;
		if (0x140000 < steer) {
			car->steer_angle = 0x140000;
		} else if (steer < -0x140000) {
			car->steer_angle = -0x140000;
		}
		car->throttle = 0;
	}
}


void FUN_00047a7d(tnfs_car_data *car) {
	// stub
}

void tnfs_ai_collision_handler() {
	int iVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	tnfs_car_data *car2;
	int j;
	tnfs_car_data *car1;
	int i;

	if (DAT_000f99ec == 0) {
		g_collision_bump_ref = 0x6666;
	} else {
		DAT_000f99ec = DAT_000f99ec - 1;
	}

	// function called once every 4 frames
	g_ai_frame_counter--;
	if (g_ai_frame_counter == 0) {
		tnfs_ai_lane_change();
		g_ai_frame_counter = 4;
	}

	if (g_total_cars_in_scene == 0) {
		return;
	}

	for (i = 0; i < g_total_cars_in_scene; i++) {
		car1 = g_car_ptr_array[i];

		iVar1 = g_tri_num_chunks * 4;

		iVar2 = car1->car_id;
		if (iVar2 >= 0) {
			if (iVar2 < g_number_of_players) {
				tnfs_player_pull_over(car1);
				iVar2 = car1->car_id;
			}
			if ((iVar2 >= 0) && (iVar2 < g_number_of_players)) {
				if ((((track_data[car1->track_slice].num_lanes & 0xf) * 0x50000
						+ (track_data[car1->track_slice].roadRightMargin >> 3) * 0x100
						* (track_data[car1->track_slice].num_lanes & 0xf)) < car1->center_line_distance)
						|| (car1->center_line_distance
								< ((track_data[car1->track_slice].num_lanes & 0xf) * 40
										+ (track_data[car1->track_slice].roadLeftMargin >> 3) * -0x100
										* (track_data[car1->track_slice].num_lanes >> 4)))) {
					FUN_00047a7d(car1);
				}
			}
		}

		if (car1->crash_state != 6) {
			iVar2 = g_total_cars_in_scene;

			//if (iVar1 - car1->track_slice < 4) {
			//  iVar2 = g_total_cars_in_scene + 4;
			//}

			if ((car1->field_4e9 & 6) == 6) {
				for (j = i + 1; j < iVar2; j++) {
					if (j < g_total_cars_in_scene - 1) {
						iVar1 = j;
					} else {
						iVar1 = j % g_total_cars_in_scene;
					}
					car2 = g_car_ptr_array[iVar1];

					if ((car1->track_slice + 2 < car2->track_slice) && (4 < -car1->track_slice))
						break;

					if ((car2->field_4e9 & 5) == 5) {
						iVar1 = abs((car1->position).x - (car2->position).x);
						iVar4 = abs((car1->position).z - (car2->position).z);
						iVar3 = car1->collision_data.edge_length + car2->collision_data.edge_length;
						if ((iVar1 < iVar3) && (iVar4 < iVar3)) {
							tnfs_collision_carcar(car1, car2);
						}
					}
				}
			}
		}
	}
}


void tnfs_ai_hidden_traffic_main(tnfs_car_data *car) {
	int node = 0;
	for (int i = 0; i < g_racer_cars_in_scene; i++) {
		if (node < g_car_ptr_array[i]->track_slice) {
			node = g_car_ptr_array[i]->track_slice;
		}
	}

	if (((car->ai_state & 4) == 0) && (car->field_158 = 0) && (node < 0x78)) {
		g_lcg_random_nbr = g_lcg_random_seed * g_lcg_random_mod;
		g_lcg_random_seed = g_lcg_random_nbr & 0xffff;
		if (((g_lcg_random_nbr & 0xffff00) >> 8 & 1) != 0) {
			car->field_158 = 1;
		}
	}

	car->track_slice = 1;
	car->track_slice_lap = 1;
	car->position.x = 0x80000;
	car->position.y = (car->car_id * 10 + 200) * 0x10000;
	car->position.z = 0x80000;

	car->collision_data.field_084--;
	if (car->collision_data.field_084 != 0) {
		return;
	}

	if (((car->car_id * 0x10 + 0xf) > node) //
			//&& (DAT_0016532c > g_number_of_traffic_cars) //
			&& ((car->ai_state & 4) == 0) // cop or traffic
			&& ((g_game_settings & 4) == 0)) { //unknown setting

		car->crash_state = 3;
		DAT_0016532c--;
		tnfs_ai_respawn_0007702c(car);
		car->field_4e9 = 7; //FIXME QUIRK

	} else {
		car->collision_data.field_084 = 1;
	}
}
