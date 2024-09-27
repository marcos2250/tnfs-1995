/*
 * globals, structs, and common TNFS functions
 */
#include "tnfs_math.h"
#include "tnfs_base.h"
#include "tnfs_files.h"
#include "tnfs_fiziks.h"
#include "tnfs_collision_2d.h"
#include "tnfs_collision_3d.h"
#include "tnfs_ai.h"

tnfs_track_data track_data[2400];
tnfs_surface_type road_surface_type_array[3];
tnfs_track_speed g_track_speed[600];

tnfs_car_specs car_specs;
tnfs_car_data g_car_array[8];
tnfs_car_data *g_car_ptr_array[8]; // 00153ba0/00153bec 8010c720/800f7e60
tnfs_car_data *player_car_ptr;
tnfs_ai_skill_cfg g_ai_skill_cfg;
tnfs_ai_opp_data g_ai_opp_data[8];
tnfs_stats_data g_stats_data[8];

int g_total_cars_in_scene = 7;
int g_racer_cars_in_scene = 2; // (including player) 001670AB DAT_8010d1c8
int g_number_of_players = 1; //001670af 8010d1cc
int g_number_of_cops = 1; //001670B3 8010d1d0
int g_number_of_traffic_cars = 4; //001670BB

// settings/flags
char is_drifting;
int g_game_time = 0;
int road_segment_count = 0;
int sound_flag = 0;
int g_selected_cheat = 0;
int cheat_crashing_cars = 0;
int g_game_settings = 0;
char g_control_throttle;
char g_control_brake;
signed char g_control_steer;
int g_police_on_chase = 0; //000fdb90
int g_police_speeding_ticket = 0; //0016513C
int g_police_chase_time = 0; //0016533c

tnfs_camera camera;
int selected_camera = 0;
tnfs_vec3 camera_position;

/* tire grip slide table  */
static const unsigned char g_slide_table[64] = {
		// front values
		2, 14, 26, 37, 51, 65, 101, 111, 120, 126, 140, 150, 160, 165, 174, 176, 176, 176, 176, 176, 176, 177, 177, 177, 200, 200, 200, 174, 167, 161, 152, 142,
		// rear values
		174, 174, 174, 174, 175, 176, 177, 177, 200, 200, 200, 200, 177, 177, 177, 177, 177, 177, 177, 177, 177, 177, 177, 177, 200, 176, 173, 170, 165, 162, 156, 153
};

/* engine torque table, for 200 rpm values */
static const unsigned int g_torque_table[120] = {
		1000, 440, 1200, 450, 1400, 460, 1600, 470, 1800, 480, 2000, 490, 2200, 530, 2400, 560, 2600, 570, 2800, 580,
		3000, 590, 3200, 592, 3400, 600, 3600, 610, 3800, 600, 4000, 580, 4200, 570, 4400, 570, 4600, 560, 4800, 560,
		5000, 560, 5200, 550, 5400, 550, 5600, 530, 5800, 510, 6000, 500, 6200, 490, 6400, 480, 6600, 470, 6800, 460
};

int DAT_800eb6a4 = 0; //800eb6a4
int DAT_8010d310 = 0; //8010d310

int DAT_000F9BB0 = 0;
int DAT_000f99e4 = 0x10000;
int DAT_000f99e8 = 0x34000;
int DAT_000f99ec = 10; //800eae14
int DAT_000f99f0 = 0x8000;
int DAT_000FDB94 = 0;
int DAT_000FDCEC = 0;
int DAT_000FDCF0 = 0;
int DAT_000f9A70 = 0;
int DAT_001039d4 = 0xFFFF; //800db6bc
int DAT_001039d8 = 0x12345678; //800db6c0
int DAT_001039dc = 0x12345679; // random value
int DAT_00144914 = 0;
int DAT_00143844 = 0;
int DAT_0014DCC4 = 0;
int DAT_0014dccc = 0xFFFF; // segment id mask
int DAT_00153B20 = 0;
int DAT_00153B24 = 0;
tnfs_car_data * DAT_00153BC4 = 0;
int DAT_00165148 = 0;
int DAT_00165340 = 0;
int DAT_0016707C = 0; //player car id?


/* create a random track and a generic car */

void auto_generate_track() {
	int pos_x = 0;
	int pos_y = 0;
	int pos_z = 0;
	int slope = 0;
	int slant = 0;
	int rnd = 0;
	int i;

	road_segment_count = 2400;

	for (i = 0; i < 2400; i++) {

		if (i % 30 == 0)
			rnd = rand();

		if (rnd & 128) {
			if (rnd & 64) {
				slope -= 10;
			} else {
				slope += 10;
			}
		} else {
			slope *= 0.9;
		}
		if (rnd & 32) {
			if (rnd & 16) {
				slant -= 20;
			} else {
				slant += 20;
			}
		} else {
			slant *= 0.9;
		}

		if (slope > 0x3FF) slope = 0x3FF;
		if (slope < -0x3FF) slope = -0x3FF;
		if (slant > 0x3FF) slant = 0x3FF;
		if (slant < -0x3FF) slant = -0x3FF;

		track_data[i].roadLeftFence = 0x50;
		track_data[i].roadRightFence = 0x50;
		track_data[i].roadLeftMargin = 0x35;
		track_data[i].roadRightMargin = 0x35;
		track_data[i].num_lanes = 0x11;
		track_data[i].fence_flag = 0;
		track_data[i].shoulder_surface_type = 0x22;
		track_data[i].item_mode = 0x3;

		track_data[i].slope = slope;
		track_data[i].heading = slant << 2;
		track_data[i].slant = slant;
		track_data[i].pos.x = pos_x;
		track_data[i].pos.y = pos_y;
		track_data[i].pos.z = pos_z;

		track_data[i].side_normal_x = (short)(math_cos_3(track_data[i].heading * -0x400) / 2);
		track_data[i].side_normal_y = (short)(math_tan_3(track_data[i].slant * -0x400) / 2);
		track_data[i].side_normal_z = (short)(math_sin_3(track_data[i].heading * -0x400) / 2);

		// next segment
		pos_x += fixmul(math_sin_3(track_data[i].heading * 0x400), 0x80000);
		pos_y += fixmul(math_tan_3(track_data[i].slope * 0x400), 0x80000);
		pos_z += fixmul(math_cos_3(track_data[i].heading * 0x400), 0x80000);
	}

	// track section speed
	for (i = 0; i < 600; i++) {
		g_track_speed[i].top_speed = 0x42;
		g_track_speed[i].legal_speed = 0x1b;
		g_track_speed[i].safe_speed = 0x2c;
	}
}

