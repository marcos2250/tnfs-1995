/*
 * globals, structs, and stub TNFS functions
 */
#include "tnfs_math.h"
#include "tnfs_base.h"
#include "tnfs_fiziks.h"
#include "tnfs_collision_3d.h"

tnfs_car_specs car_specs;
tnfs_car_data car_data;
struct tnfs_track_data track_data[2400];
int road_surface_type_array[10];

// settings/flags
char is_drifting;
int g_game_time = 1000;
int roadLeftMargin = 140;
int roadRightMargin = 140;
int roadLeftFence = 150;
int roadRightFence = 150;
char roadConstantA = 0x20;
char roadConstantB = 0x22;
int road_segment_count = 0;
int sound_flag = 0;
int cheat_mode = 0;
int DAT_8010d1c4 = 0;

int selected_camera = 0;
tnfs_vec3 camera_position;

/* tire grip slide table - default values for front engine RWD cars */
static const unsigned char g_slide_table[64] = {
// front values
		2, 14, 26, 37, 51, 65, 101, 111, 120, 126, 140, 150, 160, 165, 174, 176, 176, 176, 176, 176, 176, 177, 177, 177, 200, 200, 200, 174, 167, 161, 152, 142,
		// rear values
		174, 174, 174, 174, 175, 176, 177, 177, 200, 200, 200, 200, 177, 177, 177, 177, 177, 177, 177, 177, 177, 177, 177, 177, 200, 176, 173, 170, 165, 162, 156, 153 };

/* engine torque table, for 200 rpm values */
static const unsigned int g_torque_table[64] = { 1000, 0x1dc4f, 1200, 0x1e837, 1400, 0x20008, 1600, 0x22fa9, 1800, 0x23b92, 2000, 0x2477a, 2200, 0x2771b, 2400, 0x29ad5, 2600,
		0x2a6bd, 2800, 0x22ba5, 3000, 0x2be8e, 3200, 0x2c0ef, 3400, 0x2ca76, 3600, 0x2d65e, 3800, 0x2ca76, 4000, 0x2b2a5, 4200, 0x2a6bd, 4400, 0x2a6bd, 4600, 0x29ad5, 4800,
		0x29ad5, 5000, 0x29ad5, 5200, 0x28eec, 5400, 0x28eec, 5600, 0x2771b, 5800, 0x25f4b, 6000, 0x25362, 6200, 0x2477a, 6400, 0x23b92, 6600, 0x22fa9, 6800, 0x223c1 };

signed int g_gear_ratios[10] = { //
		-152698, //-2.33 reverse
				0x1999, //neutral
				0x2a8f5, //2.66
				0x1c7ae, //1.78
				0x14ccc, //1.30
				0x10000, //1.00
				0xcccc,  //0.75
				0x8000,  //0.50
				0, 0 };

void tnfs_sfx_play(int a, int b, int c, int d, int e, int f) {
	printf("sound %i\n", f);
}

void tnfs_replay_highlight_000502AB(char a) {
	// not used
}

void tnfs_car_size_vector(tnfs_car_data *car_data, int *angle, int *length) {
	int x;
	int y;
	int z;
	int heading;

	x = car_data->position.x - track_data[car_data->road_segment_a].pos_x;
	y = car_data->position.y - track_data[car_data->road_segment_a].pos_y;
	z = car_data->position.z - track_data[car_data->road_segment_a].pos_z;

	heading = track_data[car_data->road_segment_a].heading * 0x400;

	if (heading < 0) {
		heading = heading + 0x1000000;
	}
	*angle = heading - math_atan2(z, x);
	if (*angle < 0) {
		*angle += 0x1000000;
	}
	if (*angle > 0x1000000) {
		*angle -= 0x1000000;
	}

	if (x < 0) {
		x = -x;
	}
	if (y < 0) {
		y = -y;
	}
	if (z < 0) {
		z = -z;
	}
	if (z < x) {
		x = (z >> 2) + x;
	} else {
		x = (x >> 2) + z;
	}
	if (x < y) {
		*length = (x >> 2) + y;
	} else {
		*length = (y >> 2) + x;
	}
}

