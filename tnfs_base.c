/*
 * globals, structs, and stub TNFS functions
 */
#include "tnfs_math.h"
#include "tnfs_base.h"
#include "tnfs_collision.h"
#include "tnfs_fiziks.h"

tnfs_car_specs car_specs;
tnfs_car_data car_data;
int cheat_mode = 0;
int road_surface_type_array[10];

int roadLeftMargin = 220;
int roadRightMargin = 220;
int roadLeftFence = 200;
int roadRightFence = 200;
char roadConstantA = 0x00;
char roadConstantB = 0x22;
int road_segment_pos_x = 0;
int road_segment_pos_z = 0;
int road_segment_slope = 0;
int road_segment_heading = 0;
int sound_flag = 0;

int selected_camera = 0;
tnfs_vec3 camera_position;

void tnfs_debug_00034309(int a, int b, int x, int y) {
	//TODO
}

void tnfs_debug_000343C2(int a, int b) {
	//TODO
}

void tnfs_sfx_play(int a, int b, int c, int d, int e, int f) {
	printf("sound %i\n", f);
}

void tnfs_physics_car_vector(tnfs_car_data *car_data, int *angle, int *length) {
	*angle = car_data->angle_y;
	*length = car_data->speed;
}

int tnfs_collision_car_size(tnfs_car_data *car_data, int a) {
	return 0;
}

void tnfs_collision_debug(char a) {
	printf("collision %c\n", a);
}

/* 5b5f7 */
void tnfs_cheat_crash_cars() {
	// crashing cars cheat code
	if (cheat_mode == 4) {
		tnfs_collision_rollover_start(&car_data, 0xa0000, 0xa0000, 0xa0000);
	}
}

void tnfs_reset() {
	int i;

	cheat_mode = 0;
	roadLeftMargin = 140;
	roadRightMargin = 140;
	roadLeftFence = 140;
	roadRightFence = 140;
	roadConstantA = 0;
	roadConstantB = 0x22;
	road_segment_pos_x = 0;
	road_segment_pos_z = 0;
	road_segment_slope = 0;
	road_segment_heading = 0;
	sound_flag = 0;

	for (i = 0; i < 10; ++i) {
		road_surface_type_array[i] = 0x100;
	}

	car_specs.front_brake_percentage = 0xc000; //70%
	car_specs.front_drive_percentage = 0; //RWD
	car_specs.max_brake_force_1 = 0x133fff;
	car_specs.max_brake_force_2 = 0x133fff;
	car_specs.body_roll_factor = 0x2666; //0.15
	car_specs.body_pitch_factor = 0x2666; //0.15
	car_specs.max_slip_angle = 0x1fe667; //~45deg
	car_specs.max_speed = 0x47c000; //71m/s
	car_specs.max_tire_lateral_force = 0x150000;
	car_specs.thrust_to_acc_factor = 0x66; //1577kg TNFS scale

	car_data.is_crashed = 0;
	car_data.unknown_const_88 = &car_data;
	car_data.car_specs_ptr = &car_specs;
	car_data.weight_distribution = 0x7e49; //49% front
	car_data.weight_transfer_factor = 0x3028;
	car_data.front_friction_factor = 0xe469a;
	car_data.rear_friction_factor = 0xd10de;
	car_data.wheel_base = 0x7a9; //2.44m TNFS scale
	car_data.wheel_track = 0x3d5; //1.50m TNFS scale
	car_data.front_yaw_factor = 0x107a77;
	car_data.rear_yaw_factor = 0x107a77;
	car_data.gear_speed = 1;
	car_data.gear_speed_selected = 1;
	car_data.gear_RND = 3;
	car_data.throttle = 0;
	car_data.tcs_on = 0;
	car_data.tcs_enabled = 0;
	car_data.brake = 0;
	car_data.abs_on = 0;
	car_data.abs_enabled = 0;
	car_data.wheels_on_ground = 1;
	car_data.surface_type = 0;
	car_data.tire_grip_front = 0xe469a;
	car_data.tire_grip_rear = 0xd10de;
	car_data.slope_force_lat = 0;
	car_data.slope_force_lon = 0;

	car_data.position.x = 0;
	car_data.position.y = 150;
	car_data.position.z = 0;
	car_data.angle_x = 0;
	car_data.angle_y = 0; //uint32 0 to 16777215(0xFFFFFF)
	car_data.angle_z = 0;
	car_data.angular_speed = 0;
	car_data.speed_x = 0;
	car_data.speed_y = 0;
	car_data.speed_z = 0;
	car_data.speed = 0;
	car_data.speed_drivetrain = 0;
	car_data.speed_local_lon = 0;
	car_data.speed_local_lat = 0;
	car_data.steer_angle = 0; //int32 -1769472 to +1769472
	car_data.road_segment_a = 0;
	car_data.road_segment_b = 0;
	car_data.game_status = 1;
	car_data.unknown_flag_475 = 0;

	car_data.road_fence_normal.x = 65532;
	car_data.road_fence_normal.y = -46;
	car_data.road_fence_normal.z = 0;

	//surface normal (up)
	car_data.road_normal.x = 0;
	car_data.road_normal.y = 0x10000;
	car_data.road_normal.z = 0;

	car_data.road_heading.x = 0;
	car_data.road_heading.y = 0;
	car_data.road_heading.z = 0xfcd2;

	//surface position center
	car_data.road_position.x = 0;
	car_data.road_position.y = 0;
	car_data.road_position.z = 0;

	math_matrix_identity(&car_data.matrix);
	math_matrix_identity(&car_data.collision_data.matrix);

	car_data.collision_data.position.x = 0;
	car_data.collision_data.position.y = 0;
	car_data.collision_data.position.z = 0;
	car_data.collision_data.speed.x = 0;
	car_data.collision_data.speed.y = 0;
	car_data.collision_data.speed.z = 0;
	car_data.collision_data.field4_0x48.x = 0;
	car_data.collision_data.field4_0x48.y = 0;
	car_data.collision_data.field4_0x48.z = 0;
	car_data.collision_data.linear_acc_factor = 0xf646;
	car_data.collision_data.angular_acc_factor = 0x7dd4;
	car_data.collision_data.size.x = 0Xf645;  //63045
	car_data.collision_data.size.y = 0x92f1;  //37216
	car_data.collision_data.size.z = 0x242fe; //148222
	car_data.collision_data.crashed_time = 0;
	car_data.collision_data.angular_speed.x = 0;
	car_data.collision_data.angular_speed.y = 0;
	car_data.collision_data.angular_speed.z = 0;
	car_data.collision_data.field6_0x60 = 0;

	car_data.collision_height_offset = 0x92f1;
	car_data.field203_0x174 = 0x1e0;
	car_data.field444_0x520 = 0;
	car_data.field445_0x524 = 0;
	car_data.car_flag_0x480 = 0;

	road_segment_pos_x = 0;
	road_segment_pos_z = 0;
}