void tnfs_init_track(char *tri_file) {
	int i;
	int heading, s, c, t, dL, dR;

	// try to read a TRI file if given, if not, generate a random track
	if (tri_file == 0 || !read_tri_file(tri_file)) {
		auto_generate_track();
	}

	// model track for rendering
	for (i = 0; i < road_segment_count; i++) {
		heading = track_data[i].heading * -0x400;
		s = math_sin_3(heading);
		c = math_cos_3(heading);
		t = math_tan_3(track_data[i].slant * -0x400);

		dL = (int)(track_data[i].roadLeftMargin) * -0x2000;
		dR = (int)track_data[i].roadRightMargin * 0x2000;

		track_data[i].vf_margin_L.x = (float) (track_data[i].pos.x + fixmul(c, dL)) / 0x10000;
		track_data[i].vf_margin_L.y = (float) (track_data[i].pos.y + fixmul(t, dL)) / 0x10000;
		track_data[i].vf_margin_L.z = -(float) (track_data[i].pos.z + fixmul(s, dL)) / 0x10000;
		track_data[i].vf_margin_R.x = (float) (track_data[i].pos.x + fixmul(c, dR)) / 0x10000;
		track_data[i].vf_margin_R.y = (float) (track_data[i].pos.y + fixmul(t, dR)) / 0x10000;
		track_data[i].vf_margin_R.z = -(float) (track_data[i].pos.z + fixmul(s, dR)) / 0x10000;

		dL = (int)(track_data[i].roadLeftFence) * -0x2000;
		dR = (int)(track_data[i].roadRightFence) * 0x2000;

		track_data[i].vf_fence_L.x = (float) (track_data[i].pos.x + fixmul(c, dL)) / 0x10000;
		track_data[i].vf_fence_L.y = (float) (track_data[i].pos.y + fixmul(t, dL)) / 0x10000;
		track_data[i].vf_fence_L.z = -(float) (track_data[i].pos.z + fixmul(s, dL)) / 0x10000;
		track_data[i].vf_fence_R.x = (float) (track_data[i].pos.x + fixmul(c, dR)) / 0x10000;
		track_data[i].vf_fence_R.y = (float) (track_data[i].pos.y + fixmul(t, dR)) / 0x10000;
		track_data[i].vf_fence_R.z = -(float) (track_data[i].pos.z + fixmul(s, dR)) / 0x10000;
	}
}

void tnfs_init_surface_constants() {
	int i;

	// Road Traction sections (surface constants)
	// Tarmac
	road_surface_type_array[0].roadFriction = 0x100;
	road_surface_type_array[0].velocity_drag = 0x100;
	road_surface_type_array[0].surface_drag = 0x3333;
	road_surface_type_array[0].is_unpaved = 0;
	// Shoulder
	road_surface_type_array[1].roadFriction = 0x100;
	road_surface_type_array[1].velocity_drag = 0x1400;
	road_surface_type_array[1].surface_drag = 0x3333;
	road_surface_type_array[1].is_unpaved = 1;
	// Shoulder2
	road_surface_type_array[2].roadFriction = 0x100;
	road_surface_type_array[2].velocity_drag = 0x1400;
	road_surface_type_array[2].surface_drag = 0x3333;
	road_surface_type_array[2].is_unpaved = 1;

	// rally mode surface constants
	if (g_game_settings & 0x20) {
		for (i = 0; i < 3; i++) {
			road_surface_type_array[i].roadFriction = 0x180;
			road_surface_type_array[i].velocity_drag = 0x80;
			road_surface_type_array[i].surface_drag = 0x1999;
			road_surface_type_array[i].is_unpaved = 1;
		}
	}
}

void tnfs_create_car_specs() {
	int i;

	car_specs.mass_front = 0x3148000; //788kg
	car_specs.mass_rear = 0x3148000; //788kg
	car_specs.mass_total = 0x6290000; //1577kg
	car_specs.inverse_mass_front = 0x53; //1/788kg
	car_specs.inverse_mass_rear = 0x53; //1/788kg
	car_specs.inverse_mass = 0x29; // 1/1577
	car_specs.front_drive_percentage = 0; //RWD
	car_specs.front_brake_percentage = 0xc000; //70%
	car_specs.centre_of_gravity_height = 0x7581; //0.459
	car_specs.max_brake_force_1 = 0x133fff;
	car_specs.max_brake_force_2 = 0x133fff;
	car_specs.drag = 0x8000; //0.5
	car_specs.top_speed = 0x47cccc; //71m/s
	car_specs.efficiency = 0xb333; //0.7
	car_specs.wheelbase = 0x270A3; //2.44m
	car_specs.burnOutDiv = 0x68EB; //0.4
	car_specs.wheeltrack = 0x18000; //1.50m
	car_specs.mps_to_rpm_factor = 0x59947a; //conversion=rpm/speed/gear=1200/26.79/0.5= 26.79
	car_specs.number_of_gears = 8;
	car_specs.final_drive = 0x311eb; //3.07
	car_specs.wheel_roll_radius = 0x47ae; //28cm
	car_specs.inverse_wheel_radius = 0x3924a; //3.57
	car_specs.gear_ratio_table[0] = -152698; //-2.33 reverse
	car_specs.gear_ratio_table[1] = 0x1999; //neutral
	car_specs.gear_ratio_table[2] = 0x2a8f5; //2.66
	car_specs.gear_ratio_table[3] = 0x1c7ae; //1.78
	car_specs.gear_ratio_table[4] = 0x14ccc; //1.30
	car_specs.gear_ratio_table[5] = 0x10000; //1.00
	car_specs.gear_ratio_table[6] = 0xcccc;  //0.75
	car_specs.gear_ratio_table[7] = 0x8000;  //0.50
	car_specs.torque_table_entries = 0x33; //60;
	car_specs.front_roll_stiffness = 0x2710000; //10000
	car_specs.rear_roll_stiffness = 0x2710000; //10000
	car_specs.roll_axis_height = 0x476C; //0.279
	car_specs.cutoff_slip_angle = 0x1fe667; //~45deg
	car_specs.rpm_redline = 6000;
	car_specs.rpm_idle = 500;

	// torque table
	memcpy(car_specs.torque_table, &g_torque_table, sizeof(g_torque_table));

	car_specs.gear_upshift_rpm[0] = 5900;
	car_specs.gear_upshift_rpm[1] = 5900;
	car_specs.gear_upshift_rpm[2] = 5900;
	car_specs.gear_upshift_rpm[3] = 5900;
	car_specs.gear_upshift_rpm[4] = 5900;
	car_specs.gear_upshift_rpm[5] = 5900;
	car_specs.gear_upshift_rpm[6] = 5900;

	car_specs.gear_efficiency[0] = 0x100;
	car_specs.gear_efficiency[1] = 0x100;
	car_specs.gear_efficiency[2] = 0x100;
	car_specs.gear_efficiency[3] = 0xdc;
	car_specs.gear_efficiency[4] = 0x10e;
	car_specs.gear_efficiency[5] = 0x113;
	car_specs.gear_efficiency[6] = 0x113;
	car_specs.gear_efficiency[7] = 0x113;

	car_specs.inertia_factor = 0x8000; //0.5
	car_specs.body_roll_factor = 0x2666; //0.15
	car_specs.body_pitch_factor = 0x2666; //0.15
	car_specs.front_friction_factor = 0x2b331;
	car_specs.rear_friction_factor = 0x2f332;
	car_specs.body_length = 0x47333; //4.45m
	car_specs.body_width = 0x1eb85; //1.92m
	car_specs.lateral_accel_cutoff = 0x158000;
	car_specs.final_drive_torque_ratio = 0x280;
	car_specs.thrust_to_acc_factor = 0x66;
	car_specs.shift_timer = 3;
	car_specs.noGasRpmDec = 0x12c; //300
	car_specs.gasRpmInc = 0x258; //600
	car_specs.clutchDropRpmDec = 0xb4; //180
	car_specs.clutchDropRpmInc = 0x12c; //300
	car_specs.negTorque = 0xd; //0.0001
	car_specs.ride_height = 0x1c0cc; //1.05
	car_specs.centre_y = 0x4c; //76

	// tire grip-slip angle tables
	for (i = 0; i < 1024; i++) {
		car_specs.grip_table[i] = g_slide_table[i >> 4];
	}
}