void auto_generate_track() {
	int pos_x = 0;
	int pos_y = 0;
	int pos_z = 0;
	int slope = 0;
	int slant = 0;

	road_segment_count = 2400;

	for (int i = 0; i < 2400; i++) {

		if (i & 128) {
			if (i & 64) {
				slope -= 10;
			} else {
				slope += 10;
			}
		} else {
			slope >>= 1;
		}
		if (i & 32) {
			if (i & 16) {
				slant -= 10;
			} else {
				slant += 10;
			}
		} else {
			slant >>= 1;
		}

		if (slope > 0x3FF) slope = 0x3FF;
		if (slope < -0x3FF) slope = -0x3FF;
		if (slant > 0x3FF) slant = 0x3FF;
		if (slant < -0x3FF) slant = -0x3FF;

		track_data[i].roadLeftFence = 0x50;
		track_data[i].roadRightFence = 0x50;
		track_data[i].roadLeftMargin = 0x45;
		track_data[i].roadRightMargin = 0x45;

		track_data[i].slope = slope;
		track_data[i].heading = -slant << 2;
		track_data[i].slant = slant;
		track_data[i].pos_x = pos_x;
		track_data[i].pos_y = pos_y;
		track_data[i].pos_z = pos_z;

		// next segment
		pos_x += fixmul(math_sin_3(track_data[i].heading * 0x400), 0x80000);
		pos_y += fixmul(math_tan_3(track_data[i].slope * 0x400), 0x80000);
		pos_z += fixmul(math_cos_3(track_data[i].heading * 0x400), 0x80000);
	}
}

int readFixed32(char *buffer, int pos) {
	return (int)(buffer[pos + 3] & 0xFF) << 24 //
		| (int)(buffer[pos + 2] & 0xFF) << 16 //
		| (int)(buffer[pos + 1] & 0xFF) << 8 //
		| (int)(buffer[pos] & 0xFF);
}

int readAngle16(char *buffer, int pos) {
	int a = (int)(buffer[pos + 1] & 0xFF) << 8 | (int)(buffer[pos] & 0xFF);
	if (a > 8192) {
		a -= 16384;
	}
	return a;
}

/*
 * Import a TNFS TRI track file
 */
void read_tri_file(char * file) {
	const int chunk_size = 2400 * 36;
	char buffer[chunk_size];
	FILE *ptr;
	int i, offset;

	ptr = fopen(file,"rb");
	if (!ptr) {
		printf("File not found: %s\n", file);
		return;
	}
	fread(buffer, chunk_size, 1, ptr);

	road_segment_count = 0;
	for (i = 0; i < 2400; i++) {
		offset = i * 36 + 2444;

		track_data[i].roadLeftMargin = (int)(buffer[offset] & 0xFF);
		track_data[i].roadRightMargin = (int)(buffer[offset + 1] & 0xFF);
		track_data[i].roadLeftFence = (int)(buffer[offset + 2] & 0xFF);
		track_data[i].roadRightFence = (int)(buffer[offset + 3] & 0xFF);

		track_data[i].pos_x = readFixed32(buffer, offset + 8);
		track_data[i].pos_y = readFixed32(buffer, offset + 12);
		track_data[i].pos_z = readFixed32(buffer, offset + 16);
		track_data[i].slope = -readAngle16(buffer, offset + 20);
		track_data[i].slant = -readAngle16(buffer, offset + 22);
		track_data[i].heading = readAngle16(buffer, offset + 24);

		road_segment_count++;
		if (i > 0 && track_data[i].pos_x == 0 && track_data[i].pos_y == 0 && track_data[i].pos_z == 0) {
			break;
		}
	}
	fclose(ptr);
}

