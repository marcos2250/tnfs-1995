/*
 * Camera control
 */
#include "tnfs_math.h"
#include "tnfs_base.h"

int DAT_000144F0C = 0;
tnfs_camera_specs g_camera_specs[10];

void tnfs_camera_init() {
	selected_camera = 0;
	camera.id = 9;
	camera.transition_timer = 0x1000;
	camera.id_user = 0;
	camera.car_ptr_2 = &g_car_array[0];
	camera.car_ptr_1 = &g_car_array[0];
	camera.car_angle_ptr = &g_car_array[0].angle;
	camera.focal_distance.x = 0x90000;
	camera.focal_distance.y = 0x90000;
	camera.focal_distance.z = -0x260000;

	//0 - CMODE_INCAR
	g_camera_specs[0].focal_distance.x = -26214;
	g_camera_specs[0].focal_distance.y = 0x12000; //0x1c0cc;
	g_camera_specs[0].focal_distance.z = 0;
	g_camera_specs[0].back_distance = 0x140000;
	g_camera_specs[0].id3 = 3;
	g_camera_specs[0].next_id = 1;
	g_camera_specs[0].smoothness = 4;
	g_camera_specs[0].transition_delay = 28;
	g_camera_specs[0].min_distance = -0x38000;

	//1 - Tail cam
	g_camera_specs[1].focal_distance.x = 0;
	g_camera_specs[1].focal_distance.y = 0x2cccc; //0x1b333;
	g_camera_specs[1].focal_distance.z = -0x63333; //-0x40000;
	g_camera_specs[1].back_distance = 0x140000;
	g_camera_specs[1].id3 = 0;
	g_camera_specs[1].next_id = 2;
	g_camera_specs[1].smoothness = 2;
	g_camera_specs[1].transition_delay = 20;
	g_camera_specs[1].min_distance = -1;

	//2 - Heli cam
	g_camera_specs[2].focal_distance.x = 0;
	g_camera_specs[2].focal_distance.y = 0x2cccc << 1;
	g_camera_specs[2].focal_distance.z = -0x63333 << 1;
	g_camera_specs[2].back_distance = 0x140000;
	g_camera_specs[2].id3 = 3;
	g_camera_specs[2].next_id = 9;
	g_camera_specs[2].smoothness = 3;
	g_camera_specs[2].transition_delay = 20;
	g_camera_specs[2].min_distance = -1;

	//9 - transition to INCAR
	g_camera_specs[9].focal_distance.x = g_camera_specs[0].focal_distance.x;
	g_camera_specs[9].focal_distance.y = g_camera_specs[0].focal_distance.y;
	g_camera_specs[9].focal_distance.z = g_camera_specs[0].focal_distance.z;
	g_camera_specs[9].back_distance = 0x140000;
	g_camera_specs[9].id3 = 3;
	g_camera_specs[9].next_id = 0;
	g_camera_specs[9].smoothness = 4;
	g_camera_specs[9].transition_delay = 0x28;
	g_camera_specs[9].min_distance = -0x38000;
}

int tnfs_camera_angle_limit(tnfs_car_data *car) {
	int iVar1;
	int local_3c;
	int local_38;
	int local_20;

	iVar1 = track_data[car->track_slice & g_slice_mask].heading;
	local_20 = car->angle.y - (iVar1 * 0x400);
	if (local_20 <= 0x800000) {
		if (local_20 < -0x800000) {
			local_20 = local_20 + 0x1000000;
		}
	} else {
		local_20 = local_20 - 0x1000000;
	}
	if ((local_20 > -0x1e0000) && (local_20 < 0x1e0000)) {
		if (car->angle.z < 1) {
			local_3c = -car->angle.z;
		} else {
			local_3c = car->angle.z;
		}
		if (local_3c < 0x240001) {
			if (car->angle.x < 1) {
				local_38 = -car->angle.x;
			} else {
				local_38 = car->angle.x;
			}
			if (local_38 < 0x240001) {
				return 1;
			}
		}
	}
	return 0;
}