void tnfs_reset_car(tnfs_car_data *car) {

	car->gear_selected = -1; //-2 Reverse, -1 Neutral, 0..8 Forward gears
	car->gear_auto_selected = 2; //0 Manual mode, 1 Reverse, 2 Neutral, 3 Drive
	car->gear_shift_current = -1;
	car->gear_shift_previous = -1;
	car->gear_shift_interval = 16;
	car->tire_skid_front = 0;
	car->tire_skid_rear = 0;
	car->is_gear_engaged = 0;
	car->handbrake = 0;
	car->is_engine_cutoff = 0;
	car->is_shifting_gears = -1;
	car->throttle_previous_pos = 0;
	car->throttle = 0;
	car->tcs_on = 0;
	//car->tcs_enabled = 0;
	car->brake = 0;
	car->abs_on = 0;
	//car->abs_enabled = 0;
	car->is_crashed = 0;
	car->is_wrecked = 0;
	car->time_off_ground = 0;
	car->slide_front = 0;
	car->slide_rear = 0;
	car->wheels_on_ground = 1;
	car->surface_type = 0;
	car->surface_type_b = 0;
	//car->road_segment_a = 0;
	//car->road_segment_b = 0;
	car->slope_force_lat = 0;
	car->unknown_flag_3DD = 0;
	car->slope_force_lon = 0;

	car->position.x = track_data[car->road_segment_a].pos.x;
	car->position.y = track_data[car->road_segment_a].pos.y + 150;
	car->position.z = track_data[car->road_segment_a].pos.z;

	car->angle_x = track_data[car->road_segment_a].slope * 0x400;
	car->angle_y = track_data[car->road_segment_a].heading * 0x400;
	car->angle_z = track_data[car->road_segment_a].slant * 0x400;

	// convert slope/slant angles to signed values
	if (car->angle_x > 0x800000)
		car->angle_x -= 0x1000000;
	if (car->angle_z > 0x800000)
		car->angle_z -= 0x1000000;

	car->angle_x *= -1;
	car->angle_z *= -1;

	if (car->ai_state & 0x1000) {
		car->angle_y *= -1;
	}

	car->body_pitch = 0;
	car->body_roll = 0;
	car->angle_dy = 0;
	car->angular_speed = 0;
	car->speed_x = 0;
	car->speed_y = 0;
	car->speed_z = 0;
	car->speed = 0;
	car->car_road_speed = 0;
	car->speed_drivetrain = 0;
	car->speed_local_lat = 0;
	car->speed_local_vert = 0;
	car->speed_local_lon = 0;
	car->steer_angle = 0; //int32 -1769472 to +1769472
	car->tire_grip_loss = 0;
	car->accel_lat = 0;
	car->accel_lon = 0;
	car->road_grip_increment = 0;
	car->lap_number = 1;

	car->world_position = car->position;
	car->road_ground_position = car->position;

	car->rpm_vehicle = car_specs.rpm_idle;
	car->rpm_engine = car_specs.rpm_idle;
	car->rpm_redline = car_specs.rpm_redline;

	car->road_fence_normal.x = 0x10000;
	car->road_fence_normal.y = 0;
	car->road_fence_normal.z = 0;

	//surface normal (up)
	car->road_surface_normal.x = 0;
	car->road_surface_normal.y = 0x10000;
	car->road_surface_normal.z = 0;

	//track next node (north)
	car->road_heading.x = 0;
	car->road_heading.y = 0;
	car->road_heading.z = 0x10000;

	//surface position center
	car->road_position.x = 0;
	car->road_position.y = 0;
	car->road_position.z = 0;

	car->front_edge.x = 0x10000;
	car->front_edge.y = 0;
	car->front_edge.z = 0;

	car->side_edge.x = 0;
	car->side_edge.y = 0x10000;
	car->side_edge.z = 0;

	math_matrix_identity(&car->matrix);
	math_matrix_identity(&car->collision_data.matrix);

	car->collision_data.position.x = car->position.x;
	car->collision_data.position.y = car->position.y;
	car->collision_data.position.z = -car->position.z;
	car->collision_data.speed.x = 0;
	car->collision_data.speed.y = 0;
	car->collision_data.speed.z = 0;
	car->collision_data.field4_0x48.x = 0;
	car->collision_data.field4_0x48.y = 0;
	car->collision_data.field4_0x48.z = -0x9cf5c;
	car->collision_data.state_timer = 0;
	car->collision_data.angular_speed.x = 0;
	car->collision_data.angular_speed.y = 0;
	car->collision_data.angular_speed.z = 0;

	// ai car flags
	car->speed_target = 0;
	car->collision_data.field_084 = 0;
	car->collision_data.field_088 = 0;
	car->collision_data.field_08c = 0;
	car->collision_data.traffic_speed_factor = 0x10000; //0xb333; //0xcccc
	car->field_158 = 0;
	car->lane_slack = 0;
	car->crash_state = 3;
	car->field_461 = 0;
	car->field_4e9 = 7;
	car->field_158 = 0;

	if (car == player_car_ptr) {
		// player car
		car->car_model_id = 5; //DVIPER
		car->crash_state = 2;
		car->ai_state = 0x1e0;
		car->field_158 = 1;
		g_police_on_chase = 0;
	} else {
		// ai cars
		car->crash_state = 3;
		car->steer_angle = car->angle_y;
		if (car->car_id == 2) {
			// police car
			car->ai_state = 0x1e8;
			g_police_on_chase = 0;
			printf("respawn cop car\n");
		}
	}
}