void tnfs_init_track(char * tri_file) {
	int i;
	int heading, s, c, t, dL, dR;

	// try to read a TRI file if given, if not, generate a random track
	if (tri_file) {
		read_tri_file(tri_file);
	}
	if (road_segment_count == 0) {
		auto_generate_track();
	}

	// model track for rendering
	for (i = 0; i < road_segment_count; i++) {
		heading = track_data[i].heading * -0x400;
		s = math_sin_3(heading);
		c = math_cos_3(heading);
		t = math_tan_3(track_data[i].slant * 0x400);

		dL = -track_data[i].roadLeftMargin * 0x2000;
		dR = track_data[i].roadRightMargin * 0x2000;

		track_data[i].pointC_x = fixmul(c, dR);
		track_data[i].pointC_y = fixmul(t, dR);
		track_data[i].pointC_z = fixmul(s, dR);

		track_data[i].wall_normal_x = c;
		track_data[i].wall_normal_y = fixmul(t, 0x10000);
		track_data[i].wall_normal_z = s;

		track_data[i].pointL.x = (float) (track_data[i].pos_x + fixmul(c, dL)) / 0x10000;
		track_data[i].pointL.y = (float) (track_data[i].pos_y + fixmul(t, dL)) / 0x10000;
		track_data[i].pointL.z = -(float) (track_data[i].pos_z + fixmul(s, dL)) / 0x10000;
		track_data[i].pointR.x = (float) (track_data[i].pos_x + fixmul(c, dR)) / 0x10000;
		track_data[i].pointR.y = (float) (track_data[i].pos_y + fixmul(t, dR)) / 0x10000;
		track_data[i].pointR.z = -(float) (track_data[i].pos_z + fixmul(s, dR)) / 0x10000;

		dL = -track_data[i].roadLeftFence * 0x2000;
		dR = track_data[i].roadRightFence * 0x2000;

		track_data[i].pointLF.x = (float) (track_data[i].pos_x + fixmul(c, dL)) / 0x10000;
		track_data[i].pointLF.y = (float) (track_data[i].pos_y + fixmul(t, dL)) / 0x10000;
		track_data[i].pointLF.z = -(float) (track_data[i].pos_z + fixmul(s, dL)) / 0x10000;
		track_data[i].pointRF.x = (float) (track_data[i].pos_x + fixmul(c, dR)) / 0x10000;
		track_data[i].pointRF.y = (float) (track_data[i].pos_y + fixmul(t, dR)) / 0x10000;
		track_data[i].pointRF.z = -(float) (track_data[i].pos_z + fixmul(s, dR)) / 0x10000;
	}
}