void tnfs_camera_43327(tnfs_camera *cam) {
	// stub
}

int tnfs_camera_000436ac(int node, tnfs_vec3 *position) {
	int iVar1;
	int iVar2;
	int next;
	int curr;
	tnfs_vec3 local_88;
	tnfs_vec3 curr_node_pos;
	tnfs_vec3 local_70;
	tnfs_vec3 local_64;
	tnfs_vec3 local_58;

	next = (node + 1) & g_slice_mask;
	curr = node & g_slice_mask;
	curr_node_pos.x = track_data[curr].pos.x;
	curr_node_pos.y = track_data[curr].pos.y;
	curr_node_pos.z = track_data[curr].pos.z;

	local_58.x = curr_node_pos.x + ((int) track_data[curr].side_normal_x >> 0x10) * 2;
	local_58.y = curr_node_pos.y + ((int) track_data[curr].side_normal_y >> 0x10) * 2;
	local_58.z = curr_node_pos.z + ((int) track_data[curr].side_normal_z >> 0x10) * 2;
	local_64.x = track_data[next].pos.x - curr_node_pos.x;
	local_64.y = track_data[next].pos.y - curr_node_pos.y;
	local_64.z = track_data[next].pos.z - curr_node_pos.z;
	local_88.x = local_58.x - curr_node_pos.x;
	local_88.y = local_58.y - curr_node_pos.y;
	local_88.z = local_58.z - curr_node_pos.z;

	math_vec3_cross_product(&local_70, &local_64, &local_88);

	iVar1 = math_mul(local_70.x, (position->x - curr_node_pos.x));
	iVar2 = math_mul(local_70.z, (position->z - curr_node_pos.z));
	iVar1 = math_div(iVar1 + iVar2, local_70.y);
	return curr_node_pos.y - iVar1;
}

void sub_41F9F(int track_slice) {
	//stub
}

int sub_6356B(int a, int b) {
	return 0; //stub
}

int sub_634E2(int a, int b) {
	return 0; //stub
}