void tnfs_init_car(tnfs_car_data *car) {
	int i, aux;

	// load car specs
	if (!read_pbs_file("carspecs.pbs")) {
		tnfs_create_car_specs();
	}

	car->crash_state = 2;
	car->car_id = 0;
	car->field_4e9 = 7;
	car->position.z = 0; //0x600000;
	car->road_segment_a = 0; //0x10;
	car->road_segment_b = 0; //0x10;
	car->lap_number = 1;

	// rally mode tweaks
	if (g_game_settings & 0x20) {
		for (i = 0; i < 512; i++)
			car_specs.grip_table[i + 512] = car_specs.grip_table[i];

		car_specs.lateral_accel_cutoff = 0xd2000;
		car_specs.centre_of_gravity_height = 0x7581 * 3 / 2;
		car_specs.front_friction_factor = 0x2b331 / 2;
		car_specs.rear_friction_factor = 0x2f332 / 2;
	}

	// net wheel torque values
	i = 1;
	do {
		car_specs.torque_table[i] = //
				math_mul(math_mul(math_mul(math_mul(
						car_specs.torque_table[i] << 0x10,
						car_specs.final_drive),
						car_specs.efficiency),
						car_specs.inverse_wheel_radius),
						car_specs.inverse_mass);
		i += 2;
	} while (i < car_specs.torque_table_entries);

	car->car_length = car_specs.body_length;
	car->car_width = car_specs.body_width;
	car->abs_enabled = 0;
	car->tcs_enabled = 0;
	car->gear_auto_selected = 0;
	car->drag_const_0x4a8 = 0;
	car->drag_const_0x4aa = 0;

	/* Fiziks_InitCar() */
	tnfs_init_surface_constants();

	car->weight_distribution_front = math_mul(car_specs.mass_front, car_specs.inverse_mass);
	car->weight_distribution_rear = math_mul(car_specs.mass_rear, car_specs.inverse_mass);

	// unused specs
	car->mass_front = math_mul(car_specs.mass_total, car_specs.inverse_mass_front);
	car->mass_rear = math_mul(car_specs.mass_total, car_specs.inverse_mass_rear);

	// drag coefficient to deccel factor
	car_specs.drag = math_mul(car_specs.drag, car_specs.inverse_mass);

	car->weight_transfer_factor = math_mul(car_specs.centre_of_gravity_height, car_specs.burnOutDiv);
	car->front_friction_factor = math_mul(0x9cf5c, math_mul(car_specs.front_friction_factor, car->weight_distribution_rear));
	car->rear_friction_factor = math_mul(0x9cf5c, math_mul(car_specs.rear_friction_factor, car->weight_distribution_front));

	car->drag_const_0x4ac = (short) (car->drag_const_0x4a8 << 1);
	car->tire_grip_front = car->front_friction_factor;
	car->tire_grip_rear = car->rear_friction_factor;
	car->drag_const_0x4ae = (short) (car->drag_const_0x4aa << 1);

	car->drag_const_0x4a8 = fix8(fix8(car_specs.front_friction_factor) * 10 * car->drag_const_0x4a8);
	car->drag_const_0x4aa = fix8(fix8(car_specs.rear_friction_factor) * 10 * car->drag_const_0x4aa);

	aux = math_mul(math_mul(car_specs.wheelbase, car_specs.wheelbase), 0x324);

	car->wheel_base = math_div(aux, car_specs.wheelbase);
	car->moment_of_inertia = math_div(math_mul(aux, car_specs.inertia_factor), car_specs.wheelbase);
	car->front_yaw_factor = math_div(math_mul(car_specs.wheelbase, car->weight_distribution_front), aux);
	car->rear_yaw_factor = math_div(math_mul(car_specs.wheelbase, car->weight_distribution_rear), aux);

	car->collision_height_offset = 0x92f1;
	car->collision_data.mass = 0x10a1c;
	car->collision_data.moment_of_inertia = 0x10000;
	car->collision_data.linear_acc_factor = 0xf646;
	car->collision_data.angular_acc_factor = 0x7dd4;
	car->collision_data.size.x = car_specs.body_width / 2;
	car->collision_data.size.y = 0x92f1;
	car->collision_data.size.z = car_specs.body_length / 2;
	car->collision_data.edge_length = math_vec3_length(&car->collision_data.size);
}

/* basic game controls */

void tnfs_controls_update() {
	// steer ramp
	if (g_control_steer > 0) {
		g_car_array[0].steer_angle += 0x6C000;
		if (g_car_array[0].steer_angle > 0x1B0000)
			g_car_array[0].steer_angle = 0x1B0000;
	} else if (g_control_steer < 0) {
		g_car_array[0].steer_angle -= 0x6C000;
		if (g_car_array[0].steer_angle < -0x1B0000)
			g_car_array[0].steer_angle = -0x1B0000;
	} else {
		g_car_array[0].steer_angle >>= 1;
	}
	// throttle ramp
	if (g_control_throttle) {
		g_car_array[0].throttle += 0x11;
		if (g_car_array[0].throttle > 0xFF)
			g_car_array[0].throttle = 0xFF;
	} else {
		g_car_array[0].throttle -= 0xC;
		if (g_car_array[0].throttle < 0)
			g_car_array[0].throttle = 0;
	}
	// brake ramp
	if (g_control_brake) {
		g_car_array[0].brake += g_car_array[0].brake < 140 ? 0xC : 2;
		if (g_car_array[0].brake > 0xFF)
			g_car_array[0].brake = 0xFF;
	} else {
		g_car_array[0].brake -= 0x33;
		if (g_car_array[0].brake < 0)
			g_car_array[0].brake = 0;
	}
}