void tnfs_reset_car() {
	int i;

	cheat_mode = 0;
	sound_flag = 0;

	for (i = 0; i < 10; ++i) {
		road_surface_type_array[i] = 0x100;
	}
	for (i = 0; i < 100; ++i) {
		car_specs.torque_table[i] = 0;
	}
	for (i = 0; i < 1024; ++i) {
		car_specs.slide_table[i] = 0;
	}

	memcpy(car_specs.torque_table, &g_torque_table, 64 * 4);
	memcpy(car_specs.slide_table, &g_slide_table, 64);
	memcpy(car_specs.gear_ratio_table, &g_gear_ratios, 40);

	car_specs.torque_table_entries = 60; //0x33;
	car_specs.number_of_gears = 6;
	car_specs.front_brake_percentage = 0xc000; //70%
	car_specs.front_drive_percentage = 0; //RWD
	car_specs.max_brake_force_1 = 0x133fff;
	car_specs.max_brake_force_2 = 0x133fff;
	car_specs.unknown_const_drag = 10;
	car_specs.body_roll_factor = 0x2666; //0.15
	car_specs.body_pitch_factor = 0x2666; //0.15
	car_specs.max_speed = 0x47c000; //71m/s
	car_specs.max_slip_angle = 0x1fe667; //~45deg
	car_specs.max_tire_lateral_force = 0x150000;
	car_specs.thrust_to_acc_factor = 0x66; //1577kg TNFS scale
	car_specs.final_drive_torque_ratio = 0x240;
	car_specs.final_drive_speed_ratio = 0x59947a; //3.07 * 29,18
	car_specs.rpm_idle = 500;
	car_specs.rpm_redline = 6000;
	car_specs.rev_speed_no_gas = 300;
	car_specs.rev_speed_gas_inc = 600;
	car_specs.rev_clutch_drop_rpm_dec = 180;
	car_specs.rev_clutch_drop_rpm_inc = 400;
	car_specs.negative_torque = 0xd;
	car_specs.gear_shift_delay = 16;

	// added in PC version
	car_specs.gear_torque_table[0] = 0x100;
	car_specs.gear_torque_table[1] = 0x100;
	car_specs.gear_torque_table[2] = 0x100;
	car_specs.gear_torque_table[3] = 0xdc;
	car_specs.gear_torque_table[4] = 0x10e;
	car_specs.gear_torque_table[5] = 0x113;
	car_specs.gear_torque_table[6] = 0x113;
	car_specs.gear_torque_table[7] = 0x113;
	car_specs.gear_torque_table[8] = 0;
	car_specs.gear_torque_table[9] = 0;

	car_specs.gear_upshift_rpm[0] = 5900;
	car_specs.gear_upshift_rpm[1] = 5900;
	car_specs.gear_upshift_rpm[2] = 5900;
	car_specs.gear_upshift_rpm[3] = 5900;
	car_specs.gear_upshift_rpm[4] = 5900;
	car_specs.gear_upshift_rpm[5] = 5900;
	car_specs.gear_upshift_rpm[6] = 5900;
	car_specs.gear_upshift_rpm[7] = 5900;
	car_specs.gear_upshift_rpm[8] = 5900;
	car_specs.gear_upshift_rpm[9] = 5900;

	car_data.car_data_ptr = &car_data;
	car_data.car_specs_ptr = &car_specs;

	car_data.car_length = 0x485fc; //4.45m
	car_data.car_width = 0x1ec8a; //1.92m
	car_data.weight_distribution = 0x7e49; //49% front
	car_data.weight_transfer_factor = 0x3028;
	car_data.front_friction_factor = 0xe469a;
	car_data.rear_friction_factor = 0xd10de;
	car_data.wheel_base = 0x7a9; //2.44m TNFS scale
	car_data.wheel_track = 0x3d5; //1.50m TNFS scale
	car_data.front_yaw_factor = 0x107a77;
	car_data.rear_yaw_factor = 0x107a77;
	car_data.tire_grip_front = 0xe469a;
	car_data.tire_grip_rear = 0xd10de;

	car_data.gear_selected = -1; //-2 Reverse, -1 Neutral, 0..8 Forward gears
	car_data.gear_auto_selected = 2; //0 Manual mode, 1 Reverse, 2 Neutral, 3 Drive
	car_data.gear_shift_current = -1;
	car_data.gear_shift_previous = -1;
	car_data.is_gear_engaged = 0;
	car_data.is_engine_cutoff = 0;
	car_data.is_shifting_gears = -1;
	car_data.throttle_previous_pos = 0;
	car_data.throttle = 0;
	car_data.tcs_on = 0;
	car_data.tcs_enabled = 0;
	car_data.brake = 0;
	car_data.abs_on = 0;
	car_data.abs_enabled = 0;
	car_data.is_crashed = 0;
	car_data.is_wrecked = 0;
	car_data.time_off_ground = 0;
	car_data.wheels_on_ground = 1;
	car_data.surface_type_a = 0;
	car_data.surface_type_b = 0;
	//car_data.road_segment_a = 0;
	//car_data.road_segment_b = 0;
	car_data.slope_force_lat = 0;
	car_data.slope_force_lon = 0;
	car_data.position.x = track_data[car_data.road_segment_a].pos_x;
	car_data.position.y = track_data[car_data.road_segment_a].pos_y + 150;
	car_data.position.z = track_data[car_data.road_segment_a].pos_z;
	car_data.angle_x = track_data[car_data.road_segment_a].slope * 0x400;
	car_data.angle_y = track_data[car_data.road_segment_a].heading * 0x400; //uint32 0 to 16777215(0xFFFFFF)
	car_data.angle_z = track_data[car_data.road_segment_a].slant * 0x400;
	car_data.angle_dx = 0;
	car_data.angular_speed = 0;
	car_data.speed_x = 0;
	car_data.speed_y = 0;
	car_data.speed_z = 0;
	car_data.speed = 0;
	car_data.speed_drivetrain = 0;
	car_data.speed_local_lon = 0;
	car_data.speed_local_lat = 0;
	car_data.steer_angle = 0; //int32 -1769472 to +1769472
	car_data.slide = 0;
	car_data.lap_number = 1;
	car_data.field203_0x174 = 0x1e0;
	car_data.field444_0x520 = 0;
	car_data.field445_0x524 = 0;
	car_data.unknown_flag_475 = 0;
	car_data.unknown_flag_479 = 0;
	car_data.unknown_flag_480 = 0;
	car_data.world_position.x = 0;
	car_data.world_position.y = 0;
	car_data.world_position.z = 0;
	car_data.road_ground_position.x = 0;
	car_data.road_ground_position.y = 0;
	car_data.road_ground_position.z = 0;

	car_data.rpm_vehicle = car_specs.rpm_idle;
	car_data.rpm_engine = car_specs.rpm_idle;
	car_data.rpm_redline = car_specs.rpm_redline;

	car_data.road_fence_normal.x = 65532;
	car_data.road_fence_normal.y = -46;
	car_data.road_fence_normal.z = 0;

	//surface normal (up)
	car_data.road_surface_normal.x = 0;
	car_data.road_surface_normal.y = 0x10000;
	car_data.road_surface_normal.z = 0;

	car_data.road_heading.x = 0;
	car_data.road_heading.y = 0;
	car_data.road_heading.z = 0xfcd2;

	//surface position center
	car_data.road_position.x = 0;
	car_data.road_position.y = 0;
	car_data.road_position.z = 0;

	car_data.front_edge.x = 0;
	car_data.front_edge.y = 0;
	car_data.front_edge.z = 0;
	car_data.side_edge.x = 0;
	car_data.side_edge.y = 0;
	car_data.side_edge.z = 0;

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
}