void tnfs_camera_update(tnfs_camera *camera) {
	int v1, v2, v3;
	signed long local_10;
	int local_11;
	int local_23;
	int local_24;
	int local_25;
	int local_26;
	int local_27;
	int local_28;
	int local_32;
	int local_33;
	int local_38;
	int local_39;
	int local_41;
	int local_42;
	int local_43;
	int node;
	int local_45;
	int local_46;
	int local_47;
	int local_48;
	tnfs_camera_specs *specs;
	tnfs_vec3 vecA;
	tnfs_vec3 vecB;

	//FIXME
	camera->track_slice = camera->track_slice2 = camera->car_ptr_2->track_slice_lap;

	specs = &g_camera_specs[camera->id];

	if (((camera->id == 5) && !sub_6356B(-10, 12)) //
	|| ((camera->id == 6) && !sub_634E2(-1, 12))) {
		tnfs_camera_43327(camera);
		return;
	}

	if (specs->id3 != 0) {
		local_33 = track_data[g_slice_mask & camera->car_ptr_2->track_slice].slant * 0x400;
		v1 = (track_data[g_slice_mask & camera->car_ptr_2->track_slice].pos.z - camera->car_ptr_2->position.z) >> 8;
		v2 = v1 * (math_sin_2(local_33 >> 8) >> 8);
		v3 = (track_data[g_slice_mask & camera->car_ptr_2->track_slice].pos.x - camera->car_ptr_2->position.x) >> 8;
		local_32 = v2 - v3 * (math_cos_2(local_33 >> 8) >> 8)
				- ((track_data[g_slice_mask & camera->car_ptr_2->track_slice].roadRightMargin << 13)
						- (track_data[g_slice_mask & camera->car_ptr_2->track_slice].roadLeftMargin << 13)) / 2;
		if (track_data[g_slice_mask & camera->car_ptr_2->track_slice].slope)
			local_32 = 0;
		if (specs->id3 == 1) {
			//specs->focal_distance.x = (local_32 >> 1) + local_32;
			//specs->focal_distance.x = -specs->focal_distance.x;
			camera->transition_timer = 2;
		} else if (specs->id3 == 2) {
			//specs->focal_distance.x = -local_32 >> 1;
			camera->transition_timer = 2;
		}
	}

	/* camera transition */
	if (camera->transition_timer != 0) {
		camera->focal_distance.x += (specs->focal_distance.x - camera->focal_distance.x) >> specs->smoothness;
		camera->focal_distance.y += (specs->focal_distance.y - camera->focal_distance.y) >> specs->smoothness;
		camera->focal_distance.z += (specs->focal_distance.z - camera->focal_distance.z) >> specs->smoothness;
		camera->transition_timer--;

		if ((camera->focal_distance.z >= specs->min_distance) //
		|| (camera->transition_timer == 0)) {

			camera->focal_distance.x = specs->focal_distance.x;
			camera->focal_distance.y = specs->focal_distance.y;
			camera->focal_distance.z = specs->focal_distance.z;
			camera->transition_timer = 0;

			if (specs->next_id == 0) {
				tnfs_camera_set(camera, 0);
				return;
			}
		}
	}

	if (camera->id == 0) {
		/* in car */

		/* //FIXME ???
		 camera->target_orientation.x = (camera->car_ptr_2->body_pitch >> 8) * (0xF9AA8 >> 8) + DAT_000144F0C;
		 camera->target_orientation.z = (camera->car_ptr_2->body_roll >> 8)  * (0xF9AA8 >> 10);
		 */
		camera->target_orientation.x = camera->car_angle_ptr->x + camera->car_ptr_2->body_pitch;
		camera->target_orientation.y = camera->car_angle_ptr->y;
		camera->target_orientation.z = camera->car_angle_ptr->z + camera->car_ptr_2->body_roll;

		math_rotate_vector_xz(&specs->focal_distance, &camera->relative_position, camera->car_angle_ptr->y);
		camera->next_position.x = camera->relative_position.x + camera->car_ptr_1->position.x;
		camera->next_position.y = camera->relative_position.y + camera->car_ptr_1->position.y;
		camera->next_position.z = camera->relative_position.z + camera->car_ptr_1->position.z;
		sub_41F9F(camera->car_ptr_2->track_slice);

	} else if (camera->id == 8) {
		/* unknown cam */
		camera->focal_distance.x = specs->focal_distance.x;
		camera->focal_distance.y = specs->focal_distance.y;
		camera->focal_distance.z = specs->focal_distance.z;

		camera->relative_position.x = camera->focal_distance.x;
		camera->relative_position.y = camera->focal_distance.y;
		camera->relative_position.z = camera->focal_distance.z;

		camera->next_position.x = camera->relative_position.x + camera->car_ptr_1->position.x;
		camera->next_position.y = camera->relative_position.y + camera->car_ptr_1->position.y;
		camera->next_position.z = camera->relative_position.z + camera->car_ptr_1->position.z;

		camera->track_slice2 |= tnfs_track_node_find(&camera->next_position, &camera->track_slice);

	} else {
		/* any chase cam */
		local_42 = camera->car_ptr_2->track_center_distance >> 8;
		local_43 = 0x600 - local_42;
		node = camera->car_ptr_2->track_slice;
		local_39 = (track_data[g_slice_mask & (node + 12)].pos.x - track_data[g_slice_mask & (node - 12)].pos.x) >> 8;
		local_46 = (track_data[g_slice_mask & (node + 12)].pos.z - track_data[g_slice_mask & (node - 12)].pos.z) >> 8;
		local_47 = (track_data[g_slice_mask & (node + 13)].pos.x - track_data[g_slice_mask & (node - 11)].pos.x) >> 8;
		local_41 = (track_data[g_slice_mask & (node + 13)].pos.z - track_data[g_slice_mask & (node - 11)].pos.z) >> 8;
		local_39 = ((0x600 - local_42) * local_39 + local_42 * local_47) / 0x600;
		local_10 = (0x600 - local_42) * local_46 + local_42 * local_41;
		local_46 = ((0x600 - local_42) * local_46 + local_42 * local_41) / 0x600;
		local_45 = math_atan2(local_10 / 0x600, local_39);
		math_rotate_vector_xz(&camera->focal_distance, &camera->relative_position, -local_45);

		// raise camera to get a better view on down hills
		local_27 = ((int) track_data[g_slice_mask & camera->car_ptr_2->track_slice].slope) * 0x400;
		if (local_27 > 0x800000)
			local_27 -= 0x1000000;
		local_28 = ((int) track_data[g_slice_mask & (camera->car_ptr_2->track_slice + 1)].slope) * 0x400;
		if (local_28 > 0x800000)
			local_28 -= 0x1000000;
		local_26 = ((local_27 >> 8) * local_43 + local_42 * (local_28 >> 8)) >> 2;
		if (local_26 > 0)
			local_26 /= 2;
		local_11 = camera->focal_distance.z >> 9;
		camera->relative_position.y = camera->focal_distance.y; //FIXME
		camera->relative_position.y += (math_sin_2(local_26 >> 8) >> 8) * local_11;

		// move camera
		camera->next_position.x = camera->relative_position.x + camera->car_ptr_1->position.x;
		camera->next_position.y = camera->relative_position.y + camera->car_ptr_1->position.y;
		camera->next_position.z = camera->relative_position.z + camera->car_ptr_1->position.z;

		// change camera when crashed
		if (g_car_ptr_array[g_player_id]->crash_state == 4) {
			local_25 = tnfs_camera_000436ac(camera->track_slice, &camera->next_position);
			if (camera->next_position.y > 0x60000 + local_25)
				camera->next_position.y = 0x60000 + local_25;
		}

		camera->track_slice2 |= tnfs_track_node_find(&camera->next_position, &camera->track_slice);

		// limit to track boundaries
		vecA.x = camera->next_position.x - track_data[g_slice_mask & camera->track_slice].pos.x;
		vecA.z = camera->next_position.z - track_data[g_slice_mask & camera->track_slice].pos.z;
		local_24 = (int) (track_data[g_slice_mask & camera->track_slice].heading) * -0x400;
		vecB.x = ((math_sin_2(local_24 >> 8) * (vecA.z >> 8)) + (vecA.x >> 8) * math_cos_2(local_24 >> 8)) >> 8;
		local_23 = 0;
		if (vecB.x > track_data[g_slice_mask & camera->track_slice].roadRightFence * 0x2000) {
			vecB.x = track_data[g_slice_mask & camera->track_slice].roadRightFence * 0x2000;
			local_23 = 1;
		} else if (vecB.x < track_data[g_slice_mask & camera->track_slice].roadLeftFence * -0x2000) {
			vecB.x = track_data[g_slice_mask & camera->track_slice].roadLeftFence * -0x2000;
			local_23 = 1;
		}
		if (local_23) {
			vecB.z = ((math_cos_2(local_24 >> 8) * (vecA.z >> 8)) - (vecA.x >> 8) * math_sin_2(local_24 >> 8)) >> 8;
			math_rotate_vector_xz(&vecB, &vecA, (int) track_data[g_slice_mask & camera->track_slice].heading * 0x400);
			camera->next_position.x = track_data[g_slice_mask & camera->track_slice].pos.x + vecA.x;
			camera->next_position.z = track_data[g_slice_mask & camera->track_slice].pos.z + vecA.z;
		}

		// camera rotate
		local_48 = specs->back_distance;
		vecA.x = local_39;
		vecA.y = 0;
		vecA.z = local_46;
		math_vec3_normalize(&vecA);
		vecB.x = (local_48 * (vecA.x >> 8) >> 8) + camera->car_ptr_2->position.x;
		vecB.y = (local_48 * (vecA.y >> 8) >> 8) + camera->car_ptr_2->position.y;
		vecB.z = (local_48 * (vecA.z >> 8) >> 8) + camera->car_ptr_2->position.z;
		local_39 = vecB.x - camera->next_position.x;
		local_38 = vecB.y - camera->next_position.y;
		local_46 = vecB.z - camera->next_position.z;

		local_45 = math_atan2(local_46, local_39);
		camera->target_orientation.y = local_45; // - camera->car_angle_ptr->y;

		/*
		 // camera tilt ???
		 v46 = math_vec3_distance_XZ(&vecB, &camera->next_position);
		 v45 = -math_atan2(local_46, local_38);
		 camera->target_orientation.x = local_45 - camera->car_angle_ptr->x;
		 camera->target_orientation.z = -camera->car_angle_ptr->z;
		 */
		camera->target_orientation.x = 0;
		camera->target_orientation.z = 0;
	}

	/*
	 if (&camera->car_angle_ptr) {
	 camera->next_orientation.x = (camera->target_orientation.x + camera->car_angle_ptr->x) & 0xFFFC00;
	 camera->next_orientation.y = (camera->target_orientation.y + camera->car_angle_ptr->y) & 0xFFFC00;
	 camera->next_orientation.z = (camera->target_orientation.z + camera->car_angle_ptr->z) & 0xFFFC00;
	 }
	 */
	camera->next_orientation.x = camera->target_orientation.x & 0xFFFC00;
	camera->next_orientation.y = camera->target_orientation.y & 0xFFFC00;
	camera->next_orientation.z = camera->target_orientation.z & 0xFFFC00;

	camera->position.x = camera->next_position.x;
	camera->position.y = camera->next_position.y;
	camera->position.z = camera->next_position.z;
	camera->orientation.x = camera->next_orientation.x;
	camera->orientation.y = camera->next_orientation.y;
	camera->orientation.z = camera->next_orientation.z;

	//if (camera->id != 0)
	//	camera->orientation.z += 0xFD8F8;
}