void tnfs_change_camera() {
	selected_camera++;
	if (selected_camera > 3)
		selected_camera = 0;
}

void tnfs_change_gear_automatic(int shift) {
	g_car_array[0].gear_auto_selected += shift;

	switch (g_car_array[0].gear_auto_selected) {
	case 1:
		g_car_array[0].gear_selected = -2;
		g_car_array[0].is_gear_engaged = 1;
		printf("Gear: Reverse\n");
		break;
	case 2:
		g_car_array[0].gear_selected = -1;
		g_car_array[0].is_gear_engaged = 0;
		printf("Gear: Neutral\n");
		break;
	case 3:
		g_car_array[0].gear_selected = 0;
		g_car_array[0].is_gear_engaged = 1;
		printf("Gear: Drive\n");
		break;
	}
}

void tnfs_change_gear_manual(int shift) {
	g_car_array[0].gear_selected += shift;

	switch (g_car_array[0].gear_selected) {
	case -2:
		g_car_array[0].is_gear_engaged = 1;
		printf("Gear: Reverse\n");
		break;
	case -1:
		g_car_array[0].is_gear_engaged = 0;
		printf("Gear: Neutral\n");
		break;
	default:
		g_car_array[0].is_gear_engaged = 1;
		printf("Gear: %d\n", g_car_array[0].gear_selected + 1);
		break;
	}
}

void tnfs_change_gear_up() {
	if (g_car_array[0].gear_auto_selected == 0) {
		if (g_car_array[0].gear_selected < car_specs.number_of_gears - 1)
			tnfs_change_gear_manual(+1);
	} else {
		if (g_car_array[0].gear_auto_selected < 3)
			tnfs_change_gear_automatic(+1);
	}
}

void tnfs_change_gear_down() {
	if (g_car_array[0].gear_auto_selected == 0) {
		if (g_car_array[0].gear_selected > -2)
			tnfs_change_gear_manual(-1);
	} else {
		if (g_car_array[0].gear_auto_selected > 1)
			tnfs_change_gear_automatic(-1);
	}
}

/* additional features */

void tnfs_abs() {
	if (g_car_array[0].abs_enabled) {
		g_car_array[0].abs_enabled = 0;
		printf("ABS brakes off\n");
	} else {
		g_car_array[0].abs_enabled = 1;
		printf("ABS brakes on\n");
	}
}

void tnfs_tcs() {
	if (g_car_array[0].tcs_enabled) {
		g_car_array[0].tcs_enabled = 0;
		printf("Traction control off\n");
	} else {
		g_car_array[0].tcs_enabled = 1;
		printf("Traction control on\n");
	}
}

void tnfs_change_transmission_type() {
	if (g_car_array[0].gear_auto_selected == 0) {
		printf("Automatic Transmission mode\n");
		g_car_array[0].gear_auto_selected = 2;
		tnfs_change_gear_automatic(0);
	} else {
		printf("Manual Transmission mode\n");
		g_car_array[0].gear_auto_selected = 0;
		tnfs_change_gear_manual(0);
	}
}

void tnfs_change_traction() {
	if (g_car_array[0].car_specs_ptr->front_drive_percentage == 0x8000) {
		g_car_array[0].car_specs_ptr->front_drive_percentage = 0x10000;
		printf("Traction: FWD\n");
	} else if (g_car_array[0].car_specs_ptr->front_drive_percentage == 0) {
		g_car_array[0].car_specs_ptr->front_drive_percentage = 0x8000;
		printf("Traction: AWD\n");
	} else {
		g_car_array[0].car_specs_ptr->front_drive_percentage = 0;
		printf("Traction: RWD\n");
	}
}

void tnfs_cheat_mode() {
	g_selected_cheat++;
	cheat_crashing_cars = 0;
	g_game_settings = 0;

	if (g_selected_cheat > 2) {
		g_selected_cheat = 0;
		printf("No easter egg active.\n");
	}
	if (g_selected_cheat == 1) {
		cheat_crashing_cars = 4;
		printf("Cheat mode: Crashing cars - Press handbrake to crash\n");
	}
	if (g_selected_cheat == 2) {
		printf("Cheat mode: Rally Mode\n");
		g_game_settings = 0x20;
	}

	tnfs_init_car(g_car_ptr_array[0]);
	tnfs_reset_car(g_car_ptr_array[0]);
}

void tnfs_crash_car() {
	int i;
	for (i = 1; i < g_total_cars_in_scene; i++) {
		tnfs_collision_rollover_start(g_car_ptr_array[i], 0, 0, -0xa0000);
	}
}

/* common stub functions */

void tnfs_sfx_play(int a, int b, int c, int d, int e, int f) {
	printf("sound %i %i\n", b, f);
}

void tnfs_replay_highlight_record(char a) {
	if (g_game_time % 30 == 0)
		printf("replay highlight %i\n", a);
}

void tnfs_camera_set(tnfs_camera * camera, int id) {
	// stub
}

/* common original TNFS functions */

void tnfs_car_local_position_vector(tnfs_car_data *car, int *angle, int *length) {
	int x;
	int y;
	int z;
	int heading;

	x = car->position.x - track_data[car->road_segment_a].pos.x;
	y = car->position.y - track_data[car->road_segment_a].pos.y;
	z = car->position.z - track_data[car->road_segment_a].pos.z;

	heading = track_data[car->road_segment_a].heading * 0x400;

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

int tnfs_road_segment_find(tnfs_car_data *car, int *current) {
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
			position.x = (tracknode1->pos.x + tracknode2->pos.x) >> 1;
			position.z = (tracknode1->pos.z + tracknode2->pos.z) >> 1;
			dist1 = math_vec3_distance_squared_XZ(&position, &car->position);

			tracknode1 = &track_data[node + 1];
			tracknode2 = &track_data[node + 2];
			position.x = (tracknode1->pos.x + tracknode2->pos.x) >> 1;
			position.z = (tracknode1->pos.z + tracknode2->pos.z) >> 1;
			dist2 = math_vec3_distance_squared_XZ(&position, &car->position);

			if (dist2 < dist1) {
				changed = 1;
				*current = *current + 1;
			} else if (0 < *current) {
				tracknode1 = &track_data[node - 1];
				tracknode2 = &track_data[node];
				position.x = (tracknode1->pos.x + tracknode2->pos.x) >> 1;
				position.z = (tracknode1->pos.z + tracknode2->pos.z) >> 1;
				dist2 = math_vec3_distance_squared_XZ(&position, &car->position);

				if (dist2 < dist1) {
					node = *current;
					*current = node - 1;
					if (node - 1 < 0) {
						*current = 0;
					} else {
						changed = 1;
					}
				}
			}
		} while (node != *current);
	}
	return changed;
}