void tnfs_change_camera() {
	selected_camera++;
	if (selected_camera > 1)
		selected_camera = 0;
}

void tnfs_change_gear_automatic(int shift) {
	car_data.gear_auto_selected += shift;

	switch (car_data.gear_auto_selected) {
	case 1:
		car_data.gear_selected = -2;
		car_data.is_gear_engaged = 1;
		printf("Gear: Reverse\n");
		break;
	case 2:
		car_data.gear_selected = -1;
		car_data.is_gear_engaged = 0;
		printf("Gear: Neutral\n");
		break;
	case 3:
		car_data.gear_selected = 0;
		car_data.is_gear_engaged = 1;
		printf("Gear: Drive\n");
		break;
	}
}

void tnfs_change_gear_manual(int shift) {
	car_data.gear_selected += shift;

	switch (car_data.gear_selected) {
	case -2:
		car_data.is_gear_engaged = 1;
		printf("Gear: Reverse\n");
		break;
	case -1:
		car_data.is_gear_engaged = 0;
		printf("Gear: Neutral\n");
		break;
	default:
		car_data.is_gear_engaged = 1;
		printf("Gear: %d\n", car_data.gear_selected + 1);
		break;
	}
}

void tnfs_change_gear_up() {
	if (car_data.gear_auto_selected == 0) {
		if (car_data.gear_selected < car_specs.number_of_gears - 1)
			tnfs_change_gear_manual(+1);
	} else {
		if (car_data.gear_auto_selected < 3)
			tnfs_change_gear_automatic(+1);
	}
}

void tnfs_change_gear_down() {
	if (car_data.gear_auto_selected == 0) {
		if (car_data.gear_selected > -2)
			tnfs_change_gear_manual(-1);
	} else {
		if (car_data.gear_auto_selected > 1)
			tnfs_change_gear_automatic(-1);
	}
}

void tnfs_abs() {
	if (car_data.abs_enabled) {
		car_data.abs_enabled = 0;
		printf("ABS brakes off\n");
	} else {
		car_data.abs_enabled = 1;
		printf("ABS brakes on\n");
	}
}

void tnfs_tcs() {
	if (car_data.tcs_enabled) {
		car_data.tcs_enabled = 0;
		printf("Traction control off\n");
	} else {
		car_data.tcs_enabled = 1;
		printf("Traction control on\n");
	}
}

void tnfs_change_transmission_type() {
	if (car_data.gear_auto_selected == 0) {
		printf("Automatic Transmission mode\n");
		car_data.gear_auto_selected = 2;
		tnfs_change_gear_automatic(0);
	} else {
		printf("Manual Transmission mode\n");
		car_data.gear_auto_selected = 0;
		tnfs_change_gear_manual(0);
	}
}

void tnfs_change_traction() {
	if (car_data.car_specs_ptr->front_drive_percentage == 0x8000) {
		car_data.car_specs_ptr->front_drive_percentage = 0x10000;
		printf("Traction: FWD\n");
	} else if (car_data.car_specs_ptr->front_drive_percentage == 0) {
		car_data.car_specs_ptr->front_drive_percentage = 0x8000;
		printf("Traction: AWD\n");
	} else {
		car_data.car_specs_ptr->front_drive_percentage = 0;
		printf("Traction: RWD\n");
	}
}

void tnfs_cheat_mode() {
	if (cheat_mode == 4) {
		cheat_mode = 0;
		printf("Crashing cars cheat enabled - Press handbrake to crash\n");
	} else {
		cheat_mode = 4;
		printf("Crashing cars cheat disabled\n");
	}
}

void tnfs_crash_car() {
	tnfs_collision_rollover_start(&car_data, 0, 0, -0xa0000);
}