void tnfs_camera_auto_change(tnfs_car_data *car) {
	if (g_cam_change_delay != 0) {
		g_cam_change_delay--;
	}
	if (((camera.id != camera.id_user) || (camera.id == 0)) || (camera.id == 9)) {
		if (!tnfs_camera_angle_limit(car)) {
			g_cam_change_delay = 0x3c;
			if ((camera.id == 0) || (camera.id == 9)) {
				tnfs_camera_set(&camera, 2);
			}
		} else if ((camera.id != camera.id_user) && (g_cam_change_delay == 0)) {
			if (camera.id_user == 0) {
				tnfs_camera_set(&camera, 9);
			} else {
				tnfs_camera_set(&camera, camera.id_user);
			}
		}
	}
}

void tnfs_camera_set(tnfs_camera *camera, int id) {

	if ((id == 0) && (!tnfs_camera_angle_limit(camera->car_ptr_2))) {
		printf("Warning: Couldn\'t set cam to CMODE_INCAR because orientation out of range.\n");
		camera->id = 2;
		return;
	}

	g_cam_change_delay = 0x3c;
	camera->id = id;

	// unknown fields
	//camera->field_8 = 0;
	//camera->field_68 = tnfs_camera_specs[id].field_28;
	//camera->field_88 = &g_camera_specs_000f9878;
	camera->track_slice2 = 1;
	camera->transition_timer = g_camera_specs[id].transition_delay;

	if (camera->transition_timer == 0) {
		camera->focal_distance.x = g_camera_specs[id].focal_distance.x;
		camera->focal_distance.y = g_camera_specs[id].focal_distance.y;
		camera->focal_distance.z = g_camera_specs[id].focal_distance.z;
	}

	// ...

	camera->target_orientation.x = 0;
	camera->target_orientation.y = 0;
	camera->target_orientation.z = 0;
	tnfs_camera_update(camera);
	//FUN_000769e9();
	//tnfs_camera_00042f69((camera->specs->direction).z, g_gfx_detail_level);
}