int tnfs_road_segment_update(tnfs_car_data *car) {
	int changed;
	int segment;
	segment = car->road_segment_a;
	changed = tnfs_road_segment_find(car, &segment);
	car->road_segment_a = segment;
	car->road_segment_b = segment;
	return changed;
}

/*
 * #Traffic Speed Factors: Traffic Cars will select one of these 4 multipliers,
 * # and drive at the "Safe Speed" times this multiplier
 */
void tnfs_ai_get_speed_factor(tnfs_car_data *car) {
	int skill;
	DAT_001039d8 = DAT_001039d4 * DAT_001039dc;
	DAT_001039d4 = DAT_001039d8 & 0xffff;
	skill = (DAT_001039d8 & 0xffff00) >> 8 & 3;
	car->collision_data.traffic_speed_factor = g_ai_skill_cfg.traffic_speed_factors[skill];
}

/*
 * #Lane Slack.  Determines how close the cars drive along the centre line.  Measured in units from the
 * # centre of a lane towards the centre of the road.
 */
void tnfs_ai_get_lane_slack(tnfs_car_data *car) {
	int skill;
	DAT_001039d8 = DAT_001039d4 * DAT_001039dc;
	DAT_001039d4 = DAT_001039d8 & 0xffff;
	skill = (DAT_001039d8 & 0xffff00) >> 8 & 3;
	if ((car->ai_state & 4) == 0) {
		car->lane_slack = g_ai_skill_cfg.lane_slack[skill];
	} else {
		car->lane_slack = g_ai_opp_data[car->car_id2].lane_slack[skill];
	}
}

void tnfs_track_update_vectors(tnfs_car_data *car) {
	int node;
	tnfs_vec3 heading;
	tnfs_vec3 wall_normal;
	int iVar9, uVar5;

	// current node
	node = car->road_segment_a;
	car->road_position.x = track_data[node].pos.x;
	car->road_position.y = track_data[node].pos.y;
	car->road_position.z = track_data[node].pos.z;

	wall_normal.x = track_data[node].side_normal_x << 1;
	wall_normal.y = track_data[node].side_normal_y << 1;
	wall_normal.z = track_data[node].side_normal_z << 1;

	// next node vector
	node++;
	heading.x = track_data[node].pos.x - car->road_position.x;
	heading.y = track_data[node].pos.y - car->road_position.y;
	heading.z = track_data[node].pos.z - car->road_position.z;

	math_vec3_normalize_fast(&heading);

	// 0x10000, 0, 0 => points to right side of road
	car->road_fence_normal.x = wall_normal.x;
	car->road_fence_normal.y = wall_normal.y;
	car->road_fence_normal.z = wall_normal.z;

	// 0, 0x10000, 0 => up
	car->road_surface_normal.x = fixmul(heading.y, wall_normal.z) - fixmul(heading.z, wall_normal.y);
	car->road_surface_normal.y = fixmul(heading.z, wall_normal.x) - fixmul(heading.x, wall_normal.z);
	car->road_surface_normal.z = fixmul(heading.x, wall_normal.y) - fixmul(heading.y, wall_normal.x);

	// 0, 0, 0x10000 => north
	car->road_heading.x = heading.x;
	car->road_heading.y = heading.y;
	car->road_heading.z = heading.z;

	// unknown purpose
	if ((car->car_id < 0) || (car->car_id >= g_number_of_players)) {
		iVar9 = DAT_800eb6a4;
		if ((g_number_of_players <= car->car_id) && (car->car_id < g_racer_cars_in_scene)) {
			iVar9 = g_ai_opp_data[car->car_id].field_0x89;
		}
		DAT_001039d8 = DAT_001039d4 * DAT_001039dc;
		DAT_001039d4 = DAT_001039d8 & 0xffff;
		if (iVar9 * ((DAT_001039d8 & 0xffff00) >> 8) >> 0x10 == 1) {
			tnfs_ai_get_lane_slack(car);
		}
		// also in PSX
		if (((g_game_settings & 2) != 0) && (DAT_8010d310 == 0)) {
			DAT_001039d8 = DAT_001039d4 * DAT_001039dc;
			uVar5 = DAT_001039d4 & 0xffff00;
			DAT_001039d4 = DAT_001039d4 & 0xffff;
			if ((uVar5 >> 8) * 0x19 >> 0xe < 0xd) {
				car->car_road_speed = (car->car_road_speed * 0x5d) / 100;
			}
		}
	}
}


int tnfs_car_road_speed(tnfs_car_data *car) {
	return (-car->speed_x >> 8) * ((car->road_heading).x >> 8) //
			+ ((car->road_heading).y >> 8) * (car->speed_y >> 8) //
			+ ((car->road_heading).z >> 8) * (car->speed_z >> 8);
}

int tnfs_car_road_speed_2(tnfs_car_data *car) {
	return ((car->collision_data.speed).x >> 8) * ((car->road_heading).x >> 8) //
			+ ((car->road_heading).y >> 8) * ((car->collision_data.speed).y >> 8) //
			+ ((car->road_heading).z >> 8) * (-(car->collision_data.speed).z >> 8);
}

/*
 * setup everything
 */