int tnfs_road_segment_find(tnfs_car_data *car_data, int *current) {
	int node;
	int dist1;
	int dist2;
	int changed;
	tnfs_vec3 position;
	struct tnfs_track_data *tracknode1;
	struct tnfs_track_data *tracknode2;

	changed = 0;

	if (*current != -1) {
		do {
			node = *current;

			tracknode1 = &track_data[node];
			tracknode2 = &track_data[node + 1];
			position.x = (tracknode1->pos_x + tracknode2->pos_x) >> 1;
			position.z = (tracknode1->pos_z + tracknode2->pos_z) >> 1;
			dist1 = math_vec3_distance_squared_XZ(&position, &car_data->position);

			tracknode1 = &track_data[node + 1];
			tracknode2 = &track_data[node + 2];
			position.x = (tracknode1->pos_x + tracknode2->pos_x) >> 1;
			position.z = (tracknode1->pos_z + tracknode2->pos_z) >> 1;
			dist2 = math_vec3_distance_squared_XZ(&position, &car_data->position);

			if (dist2 < dist1) {
				changed = 1;
				*current = *current + 1;
			} else if (0 < *current) {
				tracknode1 = &track_data[node - 1];
				tracknode2 = &track_data[node];
				position.x = (tracknode1->pos_x + tracknode2->pos_x) >> 1;
				position.z = (tracknode1->pos_z + tracknode2->pos_z) >> 1;
				dist2 = math_vec3_distance_squared_XZ(&position, &car_data->position);

				if (dist2 < dist1) {
					node = *current;
					*current = node + -1;
					if (node + -1 < 0) {
						*current = 0;
					} else {
						changed = 1;
					}
				}
			}
		} while (node != *current);
	}

	// added part
	node = *current;
	roadLeftMargin = track_data[node].roadLeftMargin;
	roadRightMargin = track_data[node].roadRightMargin;
	roadLeftFence = track_data[node].roadLeftFence;
	roadRightFence = track_data[node].roadRightFence;
	car_data->road_ground_position.x = track_data[node].pos_x;
	car_data->road_ground_position.y = track_data[node].pos_y;
	car_data->road_ground_position.z = track_data[node].pos_z;
	// end

	return changed;
}

int tnfs_road_segment_update(tnfs_car_data *car) {
	int found;
	int segment;
	segment = car->road_segment_a;
	found = tnfs_road_segment_find(car, &segment);
	car->road_segment_a = segment;
	car->road_segment_b = segment;
	return found;
}

void tnfs_track_segment_boundaries(tnfs_car_data *car) {
	int node, next;
	tnfs_vec3 heading;
	tnfs_vec3 wall_normal;

	node = car->road_segment_a;
	next = node + 1;

	car->road_position.x = track_data[node].pos_x;
	car->road_position.y = track_data[node].pos_y;
	car->road_position.z = track_data[node].pos_z;

	heading.x = track_data[next].pos_x - car->road_position.x;
	heading.y = track_data[next].pos_y - car->road_position.y;
	heading.z = track_data[next].pos_z - car->road_position.z;

	wall_normal.x = track_data[node].wall_normal_x;
	wall_normal.y = track_data[node].wall_normal_y;
	wall_normal.z = track_data[node].wall_normal_z;

	math_vec3_normalize(&heading);

	car->road_fence_normal.x = wall_normal.x;
	car->road_fence_normal.y = wall_normal.y;
	car->road_fence_normal.z = wall_normal.z;

	// 0, 0x10000, 0 => up
	car->road_surface_normal.x = fixmul(heading.y, wall_normal.z) - fixmul(heading.z, wall_normal.y);
	car->road_surface_normal.y = fixmul(heading.z, wall_normal.x) - fixmul(heading.x, wall_normal.z);
	car->road_surface_normal.z = fixmul(heading.x, wall_normal.y) - fixmul(heading.y, wall_normal.x);

	car->road_heading.x = heading.x;
	car->road_heading.y = heading.y;
	car->road_heading.z = heading.z;

	// ...
}

void tnfs_update() {
	// update camera
	switch (selected_camera) {
	case 1: //heli cam
		camera_position.x = car_data.position.x;
		camera_position.y = car_data.position.y + 0x60000;
		camera_position.z = car_data.position.z - 0x100000;
		break;
	default: //chase cam
		camera_position.x = car_data.position.x;
		camera_position.y = car_data.position.y + 0x50000;
		camera_position.z = car_data.position.z - 0x96000;
		break;
	}

	if (car_data.collision_data.crashed_time == 0) {
		// driving mode loop
		tnfs_driving_main();
		// update render matrix
		matrix_create_from_pitch_yaw_roll(&car_data.matrix, car_data.angle_x + car_data.body_pitch, car_data.angle_y, car_data.angle_z + car_data.body_roll);
	} else {
		// crash mode loop
		tnfs_collision_main(&car_data);
	}
}