void tnfs_change_camera() {
	selected_camera++;
	if (selected_camera > 1)
		selected_camera = 0;
}

void tnfs_change_gear_up() {
	car_data.gear_RND = 3;
	car_data.gear_speed = 0;
	printf("Gear: Drive\n");
}

void tnfs_change_gear_down() {
	car_data.gear_RND = 1;
	car_data.gear_speed = -2;
	printf("Gear: Reverse\n");
}

void tnfs_abs() {
	car_data.abs_enabled = car_data.abs_enabled ? 0 : 1;
	if (car_data.abs_enabled) {
		printf("ABS brakes on\n");
	} else {
		printf("ABS brakes off\n");
	}
}

void tnfs_tcs() {
	car_data.tcs_enabled = car_data.tcs_enabled ? 0 : 1;
	if (car_data.tcs_enabled) {
		printf("Traction control on\n");
	} else {
		printf("Traction control off\n");
	}
}

void tnfs_cheat_mode() {
	cheat_mode = cheat_mode ? 0 : 4;
	if (cheat_mode == 4) {
		printf("Crashing cars cheat enabled - Press handbrake to crash\n");
	} else {
		printf("Crashing cars cheat disabled\n");
	}
}

void tnfs_update() {

	// update camera
	switch (selected_camera) {
		case 1: //chase cam
			camera_position.x = car_data.position.x;
			camera_position.y = 0x60000;
			camera_position.z = car_data.position.z - 0x100000;
			break;
		default: //fixed cam
			camera_position.x = 0;
			camera_position.y = 0x60000;
			camera_position.z = -0x120000;
			break;
	}

	if (car_data.collision_data.crashed_time == 0) {
		// driving mode loop
		tnfs_physics_main(&car_data);
		// update render matrix
		matrix_create_from_pitch_yaw_roll(&car_data.matrix, -car_data.angle_x -car_data.body_pitch, -car_data.angle_y, -car_data.angle_z + car_data.body_roll);
	} else {
		// crash mode loop
		tnfs_collision_main(&car_data);
	}

	// parking lot boundaries
	if (car_data.position.z > 0x600000)
		car_data.position.z = 0x600000;
	if (car_data.position.z < -0x600000)
		car_data.position.z = -0x600000;
}