void tnfs_init_sim(char *trifile) {
	int i, j;

	g_game_time = 200;
	cheat_crashing_cars = 0;
	g_game_settings = 0;
	sound_flag = 0;

	tnfs_init_track(trifile);

	// AI skill constants
	g_ai_skill_cfg.opp_desired_ahead = 0x140000;
	g_ai_skill_cfg.cop_warning_time = 400;
	g_ai_skill_cfg.max_player_runways = 2;
	g_ai_skill_cfg.traffic_density = 0x1f4;
	g_ai_skill_cfg.number_of_traffic_cars = 4;
	g_ai_skill_cfg.traffic_speed_factors[0] = 0x10000;
	g_ai_skill_cfg.traffic_speed_factors[1] = 0xe665;
	g_ai_skill_cfg.traffic_speed_factors[2] = 0xb333;
	g_ai_skill_cfg.traffic_speed_factors[3] = 0x9999;
	g_ai_skill_cfg.opp_desired_speed_c = math_mul(0x960000, 0x471c); //0x29aa68;
	g_ai_skill_cfg.lane_slack[0] = 0x10000;
	g_ai_skill_cfg.lane_slack[1] = -0x10000;
	g_ai_skill_cfg.lane_slack[2] = 0x8000;
	g_ai_skill_cfg.lane_slack[3] = 0xCCCC;

	g_ai_skill_cfg.opponent_glue_0[0] = 0x20000;
	g_ai_skill_cfg.opponent_glue_0[1] = 0x20000;
	g_ai_skill_cfg.opponent_glue_0[2] = 0x20000;
	g_ai_skill_cfg.opponent_glue_0[3] = 0x20000;
	g_ai_skill_cfg.opponent_glue_0[4] = 0x1cccc;
	g_ai_skill_cfg.opponent_glue_0[5] = 0x1cccc;
	g_ai_skill_cfg.opponent_glue_0[6] = 0x19999;
	g_ai_skill_cfg.opponent_glue_0[7] = 0x16666;
	g_ai_skill_cfg.opponent_glue_0[8] = 0x13333;
	g_ai_skill_cfg.opponent_glue_0[9] = 0x11999;
	g_ai_skill_cfg.opponent_glue_0[10] = 0x10000;
	g_ai_skill_cfg.opponent_glue_0[11] = 0xf851;
	g_ai_skill_cfg.opponent_glue_0[12] = 0xeb85;
	g_ai_skill_cfg.opponent_glue_0[13] = 0xe666;
	g_ai_skill_cfg.opponent_glue_0[14] = 0xd999;
	g_ai_skill_cfg.opponent_glue_0[15] = 0xcccc;
	g_ai_skill_cfg.opponent_glue_0[16] = 0xc000;
	g_ai_skill_cfg.opponent_glue_0[17] = 0xb333;
	g_ai_skill_cfg.opponent_glue_0[18] = 0xa666;
	g_ai_skill_cfg.opponent_glue_0[19] = 0xa666;
	g_ai_skill_cfg.opponent_glue_0[20] = 0x9999;

	g_ai_skill_cfg.opponent_glue_1[0] = 0x20000;
	g_ai_skill_cfg.opponent_glue_1[1] = 0x20000;
	g_ai_skill_cfg.opponent_glue_1[2] = 0x20000;
	g_ai_skill_cfg.opponent_glue_1[3] = 0x20000;
	g_ai_skill_cfg.opponent_glue_1[4] = 0x1cccc;
	g_ai_skill_cfg.opponent_glue_1[5] = 0x16666;
	g_ai_skill_cfg.opponent_glue_1[6] = 0x14ccc;
	g_ai_skill_cfg.opponent_glue_1[7] = 0x13333;
	g_ai_skill_cfg.opponent_glue_1[8] = 0x11999;
	g_ai_skill_cfg.opponent_glue_1[9] = 0x10000;
	g_ai_skill_cfg.opponent_glue_1[10] = 0x10000;
	g_ai_skill_cfg.opponent_glue_1[11] = 0x10000;
	g_ai_skill_cfg.opponent_glue_1[12] = 0xe666;
	g_ai_skill_cfg.opponent_glue_1[13] = 0xe666;
	g_ai_skill_cfg.opponent_glue_1[14] = 0xd999;
	g_ai_skill_cfg.opponent_glue_1[15] = 0xcccc;
	g_ai_skill_cfg.opponent_glue_1[16] = 0xc000;
	g_ai_skill_cfg.opponent_glue_1[17] = 0xc000;
	g_ai_skill_cfg.opponent_glue_1[18] = 0xb333;
	g_ai_skill_cfg.opponent_glue_1[19] = 0xb333;
	g_ai_skill_cfg.opponent_glue_1[20] = 0x9999;

	g_ai_skill_cfg.opponent_glue_2[0] = 0x90000;
	g_ai_skill_cfg.opponent_glue_2[1] = 0x30000;
	g_ai_skill_cfg.opponent_glue_2[2] = 0x20000;
	g_ai_skill_cfg.opponent_glue_2[3] = 0x20000;
	g_ai_skill_cfg.opponent_glue_2[4] = 0x18000;
	g_ai_skill_cfg.opponent_glue_2[5] = 0x18000;
	g_ai_skill_cfg.opponent_glue_2[6] = 0x18000;
	g_ai_skill_cfg.opponent_glue_2[7] = 0x18000;
	g_ai_skill_cfg.opponent_glue_2[8] = 0x13333;
	g_ai_skill_cfg.opponent_glue_2[9] = 0x10000;
	g_ai_skill_cfg.opponent_glue_2[10] = 0x10000;
	g_ai_skill_cfg.opponent_glue_2[11] = 0xcccc;
	g_ai_skill_cfg.opponent_glue_2[12] = 0xcccc;
	g_ai_skill_cfg.opponent_glue_2[13] = 0x10000;
	g_ai_skill_cfg.opponent_glue_2[14] = 0x20000;
	g_ai_skill_cfg.opponent_glue_2[15] = 0x60000;
	g_ai_skill_cfg.opponent_glue_2[16] = 0x90000;
	g_ai_skill_cfg.opponent_glue_2[17] = 0x90000;
	g_ai_skill_cfg.opponent_glue_2[18] = 0x90000;
	g_ai_skill_cfg.opponent_glue_2[19] = 0x90000;
	g_ai_skill_cfg.opponent_glue_2[20] = 0x90000;

	g_ai_skill_cfg.opponent_glue_3[0] = 0x90000;
	g_ai_skill_cfg.opponent_glue_3[1] = 0x30000;
	g_ai_skill_cfg.opponent_glue_3[2] = 0x20000;
	g_ai_skill_cfg.opponent_glue_3[3] = 0x20000;
	g_ai_skill_cfg.opponent_glue_3[4] = 0x10000;
	g_ai_skill_cfg.opponent_glue_3[5] = 0x10000;
	g_ai_skill_cfg.opponent_glue_3[6] = 0x10000;
	g_ai_skill_cfg.opponent_glue_3[7] = 0x10000;
	g_ai_skill_cfg.opponent_glue_3[8] = 0x10000;
	g_ai_skill_cfg.opponent_glue_3[9] = 0x10000;
	g_ai_skill_cfg.opponent_glue_3[10] = 0x10000;
	g_ai_skill_cfg.opponent_glue_3[11] = 0x10000;
	g_ai_skill_cfg.opponent_glue_3[12] = 0x10000;
	g_ai_skill_cfg.opponent_glue_3[13] = 0x90000;
	g_ai_skill_cfg.opponent_glue_3[14] = 0x90000;
	g_ai_skill_cfg.opponent_glue_3[15] = 0x90000;
	g_ai_skill_cfg.opponent_glue_3[16] = 0x90000;
	g_ai_skill_cfg.opponent_glue_3[17] = 0x90000;
	g_ai_skill_cfg.opponent_glue_3[18] = 0x90000;
	g_ai_skill_cfg.opponent_glue_3[19] = 0x90000;
	g_ai_skill_cfg.opponent_glue_3[20] = 0x90000;

	for (i = 0; i < 8; i++) {
		g_ai_opp_data[i].id = i;
		for (j = 0; j < 21; j++) {
			g_ai_opp_data[i].opponent_glue_factors[j] = 0x10000;
		}
		g_ai_opp_data[i].field_0x55 = 0x10000; //???
		g_ai_opp_data[i].field_0x59 = 0x10000;
		g_ai_opp_data[i].opp_block_look_behind = 0xf;
		g_ai_opp_data[i].opp_block_behind_distance = 0x50000; //
		g_ai_opp_data[i].opp_lane_change_speeds = 0x3cccc; //3.8
		g_ai_opp_data[i].field_0x69 = 0x4ccc; //0.3
		g_ai_opp_data[i].opp_oncoming_look_ahead = 0x1b;
		g_ai_opp_data[i].opp_oncoming_corner_swerve = 0;
		g_ai_opp_data[i].opp_cut_corners = 0x1b;
		for (j = 0; j < 4; j++) {
			g_ai_opp_data[i].lane_slack[i] = 0;
		}
		g_ai_opp_data[i].field_0x89 = 0x3333; //0.2

		g_stats_data[i].best_accel_time_1 = 99999;
		g_stats_data[i].best_accel_time_2 = 99999;
		g_stats_data[i].best_brake_time_2 = 999;
		g_stats_data[i].best_brake_time_1 = 999;
		g_stats_data[i].quarter_mile_speed = 0;
		g_stats_data[i].quarter_mile_time = 99999;
		g_stats_data[i].penalty_count = 0;
		g_stats_data[i].warning_count = 0;
		g_stats_data[i].field_0x1b8 = 0;
		g_stats_data[i].prev_lap_time = 0;
		g_stats_data[i].field412_0x1c0 = 0;
		g_stats_data[i].top_speed = 0;
		g_stats_data[i].field_0x1c8 = 0;
		for (j = 0; j < 17; j++) {
			g_stats_data[j].lap_timer[j] = 0;
		}
	}

	// init player car
	tnfs_init_car(&g_car_array[0]);
	g_car_array[0].car_data_ptr = &g_car_array[0];
	g_car_array[0].car_specs_ptr = &car_specs;
	g_car_array[0].road_segment_a = 18;
	tnfs_reset_car(&g_car_array[0]);
	g_car_ptr_array[0] = &g_car_array[0];
	player_car_ptr = &g_car_array[0];

	// create AI car(s)
	tnfs_ai_init();
}

/*
 * minimal basic main loop
 */
void tnfs_update() {
	int i;
	tnfs_car_data *car;
	int reset;

	g_game_time++;

	// update camera
	switch (selected_camera) {
	case 0: //chase cam
		camera.car_id = 0;
		camera.position.x = g_car_array[0].position.x;
		camera.position.y = g_car_array[0].position.y + 0x50000;
		camera.position.z = g_car_array[0].position.z - 0x96000;
		break;
	case 1: //heli cam
		camera.car_id = 0;
		camera.position.x = g_car_array[0].position.x;
		camera.position.y = g_car_array[0].position.y + 0x60000;
		camera.position.z = g_car_array[0].position.z - 0x100000;
		break;
	case 2: //opponent cam
		camera.car_id = 1;
		camera.position.x = g_car_array[1].position.x;
		camera.position.y = g_car_array[1].position.y + 0x60000;
		camera.position.z = g_car_array[1].position.z - 0x100000;
		break;
	case 3: //cop cam
		camera.car_id = g_racer_cars_in_scene;
		camera.position.x = g_car_array[camera.car_id].position.x;
		camera.position.y = g_car_array[camera.car_id].position.y + 0x60000;
		camera.position.z = g_car_array[camera.car_id].position.z - 0x100000;
		break;
	}

	player_car_ptr->car_road_speed = tnfs_car_road_speed(player_car_ptr);

	tnfs_controls_update();
	tnfs_ai_collision_handler();

	// for each car
	for (i = 0; i < g_total_cars_in_scene; i++) {
		car = g_car_ptr_array[i];

		if (car->crash_state != 4) {
			if (i < g_number_of_players) {
				tnfs_driving_main(car);
				math_matrix_from_pitch_yaw_roll(&car->matrix, car->angle_x + car->body_pitch, car->angle_y, car->angle_z + car->body_roll);
			} else {
				tnfs_ai_driving_main(car);
			}
		} else {
			tnfs_collision_main(car);
		}

		// tweak to allow circuit track lap
		reset = 0;
		if ((car->road_segment_a > road_segment_count - 2) && (car->car_road_speed > 0x1000)) {
			car->road_segment_a = 0;
			car->road_segment_b = 0;
			reset = 1;
		} else if ((car->road_segment_a == 0) && (car->car_road_speed < -0x1000)) {
			car->road_segment_a = car->road_segment_b = road_segment_count - 2;
			reset = 1;
		}
		if (reset //
			&& ((abs(car->position.x - track_data[car->road_segment_a].pos.x) > 0x1000000)
			|| (abs(car->position.z - track_data[car->road_segment_a].pos.z) > 0x1000000))) {
			tnfs_reset_car(car);
		}

		// set ground point for the collision engine
		car->road_ground_position = track_data[car->road_segment_a].pos;
	}
}
