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
#include "tnfs_camera.h"

const int g_gravity_const = 0x9CF5C;

tnfs_track_data track_data[2400];
tnfs_surface_type road_surface_type_array[3];
tnfs_track_speed g_track_speed[600];

tnfs_car_specs car_specs;
tnfs_car_data g_car_array[8];
tnfs_car_data *g_car_ptr_array[8]; // 00153ba0/00153bec 8010c720/800f7e60
tnfs_car_data *player_car_ptr;
tnfs_car_data *g_cop_car_ptr = 0; // (DOS DAT_001651a8 is g_ai_skill_cfg.number_of_traffic_cars, not this)
tnfs_ai_skill_cfg g_ai_skill_cfg;
tnfs_ai_opp_data g_ai_opp_data[8];
tnfs_stats_data g_stats_data[8];
char * g_pbs_file = 0;

int g_total_cars_in_scene = 7;
int g_racer_cars_in_scene = 2; // (including player) 001670AB DAT_8010d1c8
int g_number_of_players = 1; //001670af 8010d1cc
int g_number_of_cops = 1; //001670B3 8010d1d0
int g_number_of_traffic_cars = 4; //001670BB

// settings/flags
char is_drifting;
int iSimTimeClock = 0;
int g_road_node_count = 0;
int g_tri_num_chunks = 0;
tnfs_road_object g_road_objects[1000];
int g_road_object_count = 0;
char g_sign_status[2000];
int g_road_finish_node = 0;
int sound_flag = 0;
int g_selected_cheat = 0;
int cheat_crashing_cars = 0;
int g_game_settings = 0;
char g_control_throttle;
char g_control_brake;
signed char g_control_steer;
char g_control_gear; // shift button for the next tick: 0 none, 1 down, 2 up
// analog controls (Win95 0x4fb724/0x4fb728 per player: steering and pedal device, 1 joystick); axes as Win95 0x4fce54/0x4fce58
char g_control_analog_steering; // 1: tnfs_control_steering_b
char g_control_analog_pedals; // 1: tnfs_control_pedals_analog
signed char g_control_steer_axis; // -0x20 left .. 0x1f right
signed char g_control_pedal_axis; // -0x1e full throttle .. 0x1e full brake
char g_control_handbrake; // handbrake key (control word bit 15)
char g_control_horn; // horn key (control word bit 14)
// control word per player (DOS 0x1458c4, SE 0x4fb704), latched once per control step, see tnfs_control_word_get
int g_control_word[8];
int g_police_on_chase = 0; //000fdb90
int g_police_speeding_ticket = 0; //0016513C
int g_police_chase_time = 0; //0016533c
int g_police_ticket_time = 0;

tnfs_camera camera;
int selected_camera = 0;

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

int g_collision_bump_ref = 0x6666; //DAT_000f99f0 800eae58
int g_collision_force_carcar; // f9a70 800dae58
int g_collision_force_wall; // f9a74;
int DAT_000F9BB0 = 0;
int DAT_000f99e4 = 0x10000;
int DAT_000f99e8 = 0x34000;
int DAT_000f99ec = 10; //800eae14
int DAT_000f99f0 = 0x8000;
int DAT_000fae60 = 1;
int DAT_000FDB94 = 0;
int DAT_000FDCEC = -1;
int DAT_000FDCF0 = -1;
int DAT_000f9A70 = 0;
int g_lcg_random_mod = 0xFFFF; //800db6bc
int g_lcg_random_nbr = 0x12345678; //800db6c0 g_car_random_index
int g_lcg_random_seed = 0x12345679; // random value
int g_camera_node = 0; //144914
int g_is_closed_track = 0; // 14DCC4
int g_slice_mask = 0xFFFF; // 14dccc
int g_track_laps = 4; //00153b08
int g_track_slice = 0x76b; //00153b0c
int DAT_00153B20 = 0; // game over flag
int DAT_00153B24 = 0; // game over flag 2?
tnfs_car_data * DAT_00153BC4 = 0; //player car ptr 2
int DAT_00165148 = 1; // center lane distance/margin
int g_player_id = 0; //16707C
int g_cam_change_delay = 0; // 00143844

int g_race_positions[8] = { 7, 6, 5, 4, 3, 2, 1, 0 }; // 00167179

// analog steering target by |input| (DOS 0xf9ef0, Win95 0x4baff0, PSX 0x800bd518), 0..32 are read
int g_steering_analog_table[36] = { //
		0x3333, 0x6666, 0xb333, 0x10000, 0x16666, 0x1cccc, 0x24ccc, 0x2cccc, 0x36666, 0x40000, 0x4cccc, 0x5b333, //
		0x69999, 0x78000, 0x88000, 0x98000, 0xa8000, 0xb8000, 0xc8000, 0xd8000, 0xe8000, 0xf8000, 0x108000, 0x118000, //
		0x128000, 0x138000, 0x14b333, 0x15e666, 0x173333, 0x184ccc, 0x199999, 0x1ae666, 0x1c8000, 0x1e0000, 0x1f0000, 0x200000 };

/* create a random track and a generic car */

void auto_generate_track() {
	int pos_x = 0;
	int pos_y = 0;
	int pos_z = 0;
	int slope = 0;
	int slant = 0;
	int heading = 0;
	int rnd = 0;
	int i;

	g_road_node_count = 2395;
	g_tri_num_chunks = 598;
	g_road_finish_node = g_road_node_count - 0xb5;

	for (i = 0; i < 2400; i++) {

		if (i > 20 && i % 20 == 0) {
			g_lcg_random_nbr = g_lcg_random_mod * g_lcg_random_seed;
			g_lcg_random_mod = g_lcg_random_nbr & 0xffff;
			rnd = g_lcg_random_nbr;
		}

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
		if (heading > 0xFFF) slant *= 0.9;
		if (heading < -0xFFF) slant *= 0.9;

		track_data[i].roadLeftFence = 0x50;
		track_data[i].roadRightFence = 0x50;
		track_data[i].roadLeftMargin = 0x25;
		track_data[i].roadRightMargin = 0x25;
		track_data[i].num_lanes = 0x11;
		track_data[i].fence_flag = 0;
		track_data[i].shoulder_surface_type = 0x22;
		track_data[i].item_mode = 0x3;

		track_data[i].slope = slope;
		track_data[i].heading = heading;
		track_data[i].slant = slant;
		track_data[i].pos.x = pos_x;
		track_data[i].pos.y = pos_y;
		track_data[i].pos.z = pos_z;

		track_data[i].side_normal_x = (short)(math_cos_3(track_data[i].heading * -0x400) / 2);
		track_data[i].side_normal_y = (short)(math_tan_3(track_data[i].slant * -0x400) / 2);
		track_data[i].side_normal_z = (short)(math_sin_3(track_data[i].heading * -0x400) / 2);

		// next node
		pos_x += fixmul(math_sin_3(track_data[i].heading * 0x400), 0x80000);
		pos_y += fixmul(math_tan_3(track_data[i].slope * 0x400), 0x80000);
		pos_z += fixmul(math_cos_3(track_data[i].heading * 0x400), 0x80000);
		heading += slant >> 3;
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
	for (i = 0; i < g_road_node_count; i++) {
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
	car_specs.wheelbase_inv = 0x68EB; //0.4
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
	car_specs.maxAutoSteerAngle = 983; // DVIPER.PBS steering and pedal values
	car_specs.autoRampMultShift = 1;
	car_specs.autoRampDivShift = 1;
	car_specs.steerModel = 2;
	car_specs.vel1_AS2 = 2;
	car_specs.vel2_AS2 = 4;
	car_specs.vel3_AS2 = 8;
	car_specs.vel4_AS2 = 16;
	car_specs.velRamp_AS2 = 0x23333; //2.2
	car_specs.velAttenuate_AS2 = 0xa3d; //0.04
	car_specs.autoRampMultShift_AS2 = 1;
	car_specs.autoRampDivShift_AS2 = 1;
	car_specs.lateral_accel_cutoff = 0x158000;
	car_specs.field_0x320 = 0x280;
	car_specs.field_0x324 = 0x66;
	car_specs.final_drive_torque_ratio = 0x280;
	car_specs.thrust_to_acc_factor = 0x66;
	car_specs.abs_equipped = 0;
	car_specs.tcs_equipped = 0;
	car_specs.throttle_on_ramp = 25;
	car_specs.throttle_off_ramp = 17;
	car_specs.brake_on_ramp_1 = 12;
	car_specs.brake_on_ramp_2 = 2;
	car_specs.brake_off_ramp_1 = 51;
	car_specs.brake_off_ramp_2 = 51;
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
	car->road_object_slice = 0;
	car->road_object_index = 0;
	car->is_crashed = 0;
	car->is_wrecked = 0;
	car->time_off_ground = 0;
	car->slide_front = 0;
	car->slide_rear = 0;
	car->wheels_on_ground = 1;
	car->surface_type = 0;
	car->surface_type_b = 0;
	car->in_tunnel = 0;
	car->in_tunnel_or_8 = 0;
	car->on_cobbles = 0;
	car->waterfall_left = 0;
	car->waterfall_right = 0;
	car->slope_force_lat = 0;
	car->unknown_flag_3DD = 0;
	car->slope_force_lon = 0;

	car->position.x = track_data[car->track_slice].pos.x;
	car->position.y = track_data[car->track_slice].pos.y + 150;
	car->position.z = track_data[car->track_slice].pos.z;

	car->angle.x = track_data[car->track_slice].slope * 0x400;
	car->angle.y = track_data[car->track_slice].heading * 0x400;
	car->angle.z = track_data[car->track_slice].slant * 0x400;

	// convert slope/slant angles to signed values
	if (car->angle.x > 0x800000)
		car->angle.x -= 0x1000000;
	if (car->angle.z > 0x800000)
		car->angle.z -= 0x1000000;

	car->angle.x *= -1;
	car->angle.z *= -1;

	if (car->ai_state & 0x1000) {
		car->angle.y *= -1;
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

	car->track_center_distance = 0;

	math_matrix_identity(&car->matrix);
	math_matrix_identity(&car->collision_data.matrix);

	car->collision_data.position.x = car->position.x;
	car->collision_data.position.y = car->position.y;
	car->collision_data.position.z = -car->position.z;
	car->collision_data.speed.x = 0;
	car->collision_data.speed.y = 0;
	car->collision_data.speed.z = 0;
	car->collision_data.gravity_vector.x = 0;
	car->collision_data.gravity_vector.y = -0x9cf5c;
	car->collision_data.gravity_vector.z = 0;
	car->collision_data.state_timer = 0;
	car->collision_data.angular_speed.x = 0;
	car->collision_data.angular_speed.y = 0;
	car->collision_data.angular_speed.z = 0;

	// ai car flags
	car->speed_target = 0;
	car->target_angle = 0;
	car->collision_data.field_084 = 0;
	car->collision_data.field_088 = 0;
	car->collision_data.field_08c = 0;
	car->collision_data.traffic_speed_factor = 0x10000;
	car->lane_slack = 0;
	car->crash_state = 3;
	car->field_461 = 0;
	car->field_4e9 = 7;
	car->field_158 = 0;

	car->field_4c5 = 0;
	car->field_4c9 = 0;
	car->field_4cd = 0;
	car->field_4d1 = 0;
	car->field_4d3 = 0;

	if (car == player_car_ptr) {
		// player car
		car->car_model_id = 5; //DVIPER
		car->crash_state = 2;
		car->ai_state = 0x1e0;
		car->field_158 = 1;
		g_police_on_chase = 0;
		g_police_speeding_ticket = 0;
		g_police_ticket_time = 0;
		g_police_chase_time = 0;
	} else {
		// ai cars
		car->crash_state = 3;
		if (car->car_id == g_racer_cars_in_scene) {
			// police car
			car->ai_state = 0x1e8;
			g_police_on_chase = 0;
			printf("respawn cop car\n");
		}
	}
}

void tnfs_Fiziks_InitCar(tnfs_car_data *car) {
	int aux;

	tnfs_init_surface_constants();

	// weight distribution
	car->weight_distribution_front = math_mul(car->car_specs_ptr->mass_front, car->car_specs_ptr->inverse_mass);
	car->weight_distribution_rear = math_mul(car->car_specs_ptr->mass_rear, car->car_specs_ptr->inverse_mass);
	// more precise way of doing it:
	//car->weight_distribution_front = math_div(car->car_specs_ptr->mass_front, car->car_specs_ptr->mass_total);
	//car->weight_distribution_rear = 0x10000 - car->weight_distribution_front;

	// unused specs
	car->mass_front = math_mul(car->car_specs_ptr->mass_total, car->car_specs_ptr->inverse_mass_front);
	car->mass_rear = math_mul(car->car_specs_ptr->mass_total, car->car_specs_ptr->inverse_mass_rear);

	// drag coefficient to deccel factor
	car->car_specs_ptr->drag = math_mul(car->car_specs_ptr->drag, car->car_specs_ptr->inverse_mass);

	car->weight_transfer_factor = math_mul(car->car_specs_ptr->centre_of_gravity_height, car->car_specs_ptr->wheelbase_inv);
	car->front_friction_factor = math_mul(car->car_specs_ptr->front_friction_factor, math_mul(car->weight_distribution_front, g_gravity_const));
	car->rear_friction_factor = math_mul(car->car_specs_ptr->rear_friction_factor, math_mul(car->weight_distribution_rear, g_gravity_const));

	car->drag_const_0x4ac = (short) (car->drag_const_0x4a8 << 1);
	car->tire_grip_front = car->front_friction_factor;
	car->tire_grip_rear = car->rear_friction_factor;
	car->drag_const_0x4ae = (short) (car->drag_const_0x4aa << 1);

	car->drag_const_0x4a8 = fix8(fix8(car->car_specs_ptr->front_friction_factor) * 10 * car->drag_const_0x4a8);
	car->drag_const_0x4aa = fix8(fix8(car->car_specs_ptr->rear_friction_factor) * 10 * car->drag_const_0x4aa);

	aux = math_mul(math_mul(car->car_specs_ptr->wheelbase, car->car_specs_ptr->wheelbase), 0x324);

	car->wheel_base = math_div(aux, car->car_specs_ptr->wheelbase);
	car->moment_of_inertia = math_div(math_mul(aux, car->car_specs_ptr->inertia_factor), car->car_specs_ptr->wheelbase);
	car->front_yaw_factor = math_div(math_mul(car->weight_distribution_rear, car->car_specs_ptr->wheelbase), aux);
	car->rear_yaw_factor = math_div(math_mul(car->weight_distribution_front, car->car_specs_ptr->wheelbase), aux);

	//collision body specs
	car->collision_height_offset = 0x92f1;
	car->collision_data.mass = 0x10a1c;
	car->collision_data.moment_of_inertia = 0x10000;
	car->collision_data.linear_acc_factor = 0xf646;
	car->collision_data.angular_acc_factor = 0x7dd4;
	car->collision_data.size.x = car->car_specs_ptr->body_width / 2;
	car->collision_data.size.y = 0x92f1;
	car->collision_data.size.z = car->car_specs_ptr->body_length / 2;
	car->collision_data.edge_length = math_vec3_length(&car->collision_data.size);
}

void tnfs_init_car() {
	int i;

	tnfs_car_data *car = &g_car_array[0];
	g_car_array[0].car_data_ptr = &g_car_array[0];
	g_car_array[0].car_specs_ptr = &car_specs;
	g_car_array[0].track_slice = 0x10;
	player_car_ptr = &g_car_array[0];
	g_car_ptr_array[0] = &g_car_array[0];
	g_car_array[0].field_4e9 = 7;
	tnfs_initial_position(&g_car_array[0]);

	// load car specs
	if (g_pbs_file == 0 || !read_pbs_file(g_pbs_file)) {
		tnfs_create_car_specs();
	}

	car->crash_state = 2;
	car->car_id = 0;
	car->field_4e9 = 7;
	car->position.z = 0; //0x600000;
	car->track_slice = 0x10;
	car->track_slice_lap = 0x10;
	car->lap_number = 1;

	// rally mode tweaks, Fiziks_PreInitCar (PC version: Win95 0x4400f4; DOS 0x63e72 only on tracks 1, 2, 3 and 5)
	// PSX version (0x80038ea4) instead: lateral_accel_cutoff x 0.625, centre_of_gravity_height x 7/4,
	// efficiency x a per track/segment table (max 0x19000), front_drive_percentage 0x8000
	if (g_game_settings & 0x20) {
		for (i = 0; i < 512; i++)
			car_specs.grip_table[i + 512] = car_specs.grip_table[i];

		car_specs.field_0x320 = car_specs.field_0x320 / 2;
		car_specs.field_0x324 = 0x10000 / car_specs.field_0x320;
		car_specs.final_drive_torque_ratio = car_specs.final_drive_torque_ratio / 2;
		car_specs.thrust_to_acc_factor = 0x10000 / car_specs.final_drive_torque_ratio;
		car_specs.front_friction_factor = car_specs.front_friction_factor / 2;
		car_specs.rear_friction_factor = car_specs.rear_friction_factor / 2;
		car_specs.max_brake_force_1 = car_specs.max_brake_force_1 / 2;
		car_specs.max_brake_force_2 = car_specs.max_brake_force_2 / 2;
		car_specs.centre_of_gravity_height = car_specs.centre_of_gravity_height * 3 / 2;
		if (selected_track == 3) {
			car_specs.efficiency = fix8(car_specs.efficiency * 0x14c);
		} else {
			car_specs.efficiency = fix8(car_specs.efficiency * 0x133);
		}
	}

	// net wheel torque values; tnfs_reset_car starts the car with the automatic gearbox
	tnfs_load_torque_table(&car_specs, 1);

	car->car_length = car_specs.body_length;
	car->car_width = car_specs.body_width;
	// the front-end ABS/TCS options are taken as on
	car->abs_enabled = car_specs.abs_equipped > 0;
	car->tcs_enabled = car_specs.tcs_equipped > 0;
	car->gear_auto_selected = 0;
	car->drag_const_0x4a8 = 0;
	car->drag_const_0x4aa = 0;

	tnfs_Fiziks_InitCar(car);

	tnfs_reset_car(car);
}

/* basic game controls */

/*
 * steering with digital controls (Win95 0x438954 == PSX 0x800334a4; DOS 0x5e050 has no ">> autoRampDivShift_AS2" branch)
 */
void tnfs_control_steering_a(tnfs_car_data *car, int input, int auto_steer) {
	tnfs_car_specs *specs;
	int target;
	int rate;
	int attenuation;
	int diff;

	specs = car->car_specs_ptr;

	input = (input & 0x3f) - 0x20;
	if (input > 0) {
		input++;
	}
	if (input >= 0x10) {
		target = 0x1b0000;
	} else if (input < -0xf) {
		target = -0x1b0000;
	} else {
		target = 0;
	}

	// auto steer, when centred and moving forward
	if (car->speed_local_lon >= 0 && target == 0) {
		if (abs(auto_steer) < (car->speed_local_lon >> 0x10) * specs->maxAutoSteerAngle) {
			target = auto_steer;
		}
	}

	attenuation = (abs(car->speed_local_lon) >> 0x10) * specs->velAttenuate_AS2;
	if (attenuation > 0x18000) {
		attenuation = 0x18000;
	}

	// PSX version, rally mode
	if ((g_game_settings & 0x20) != 0) {
		attenuation = 0;
		auto_steer = target;
	}

	rate = specs->velRamp_AS2 - attenuation;
	if (rate > 0x19999) {
		rate = 0x19999;
	}
	rate *= specs->vel2_AS2;

	if (car->speed_local_lon > 0 && abs(auto_steer) < 0x400000) {
		if ((auto_steer - car->steer_angle > 0 && target - car->steer_angle > 0) //
				|| (auto_steer - car->steer_angle < 0 && target - car->steer_angle < 0)) {
			rate = rate << specs->autoRampMultShift_AS2;
			if (target == 0) {
				rate = rate << 1;
			}
		} else {
			rate = rate >> specs->autoRampDivShift_AS2;
			if (target == 0) {
				rate = rate / 2;
			}
		}
	}

	diff = abs(car->steer_angle - target);
	if (diff < rate) {
		rate = diff;
	}
	if (car->steer_angle < target) {
		car->steer_angle += rate;
	} else if (car->steer_angle > target) {
		car->steer_angle -= rate;
	} else {
		car->steer_angle = target;
	}
}

/*
 * steering with analog controls (DOS 0x5e27f == Win95 0x438aac)
 * PSX 0x8003367c: rate caps 0xa0000/0x140000, attenuation >> 8, rally mode sets the steer angle to the target
 */
void tnfs_control_steering_b(tnfs_car_data *car, int input, int auto_steer) {
	int target;
	int attenuation;
	int diff;
	int auto_diff;
	int rate;

	input = (input & 0x3f) - 0x20;
	if (input > 0) {
		input++;
	}
	if (input < 0) {
		target = -g_steering_analog_table[-input];
	} else {
		target = g_steering_analog_table[input];
	}

	if (car->speed_local_lon < 0) {
		auto_steer = 0;
	}
	if (car->speed_local_lon <= 0x120000) {
		attenuation = 0;
	} else {
		attenuation = fix7(car->speed_local_lon - 0x120000);
	}

	diff = target - car->steer_angle;
	auto_diff = auto_steer - car->steer_angle;
	if (diff > 0 && auto_diff > 0) {
		// towards the auto steer target: no rate cap
		rate = diff;
		if (auto_diff < diff) {
			rate = auto_diff;
		}
	} else if (diff < 0 && auto_diff < 0) {
		rate = diff;
		if (diff < auto_diff) {
			rate = auto_diff;
		}
	} else if (diff <= 0) {
		if (diff < -0xc0000) {
			rate = -(0x40000 - attenuation);
		} else {
			rate = -(0x18000 - attenuation);
		}
		if (rate < diff) {
			rate = diff;
		}
	} else {
		if (diff <= 0xc0000) {
			rate = 0x18000 - attenuation;
		} else {
			rate = 0x40000 - attenuation;
		}
		if (diff < rate) {
			rate = diff;
		}
	}
	car->steer_angle += rate;
}

/*
 * throttle ramp (DOS 0x5e42a, PSX 0x800337f4)
 */
void tnfs_control_throttle(tnfs_car_data *car, int target) {
	if (car->throttle < target) {
		if (car->car_specs_ptr->throttle_on_ramp < target - car->throttle) {
			car->throttle += car->car_specs_ptr->throttle_on_ramp;
			return;
		}
	} else {
		if (car->throttle <= target) {
			return;
		}
		if (car->car_specs_ptr->throttle_off_ramp < car->throttle - target) {
			car->throttle -= car->car_specs_ptr->throttle_off_ramp;
			return;
		}
	}
	car->throttle = target;
}

/*
 * brake ramp (DOS 0x5e4e9, PSX 0x80033864)
 */
void tnfs_control_brake(tnfs_car_data *car, int target) {
	int ramp;
	if (car->brake < target) {
		if (car->brake < 144) {
			ramp = car->car_specs_ptr->brake_on_ramp_1;
		} else {
			ramp = car->car_specs_ptr->brake_on_ramp_2;
		}
		ramp = ramp + fix2(ramp);
		if (ramp < target - car->brake) {
			car->brake += ramp;
			return;
		}
	} else {
		if (car->brake <= target) {
			return;
		}
		if (car->brake < 144) {
			ramp = car->car_specs_ptr->brake_off_ramp_1;
		} else {
			ramp = car->car_specs_ptr->brake_off_ramp_2;
		}
		if (ramp < car->brake - target) {
			car->brake -= ramp;
			return;
		}
	}
	car->brake = target;
}

void tnfs_control_shift_gears(tnfs_car_data *car, int button);

/*
 * analog pedals, one axis: 0x1e centre, lower throttle, higher brake (DOS 0x5eb55 == Win95 0x438ff0, PC version)
 * PSX 0x80033b8c: separate throttle and brake, 3 bits each (x0x25), through tnfs_control_throttle/brake
 * input is the control word: buttons (bits 12-15) as tnfs_control_pedals_digital, pedal axis in bits 6-11
 */
void tnfs_control_pedals_analog(tnfs_car_data *car, int input) {
	int throttle;
	int rate;
	int buttons;

	throttle = 0;
	buttons = input >> 12;
	if (buttons & 2) {
		tnfs_control_shift_gears(car, 1);
	} else if (buttons & 1) {
		tnfs_control_shift_gears(car, 2);
	} else {
		tnfs_control_shift_gears(car, 0);
	}
	car->handbrake = (buttons & 8) != 0;
	car->horn = (buttons & 4) != 0;

	input = ((input >> 6) & 0x3f) - 0x1e;
	if (input > 0) {
		input++;
	}
	if (input < 1) {
		throttle = fix8(input * -0x889);
		car->brake = 0;
	} else {
		car->throttle = 0;
		if (car->abs_enabled == 0) {
			tnfs_control_brake(car, fix8(input * 0x889));
		} else {
			car->brake = fix8(input * 0x889);
		}
	}

	if (car->is_gear_engaged == 0) {
		rate = 0xc;
	} else {
		rate = 0x33;
	}
	if (car->throttle < throttle) {
		if (throttle - car->throttle < 0x4d) {
			car->throttle = throttle;
		} else {
			car->throttle += 0x4c;
		}
	} else if (throttle < car->throttle) {
		if (rate < car->throttle - throttle) {
			car->throttle -= rate;
		} else {
			car->throttle = throttle;
		}
	}
	if (car->throttle > 0xff) {
		car->throttle = 0xff;
	}
	if (car->brake > 0xff) {
		car->brake = 0xff;
	}
}

/*
 * gear shift button, 0 none, 1 down, 2 up (DOS 0x5e62f, PSX 0x80033938)
 */
void tnfs_control_shift_gears(tnfs_car_data *car, int button) {
	tnfs_car_specs *specs;

	specs = car->car_specs_ptr;

	if (iSimTimeClock < 300) {
		car->gear_shift_button = button;
		return;
	}

	if (button != 0) {
		if (car->gear_auto_selected < 1) {
			// manual transmission
			if (car->gear_shift_button != 1 && button == 1 && car->gear_selected > -2) {
				car->gear_selected--;
				car->is_gear_engaged = 0;
				car->is_shifting_gears = specs->shift_timer / 2;
				car->throttle_previous_pos = car->throttle;
				car->is_engine_cutoff = 1;
			} else if (car->gear_shift_button != 2 && button == 2 && car->gear_selected < specs->number_of_gears - 3) {
				car->gear_selected++;
				car->is_gear_engaged = 0;
				if (car->car_model_id == 4 && (iSimTimeClock & 0x31) == 0x10) { // F512TR
					car->is_shifting_gears = specs->shift_timer + 3;
				} else {
					car->is_shifting_gears = specs->shift_timer;
				}
				car->throttle_previous_pos = car->throttle;
				car->is_engine_cutoff = 1;
			}
		} else {
			// automatic transmission: 1 Reverse, 2 Neutral, 3 Drive
			if (car->gear_shift_button != 1 && button == 1 && car->gear_auto_selected > 1) {
				car->gear_auto_selected--;
				if (car->gear_auto_selected == 2) {
					car->gear_selected = -1;
					car->is_gear_engaged = 0;
				} else {
					car->gear_selected = 0;
					car->is_gear_engaged = 1;
					car->is_engine_cutoff = 0;
				}
			} else if (car->gear_shift_button != 2 && button == 2 && car->gear_auto_selected < 3) {
				car->gear_auto_selected++;
				if (car->gear_auto_selected == 2) {
					car->is_gear_engaged = 0;
					car->gear_selected = -1;
				} else {
					car->gear_selected = 0;
					car->is_gear_engaged = 0;
					car->is_shifting_gears = specs->shift_timer + 1;
					car->throttle_previous_pos = car->throttle;
					car->is_engine_cutoff = 1;
				}
			}
		}
	}
	car->gear_shift_button = button;

	// manual transmission shift time
	if (car->gear_auto_selected == 0 && car->is_shifting_gears >= 0) {
		if (car->is_shifting_gears == 0) {
			car->is_engine_cutoff = 0;
			car->throttle = car->throttle_previous_pos;
			if (car->gear_selected != -1) {
				car->is_gear_engaged = 1;
			}
		}
		car->is_shifting_gears--;
	}
}

/*
 * auto steer target: heading to the road 6 slices ahead, at the lane (-2, 0 or +2 m) the car is in
 * (PSX 0x80018a4c, DOS 0x41518)
 */
int tnfs_control_auto_steer_target(tnfs_car_data *car) {
	int heading;
	int side;
	int lane;
	int x;
	int z;
	int angle;
	tnfs_track_data *node;

	node = &track_data[car->track_slice & g_slice_mask];
	heading = node->heading << 2;
	side = math_mul(math_sin_2(heading), node->pos.z - car->position.z) - math_mul(math_cos_2(heading), node->pos.x - car->position.x);
	if (side < -45875) {
		lane = -0x20000;
	} else if (side > 45875) {
		lane = 0x20000;
	} else {
		lane = 0;
	}

	node = &track_data[(car->track_slice + 6) & g_slice_mask];
	heading = node->heading << 2;
	x = node->pos.x + math_mul(math_cos_2(heading), lane);
	z = node->pos.z - math_mul(math_sin_2(heading), lane);

	angle = math_atan2(z - car->position.z, x - car->position.x) - car->angle.y;
	if (angle > 0x800000) {
		angle -= 0x1000000;
	} else if (angle < -0x800000) {
		angle += 0x1000000;
	}
	return angle;
}

/*
 * digital pedals (DOS 0x5e9c9 == Win95 0x438f00, PC version), input is the control word:
 * buttons (bits 12-15): 2 shift down, 1 shift up (down wins), 8 handbrake, 4 horn;
 * one pedal axis (bits 6-11, 0x1e centre; keyboard: throttle 5, brake 0x3c, brake wins when both are held):
 * < -15: brake = 0, throttle ramps to 0xff; > 15: brake ramps to 0xff and throttle to 0;
 * centre: brake capped at 100, the throttle ramps down to 0 first, then the brake.
 * PSX tnfs_control_pedals_digital (0x80033c8c) reads separate throttle and brake bits instead.
 */
void tnfs_control_pedals_digital(tnfs_car_data *car, int input) {
	int buttons;
	int pedal;

	buttons = input >> 12;
	pedal = (input >> 6) & 0x3f;
	if (buttons & 2) {
		tnfs_control_shift_gears(car, 1);
	} else if (buttons & 1) {
		tnfs_control_shift_gears(car, 2);
	} else {
		tnfs_control_shift_gears(car, 0);
	}
	car->handbrake = (buttons & 8) != 0;
	car->horn = (buttons & 4) != 0;

	pedal = pedal - 0x1e;
	if (pedal > 0) {
		pedal++;
	}
	if (pedal < -0xf) {
		car->brake = 0;
		tnfs_control_throttle(car, 0xff);
	} else if (pedal < 0x10) {
		if (car->brake > 100) {
			car->brake = 100;
		}
		if (car->throttle == 0) {
			if (car->brake != 0) {
				tnfs_control_brake(car, 0);
			}
		} else {
			tnfs_control_throttle(car, 0);
		}
	} else {
		tnfs_control_brake(car, 0xff);
		tnfs_control_throttle(car, 0);
	}
}

/*
 * Control word of the local player (Win95 SE tnfs_control_word_get 0x43915c; DOS FUN_0005ed8b is the same without
 * the mirror mode, with right winning over left), from the front end's input state (g_control_*):
 * bits 0-5 steering: keyboard 0 left / 0x20 / 0x3c right (both keys held: 0x20), joystick g_control_steer_axis + 0x20;
 * bits 6-11 pedals: keyboard 0x1e / throttle 5 / brake 0x3c (brake wins), joystick g_control_pedal_axis + 0x1e;
 * bits 12-15 buttons: 1 shift up, 2 shift down (SE: up wins; DOS sets both and the pedal functions let down win), 4 horn,
 * 8 handbrake. Neutral word 0x7a0.
 * The device per control is g_control_analog_steering / g_control_analog_pedals (SE g_control_steer_device /
 * g_control_pedal_device of the player: 0 keyboard, 1 joystick); shifting is always on the keyboard here.
 * Not ported: mouse and joystick-button devices, SE mirror mode (flips the steering), the disconnected network
 * player (0xffff87a0) and SE's B+R+W cheat (both shift bits with the automatic gearbox, turned into a launch
 * boost flag DAT_004faa5c in FUN_004397f4, which is not recorded in the replay).
 * The original reads the keys once per control step (SE timer FUN_0043b970 pushes a word every 2 timer ticks, the
 * recorder pops it), so a shift press (g_control_gear) is consumed here.
 */
int tnfs_control_word_get() {
	int steer;
	int pedal;
	int buttons;

	if (g_control_analog_steering == 0) {
		steer = 0x20;
		if (g_control_steer < 0) {
			steer = 0;
		} else if (g_control_steer > 0) {
			steer = 0x3c;
		}
	} else {
		steer = g_control_steer_axis + 0x20;
	}

	if (g_control_analog_pedals == 0) {
		pedal = 0x1e;
		if (g_control_throttle) {
			pedal = 5;
		}
		if (g_control_brake) {
			pedal = 0x3c;
		}
	} else {
		pedal = g_control_pedal_axis + 0x1e;
	}

	buttons = 0;
	if (g_control_gear == 2) {
		buttons = 1;
	} else if (g_control_gear == 1) {
		buttons = 2;
	}
	g_control_gear = 0;
	if (g_control_handbrake) {
		buttons |= 8;
	}
	if (g_control_horn) {
		buttons |= 4;
	}
	return buttons << 12 | steer | pedal << 6;
}

/*
 * player controls (DOS 0x41b49, Win95 0x401400, PSX 0x80018f78), the control handler of the player cars (DOS handler
 * table 0x153b60 [1], car +0x4dd = 1; AI cars have the empty handler 2, DOS 0x7bc76). The game loop calls it on control
 * steps only (general_flags & 4: every second simulation tick, 15 per second) with the latched control word.
 * The device decides the steering and pedal functions (DOS/SE per player, here g_control_analog_* for every player).
 */
void tnfs_controls_update(tnfs_car_data *car) {
	int auto_steer;
	int word;

	word = g_control_word[car->car_id2];

	auto_steer = tnfs_control_auto_steer_target(car);
	// PC version (DOS 0x41b7c, Win95 0x401412), PSX version: 0x400000
	if (abs(auto_steer) > 0x40) {
		auto_steer = 0;
	}
	if (!g_control_analog_steering) {
		tnfs_control_steering_a(car, word, auto_steer);
	} else {
		tnfs_control_steering_b(car, word, auto_steer);
	}

	if (g_control_analog_pedals) {
		tnfs_control_pedals_analog(car, word);
	} else {
		tnfs_control_pedals_digital(car, word);
	}

	//checkpoint flick
	if (tnfs_racer_crossed_finish_line(car)) {
		tnfs_driving_checkpoint_flick(car);
	} else {
		car->field_4cd = 0;
	}
}

void tnfs_change_camera() {
	selected_camera++;
	if (selected_camera > 2)
		selected_camera = 0;

	camera.id_user = selected_camera;

	if (selected_camera == 0) {
		tnfs_camera_set(&camera, 9);
	} else {
		tnfs_camera_set(&camera, selected_camera);
	}
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
	// pressed until the next control word is latched, see tnfs_control_word_get
	g_control_gear = 2;
}

void tnfs_change_gear_down() {
	g_control_gear = 1;
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

	tnfs_init_car();
	tnfs_reset_car(g_car_ptr_array[0]);
}

void tnfs_crash_car() {
	int i;
	for (i = 1; i < g_total_cars_in_scene; i++) {
		tnfs_collision_rollover_start(g_car_ptr_array[i], 0, 0, -0xa0000);
	}
}

/* common stub functions */

// The collision code (car-car, crash model, fence) calls the game's sound function through a wrapper that
// multiplies the volume by 20 (DOS 0x66360, PSX 0x8003abf8), so there the volume passed here is 1/20 of what
// the sample selection sees. Other callers (prop hits, gear shift, landing) pass it unchanged.
void tnfs_sfx_play(int a, int id1, int id2, int volume, int distance, int direction) {
	printf("sound %i %i\n", id1, id2);
}

/*
 * Replays (Win95 SE layout, see tnfs_replay_file in tnfs_base.h; DOS has the same code with other sizes).
 *
 * The original records the control word of every player once per control step (every 4 clock units, every second
 * simulation tick: the game reads its controls only then) and, every TNFS_REPLAY_KEYFRAME clock units, a keyframe of
 * every car and of the race globals. Right after saving a keyframe it reinstates it (restores the cars from it), so
 * the live race goes through exactly what a playback does. A playback reinstates the keyframe at each keyframe time
 * and feeds the recorded control words to the same simulation. The buffer is circular: the replay tick is the clock
 * modulo TNFS_REPLAY_TICKS.
 *
 * Replay mode g_is_playing (DOS 0xfd8e0, SE DAT_004c5390): 0 off (sim init), 1 race (recording), 2 playing back,
 * 3 cueing (simulating to a clock without drawing), 4 scrubbing (rewind / fast forward buttons held: no simulation).
 *
 * Front-end parts of the original functions (sounds, camera choice, replay panel, skid marks, HUD messages, timers)
 * are left out; they are noted where they happen.
 */
tnfs_replay_file g_replay;
int g_replay_tick = 0; // clock % TNFS_REPLAY_TICKS (DOS 0x15af44, SE 0x512d90)
int g_replay_speed = 4; // 4 full, 3 half, 2 quarter, 1 eighth, 0 paused (DOS 0xfd8e8, SE 0x4c5398)
int g_replay_frame_step = 0; // simulation ticks left in a frame step while paused (DOS 0xfd8ec, SE 0x4c539c)
int g_replay_speed_skip = 0; // timer ticks per simulation tick - 1 in slow motion: 0, 2, 4, 8 (DOS 0xfd8f0, SE 0x4c53a0)
int g_replay_return_mode = 4; // mode after the end of a replay: 1 back to the race, 4 replay again (DOS 0xfd8e4, SE 0x4c5394)
int g_replay_clip_cut = 0; // a crashed car was put back on the road (cuts a highlight clip) (DOS 0xfd8dc, SE 0x4c538c)
int g_replay_highlights_only = 0; // leave after the last highlight clip instead of looping (DOS 0x15c598, SE 0x5137e4)
extern int general_flags;
// AI globals of the keyframes (tnfs_ai.c)
extern int g_some_car_array2[8];
extern tnfs_random_struct g_random_struct[2];
extern int g_cop_park_position;
extern int g_random_direction[2];
extern int DAT_0016532c;

// world block: header
#define REPLAY_WORLD_END_CLOCK 0x04 // last recorded clock (DOS g_min_iSimTimeClock 0x15af4c, SE 0x5120c8)
#define REPLAY_WORLD_COUNTDOWN 0x10 // control steps until the next keyframe (DOS 0x15af58: 300, SE 0x5120d4: 0x1c2)
#define REPLAY_WORLD_KEYFRAMES 0x14 // keyframe records of 0xa2 bytes (room for 20 in SE, 16 used)
// recording block
#define REPLAY_CONTROLS_HIGH (2 * TNFS_REPLAY_SAMPLES) // low bytes of player 0, 1, then high bytes of player 0, 1
#define REPLAY_PLAYER_FRAMES (4 * TNFS_REPLAY_SAMPLES) // 0x7080 [player][keyframe] 0x1a4 bytes (DOS 36000)
#define REPLAY_OTHER_FRAMES (REPLAY_PLAYER_FRAMES + 2 * TNFS_REPLAY_KEYFRAMES * 0x1a4) // 0xa500 [keyframe][slot] 0x172

static void replay_put32(unsigned char *p, int v) {
	p[0] = (unsigned char) v;
	p[1] = (unsigned char) (v >> 8);
	p[2] = (unsigned char) (v >> 16);
	p[3] = (unsigned char) (v >> 24);
}

static int replay_get32(const unsigned char *p) {
	return (int) ((unsigned int) p[0] | (unsigned int) p[1] << 8 | (unsigned int) p[2] << 16 | (unsigned int) p[3] << 24);
}

static void replay_put_vec3(unsigned char *p, tnfs_vec3 *v) {
	replay_put32(p, v->x);
	replay_put32(p + 4, v->y);
	replay_put32(p + 8, v->z);
}

static void replay_get_vec3(const unsigned char *p, tnfs_vec3 *v) {
	v->x = replay_get32(p);
	v->y = replay_get32(p + 4);
	v->z = replay_get32(p + 8);
}

static void replay_put_vec9(unsigned char *p, tnfs_vec9 *m) {
	replay_put32(p, m->ax);
	replay_put32(p + 4, m->ay);
	replay_put32(p + 8, m->az);
	replay_put32(p + 12, m->bx);
	replay_put32(p + 16, m->by);
	replay_put32(p + 20, m->bz);
	replay_put32(p + 24, m->cx);
	replay_put32(p + 28, m->cy);
	replay_put32(p + 32, m->cz);
}

static void replay_get_vec9(const unsigned char *p, tnfs_vec9 *m) {
	m->ax = replay_get32(p);
	m->ay = replay_get32(p + 4);
	m->az = replay_get32(p + 8);
	m->bx = replay_get32(p + 12);
	m->by = replay_get32(p + 16);
	m->bz = replay_get32(p + 20);
	m->cx = replay_get32(p + 24);
	m->cy = replay_get32(p + 28);
	m->cz = replay_get32(p + 32);
}

static void replay_set_flags() {
	general_flags = (iSimTimeClock & 1) == 0;
	if ((iSimTimeClock & 3) == 0) {
		general_flags |= 4;
	}
	if ((iSimTimeClock & 7) == 0) {
		general_flags |= 0x10;
	}
}

int tnfs_replay_end_clock() {
	return replay_get32(g_replay.world + REPLAY_WORLD_END_CLOCK);
}

/*
 * Random generator seed (Win95 SE 0x428fe0 == DOS 0x8cd90; PSX 0x8005c3dc), called with the setup's random_seed
 * (SE: time() at race start, or the seed of a loaded replay) before the race is initialized.
 */
void tnfs_random_seed(unsigned int seed) {
	g_lcg_random_mod = seed * 3;
	if ((seed & 1) == 0) {
		seed = seed + 1;
	}
	g_lcg_random_seed = seed;
	if ((g_lcg_random_mod & 1) == 0) {
		g_lcg_random_mod = g_lcg_random_mod + 1;
	}
}

/*
 * Road sign status copies (Win95 SE 0x4501fc / 0x450250 / 0x4501a8, DOS 0x708c6 / 0x7091e / 0x7085f): byte 1 of
 * each prop is the replay copy of byte 0 (the keyframe + 1 when it was knocked down).
 */
void tnfs_sign_status_save() {
	int i;
	for (i = 0; i < g_road_object_count; i++) {
		g_sign_status[i * 2 + 1] = g_sign_status[i * 2];
	}
}

void tnfs_sign_status_restore() {
	int i;
	for (i = 0; i < g_road_object_count; i++) {
		g_sign_status[i * 2] = g_sign_status[i * 2 + 1];
	}
}

// stands the signs knocked down at keyframe "keyframe" or later up again
void tnfs_sign_status_replay_rewind(int keyframe) {
	int i;
	for (i = 0; i < g_road_object_count; i++) {
		if (keyframe <= (int) (unsigned char) g_sign_status[i * 2] - 1) {
			g_sign_status[i * 2] = 0;
		}
	}
}

/*
 * Replay buffers reset at race init (Win95 SE FUN_0044edd8, DOS tnfs_car_init_Mobile_InitCarData 0x6f28e).
 * The original allocates the recording buffer ("UsrCtrlBuff") without clearing it.
 */
void tnfs_replay_init() {
	int i;
	replay_put32(g_replay.world + REPLAY_WORLD_COUNTDOWN, 0x1c2);
	g_replay.highlights.clip = -2;
	replay_put32(g_replay.world, 0);
	replay_put32(g_replay.world + REPLAY_WORLD_END_CLOCK, 0);
	g_replay.highlights.return_pending = 0;
	g_replay.highlights.camera_restart = 0;
	g_replay.highlights.reinstating = 0;
	g_replay.highlights.current_second = 0;
	g_replay.highlights.play_count = 1;
	g_replay.highlights.panel_state = 6;
	for (i = 0; i < 480; i++) {
		g_replay.highlights.seconds[i] = 0;
	}
	g_replay.highlights.tv_camera = 0;
	g_replay_highlights_only = 0;
}

/*
 * Highlight scoring (Win95 SE FUN_0044d744, DOS 0x6d15b, PSX 0x800489a4), recording only: keeps the best score of each
 * second, marked with + 0x80 until tnfs_replay_highlights_select picks it. A later event in the same second wins only
 * when its score is higher than the stored value including the mark. Scores: 0x5c rollover start, 0x54 landing after
 * > 10 ticks in the air, 0x32 hard fence hit, 0x57 sliding sideways / spinning, 0x28 skidding fast, 0x78 the player
 * first moves, 0x50 race order change, 0x52 cop spun out, car-car hits from a table (20..98).
 */
void tnfs_replay_highlight_record(int score) {
	int *second;
	if (g_is_playing == 1) {
		g_replay.highlights.current_second = g_replay_tick / 0x3c;
		second = &g_replay.highlights.seconds[g_replay.highlights.current_second];
		if (*second >> 8 < iSimTimeClock / 0x3c) {
			*second = 0;
		}
		if ((*second & 0xff) < score) {
			*second = (iSimTimeClock / 0x3c) << 8 | (score + 0x80);
		}
	}
}

/*
 * Highlight clips (Win95 SE FUN_0044d7d8, DOS 0x6d221 with 600 seconds and + 0x28): up to 30 seconds with the best
 * marked scores, oldest first, each becomes the clip second - 2 .. second + 5 (clock rounded down to 4), overlapping
 * clips are merged (the higher coolness is kept). No highlight: one clip of the whole race, coolness 200.
 */
void tnfs_replay_highlights_select() {
	tnfs_replay_highlights *hl = &g_replay.highlights;
	int end;
	int count;
	int min;
	int pass;
	int i;
	int best;
	int best_i;
	int value;
	int swapped;
	int sorted;
	int cc;
	int s;
	int next;

	end = tnfs_replay_end_clock();
	count = end / 0x3c;
	if (count > 480) {
		count = 480;
	}
	min = (end - TNFS_REPLAY_TICKS) / 0x3c;
	if (min < -2) {
		min = -2;
	} else {
		min = min + 0x3c;
	}

	best_i = 0;
	s = 0;
	for (pass = 0; pass < 30; pass++) {
		best = 0x80;
		for (i = 0; i < count; i++) {
			value = hl->seconds[i] & 0xff;
			if (best <= value && min <= hl->seconds[i] >> 8) {
				hl->best_seconds[s] = hl->seconds[i] >> 8;
				best = value;
				best_i = i;
			}
		}
		if (best > 0x80) {
			s++;
			hl->seconds[best_i] -= 0x80;
		}
	}
	for (i = 0; i < count; i++) {
		value = hl->seconds[i] & 0xff;
		if (value != 0 && value < 0x80) {
			hl->seconds[i] += 0x80;
		}
	}

	sorted = 0;
	if (s > 0) {
		do {
			swapped = 0;
			for (i = 1; i < s; i++) {
				if (hl->best_seconds[i] < hl->best_seconds[i - 1]) {
					value = hl->best_seconds[i];
					hl->best_seconds[i] = hl->best_seconds[i - 1];
					hl->best_seconds[i - 1] = value;
					swapped = 1;
				}
			}
		} while (swapped && ++sorted < s);
	}

	hl->clip_count = 0;
	for (i = 0; i < s; i++) {
		cc = hl->clip_count;
		next = cc;
		if (min <= hl->best_seconds[i] - 2) {
			hl->clips[cc][0] = (hl->best_seconds[i] - 2) * 0x3c;
			hl->clips[cc][1] = (hl->best_seconds[i] + 5) * 0x3c;
			// the original indexes the seconds by the absolute second, which is past the array after 8 minutes: up to
			// second 488 it reads the following highlights fields (as here), from 489 on SE globals the port doesn't
			// have (0 here); coolness only picks the camera and slow motion (front end) and the merged maximum
			value = hl->best_seconds[i];
			if (value >= 0 && value < 480 + 9) {
				hl->clips[cc][2] = ((int *) hl->seconds)[value] & 0xff;
			} else {
				hl->clips[cc][2] = 0;
			}
			hl->clips[cc][0] -= hl->clips[cc][0] % 4;
			hl->clips[cc][1] -= hl->clips[cc][1] % 4;
			if (hl->clips[cc][0] < 0) {
				hl->clips[cc][0] = 0;
			}
			if (end < hl->clips[cc][1]) {
				hl->clips[cc][1] = end;
			}
			next = cc + 1;
			if (next > 1 && hl->clips[cc][0] < hl->clips[cc - 1][1]) {
				hl->clips[cc - 1][1] = hl->clips[cc][1];
				if (hl->clips[cc - 1][2] < hl->clips[cc][2]) {
					hl->clips[cc - 1][2] = hl->clips[cc][2];
				}
				next = cc;
			}
		}
		hl->clip_count = next;
	}
	if (hl->clip_count == 0) {
		hl->clips[0][0] = 0;
		hl->clips[0][1] = end;
		hl->clips[0][2] = 200;
		hl->clip_count = 1;
	}
}

/*
 * Control samples (Win95 SE FUN_0044db34 store / FUN_0044dbc8 load, DOS 0x6d8f9 / 0x6d9ac): one per control step at
 * replay tick / 4; low byte of the word in the first area, high byte in the second (2 players).
 */
static void tnfs_replay_store_controls() {
	int i;
	int sample;
	sample = g_replay_tick / 4;
	for (i = 0; i < g_number_of_players && i < 2; i++) {
		g_replay.recording[i * TNFS_REPLAY_SAMPLES + sample] = (unsigned char) g_control_word[i];
		g_replay.recording[REPLAY_CONTROLS_HIGH + i * TNFS_REPLAY_SAMPLES + sample] = (unsigned char) (g_control_word[i] >> 8);
	}
}

static void tnfs_replay_load_controls() {
	int i;
	int sample;
	sample = g_replay_tick / 4;
	for (i = 0; i < g_number_of_players && i < 2; i++) {
		g_control_word[i] = g_replay.recording[i * TNFS_REPLAY_SAMPLES + sample] //
				| g_replay.recording[REPLAY_CONTROLS_HIGH + i * TNFS_REPLAY_SAMPLES + sample] << 8;
	}
}

/*
 * Car state of a keyframe, 0x168 bytes (Win95 SE FUN_0044dc40 save / FUN_0044dd9c restore, DOS 0x6da74 / 0x6dc64).
 * Offsets in the comments are the record's; car offsets are DOS (SE + 0x31 from 0x47d on).
 */
static void tnfs_replay_car_save(tnfs_car_data *car, unsigned char *r) {
	tnfs_collision_data *c = &car->collision_data;

	// 0x00 control handler (DOS +0x4dd: 1 player controls, 2 AI); the port has no such field
	replay_put32(r + 0x00, (car->car_id >= 0 && car->car_id < g_number_of_players) ? 1 : 2);
	replay_put32(r + 0x04, car->crash_state);
	replay_put32(r + 0x08, car->car_id << 24 | car->field_4e9 << 8);
	replay_put_vec3(r + 0x0c, &car->position);
	replay_put_vec3(r + 0x18, &car->angle);
	replay_put32(r + 0x24, car->steer_angle);
	replay_put32(r + 0x28, car->target_angle);
	replay_put32(r + 0x2c, car->is_crashed);
	replay_put_vec9(r + 0x30, &car->matrix);
	replay_put32(r + 0x54, car->track_slice);
	replay_put32(r + 0x58, car->lap_number);
	replay_put32(r + 0x5c, car->speed_x);
	replay_put32(r + 0x60, car->speed_y);
	replay_put32(r + 0x64, car->speed_z);
	replay_put32(r + 0x68, car->speed_local_lat);
	replay_put32(r + 0x6c, car->speed_local_vert);
	replay_put32(r + 0x70, car->speed_local_lon);
	replay_put32(r + 0x74, car->speed);
	replay_put32(r + 0x78, car->angular_speed);
	replay_put32(r + 0x7c, car->car_length);
	replay_put32(r + 0x80, car->car_width);
	replay_put32(r + 0x84, car->center_line_distance);
	replay_put32(r + 0x88, car->side_width);
	replay_put_vec3(r + 0x8c, &car->road_fence_normal);
	replay_put_vec3(r + 0x98, &car->road_surface_normal);
	replay_put_vec3(r + 0xa4, &car->road_heading);
	replay_put_vec3(r + 0xb0, &car->road_position);
	replay_put32(r + 0xbc, car->ai_state);
	replay_put32(r + 0xc0, car->collision_height_offset);
	// 0xc4: the 0x94 bytes of the crash body (DOS tnfs_collision_data)
	replay_put_vec9(r + 0xc4, &c->matrix);
	replay_put_vec3(r + 0xc4 + 0x24, &c->position);
	replay_put_vec3(r + 0xc4 + 0x30, &c->speed);
	replay_put_vec3(r + 0xc4 + 0x3c, &c->angular_speed);
	replay_put_vec3(r + 0xc4 + 0x48, &c->gravity_vector);
	// 0xc4 + 0x54..0x5f: not in the port's struct
	replay_put32(r + 0xc4 + 0x60, c->mass);
	replay_put32(r + 0xc4 + 0x64, c->moment_of_inertia);
	replay_put32(r + 0xc4 + 0x68, c->linear_acc_factor);
	replay_put32(r + 0xc4 + 0x6c, c->angular_acc_factor);
	replay_put32(r + 0xc4 + 0x70, c->edge_length);
	replay_put_vec3(r + 0xc4 + 0x74, &c->size);
	replay_put32(r + 0xc4 + 0x80, c->state_timer);
	replay_put32(r + 0xc4 + 0x84, c->field_084);
	replay_put32(r + 0xc4 + 0x88, c->field_088);
	replay_put32(r + 0xc4 + 0x8c, c->field_08c);
	replay_put32(r + 0xc4 + 0x90, c->traffic_speed_factor);
	replay_put32(r + 0x158, car->car_road_speed);
	replay_put32(r + 0x15c, car->field_158);
	replay_put32(r + 0x160, car->lane_slack);
	replay_put32(r + 0x164, car->is_wrecked);
}

static void tnfs_replay_car_restore(tnfs_car_data *car, const unsigned char *r) {
	tnfs_collision_data *c = &car->collision_data;
	int v;

	// 0x00 control handler: not kept by the port
	car->crash_state = replay_get32(r + 0x04);
	v = replay_get32(r + 0x08);
	car->car_id = v >> 24;
	car->field_4e9 = (v >> 8) & 0xffff;
	replay_get_vec3(r + 0x0c, &car->position);
	replay_get_vec3(r + 0x18, &car->angle);
	car->steer_angle = replay_get32(r + 0x24);
	car->target_angle = replay_get32(r + 0x28);
	car->is_crashed = replay_get32(r + 0x2c);
	replay_get_vec9(r + 0x30, &car->matrix);
	car->track_slice = replay_get32(r + 0x54);
	car->lap_number = replay_get32(r + 0x58);
	car->speed_x = replay_get32(r + 0x5c);
	car->speed_y = replay_get32(r + 0x60);
	car->speed_z = replay_get32(r + 0x64);
	car->speed_local_lat = replay_get32(r + 0x68);
	car->speed_local_vert = replay_get32(r + 0x6c);
	car->speed_local_lon = replay_get32(r + 0x70);
	car->speed = replay_get32(r + 0x74);
	car->angular_speed = replay_get32(r + 0x78);
	car->car_length = replay_get32(r + 0x7c);
	car->car_width = replay_get32(r + 0x80);
	car->center_line_distance = replay_get32(r + 0x84);
	car->side_width = replay_get32(r + 0x88);
	replay_get_vec3(r + 0x8c, &car->road_fence_normal);
	replay_get_vec3(r + 0x98, &car->road_surface_normal);
	replay_get_vec3(r + 0xa4, &car->road_heading);
	replay_get_vec3(r + 0xb0, &car->road_position);
	car->ai_state = replay_get32(r + 0xbc);
	car->collision_height_offset = replay_get32(r + 0xc0);
	replay_get_vec9(r + 0xc4, &c->matrix);
	replay_get_vec3(r + 0xc4 + 0x24, &c->position);
	replay_get_vec3(r + 0xc4 + 0x30, &c->speed);
	replay_get_vec3(r + 0xc4 + 0x3c, &c->angular_speed);
	replay_get_vec3(r + 0xc4 + 0x48, &c->gravity_vector);
	c->mass = replay_get32(r + 0xc4 + 0x60);
	c->moment_of_inertia = replay_get32(r + 0xc4 + 0x64);
	c->linear_acc_factor = replay_get32(r + 0xc4 + 0x68);
	c->angular_acc_factor = replay_get32(r + 0xc4 + 0x6c);
	c->edge_length = replay_get32(r + 0xc4 + 0x70);
	replay_get_vec3(r + 0xc4 + 0x74, &c->size);
	c->state_timer = replay_get32(r + 0xc4 + 0x80);
	c->field_084 = replay_get32(r + 0xc4 + 0x84);
	c->field_088 = replay_get32(r + 0xc4 + 0x88);
	c->field_08c = replay_get32(r + 0xc4 + 0x8c);
	c->traffic_speed_factor = replay_get32(r + 0xc4 + 0x90);
	car->car_road_speed = replay_get32(r + 0x158);
	car->field_158 = replay_get32(r + 0x15c);
	car->lane_slack = replay_get32(r + 0x160);
	car->is_wrecked = replay_get32(r + 0x164);
	car->slide_front = 0;
	car->slide_rear = 0;
	car->track_slice_lap = (car->lap_number - 1) * g_tri_num_chunks * 4 + car->track_slice;
}

/*
 * Keyframe of one car (Win95 SE FUN_0044e154, DOS 0x6e298): players (car_id < g_number_of_players) in the player area
 * by car_id, 0x1a4 bytes; the other cars in the next free slot of the keyframe, 0x172 bytes.
 */
static void tnfs_replay_car_keyframe_save(tnfs_car_data *car, int slot) {
	unsigned char *r;
	int k = g_replay_tick / TNFS_REPLAY_KEYFRAME;

	if (car->car_id < g_number_of_players) {
		r = g_replay.recording + REPLAY_PLAYER_FRAMES + car->car_id * (TNFS_REPLAY_KEYFRAMES * 0x1a4) + k * 0x1a4;
		tnfs_replay_car_save(car, r);
		replay_put32(r + 0x170, car->road_grip_increment);
		replay_put32(r + 0x174, car->tire_grip_rear);
		replay_put32(r + 0x178, car->tire_grip_front);
		replay_put32(r + 0x190, car->slope_force_lat);
		replay_put32(r + 0x194, car->unknown_flag_3DD);
		replay_put32(r + 0x198, car->slope_force_lon);
		replay_put32(r + 0x19c, car->thrust);
		replay_put32(r + 0x18c, car->gear_shift_button);
		replay_put32(r + 0x180, car->tire_grip_loss);
		replay_put32(r + 0x17c, car->speed_drivetrain);
		r[0x184] = (unsigned char) car->gear_auto_selected;
		r[0x168] = (unsigned char) car->throttle;
		r[0x169] = (unsigned char) car->throttle_previous_pos;
		r[0x16a] = (unsigned char) car->brake;
		replay_put32(r + 0x188, car->time_off_ground);
		replay_put32(r + 0x16c, car->rpm_vehicle << 16 | (car->rpm_engine & 0xffff));
		r[0x186] = (unsigned char) (car->is_engine_cutoff * 2 | car->handbrake << 2 | car->is_gear_engaged << 3 | car->wheels_on_ground);
		if ((car->tire_skid_rear & 2) != 0) {
			r[0x186] |= 0x20;
		}
		// 0x187 is never written
		r[0x185] = (unsigned char) (car->gear_selected + 2);
		r[0x16b] = (unsigned char) (car->is_shifting_gears + 100);
		replay_put32(r + 0x1a0, car->surface_type);
	} else {
		r = g_replay.recording + REPLAY_OTHER_FRAMES + k * (8 * 0x172) + slot * 0x172;
		tnfs_replay_car_save(car, r);
		replay_put32(r + 0x168, car->speed_target);
		replay_put32(r + 0x16c, car->target_center_line);
		r[0x171] = 0; // byte at DOS +0x340, which nothing else reads
		r[0x170] = car->wheels_on_ground;
	}
}

/*
 * Reinstates the cars of a keyframe (Win95 SE FUN_0044df20, DOS 0x6df8e): player "index", or the other car in slot
 * "index" (whose record names the car).
 */
static void tnfs_replay_car_keyframe_restore(int is_player, int k, int index) {
	tnfs_car_data *car;
	const unsigned char *r;
	int flags;

	if (is_player) {
		car = g_car_ptr_array[index];
		r = g_replay.recording + REPLAY_PLAYER_FRAMES + index * (TNFS_REPLAY_KEYFRAMES * 0x1a4) + k * 0x1a4;
		tnfs_replay_car_restore(car, r);
		car->throttle = r[0x168];
		car->throttle_previous_pos = r[0x169];
		car->brake = r[0x16a];
		car->road_grip_increment = replay_get32(r + 0x170);
		car->tire_grip_rear = replay_get32(r + 0x174);
		car->tire_grip_front = replay_get32(r + 0x178);
		car->slope_force_lat = replay_get32(r + 0x190);
		car->unknown_flag_3DD = replay_get32(r + 0x194);
		car->slope_force_lon = replay_get32(r + 0x198);
		car->thrust = replay_get32(r + 0x19c);
		car->gear_shift_button = replay_get32(r + 0x18c);
		car->tire_grip_loss = replay_get32(r + 0x180);
		car->speed_drivetrain = replay_get32(r + 0x17c);
		car->gear_auto_selected = r[0x184];
		car->is_shifting_gears = r[0x16b] - 100;
		car->gear_selected = r[0x185] - 2;
		car->rpm_vehicle = replay_get32(r + 0x16c) >> 16;
		car->rpm_engine = replay_get32(r + 0x16c) & 0xffff;
		car->time_off_ground = replay_get32(r + 0x188);
		flags = r[0x186];
		if (((flags >> 5) & 1) == 0) {
			car->tire_skid_rear = 0;
		} else {
			car->tire_skid_rear = 2;
		}
		car->is_gear_engaged = (flags >> 3) & 1;
		car->handbrake = (flags >> 2) & 1;
		car->is_engine_cutoff = (flags >> 1) & 1;
		car->wheels_on_ground = flags & 1;
		car->surface_type = replay_get32(r + 0x1a0);
		car->tire_skid_front = 0;
		// the crash model's tyre sound override (SE DAT_004c5f98 / 9c)
		DAT_000FDCEC = -1;
		DAT_000FDCF0 = -1;
	} else {
		r = g_replay.recording + REPLAY_OTHER_FRAMES + k * (8 * 0x172) + index * 0x172;
		if ((replay_get32(r + 8) >> 24) >= g_total_cars_in_scene) {
			// the port has 8 car slots, SE 9 (a head-to-head with a cop and 6 traffic cars): not restored
			return;
		}
		car = g_car_ptr_array[replay_get32(r + 8) >> 24];
		tnfs_replay_car_restore(car, r);
		car->speed_target = replay_get32(r + 0x168);
		car->target_center_line = replay_get32(r + 0x16c);
		car->wheels_on_ground = r[0x170];
		// r[0x171] goes to the byte at DOS +0x340, which nothing else reads
	}
}

/*
 * Race globals of keyframe k, 0xa2 bytes at world + 0x14 + k * 0xa2 (Win95 SE FUN_0044e8f8 / tnfs_replay_reinstate,
 * DOS tnfs_control_0006ec66 / 0x6e53b). Unaligned record, so written byte by byte.
 */
static void tnfs_replay_world_save(int k, int other_cars) {
	unsigned char *r = g_replay.world + REPLAY_WORLD_KEYFRAMES + k * 0xa2;
	int i;

	for (i = 0; i < g_total_cars_in_scene; i++) {
		// 0x00 handler order (DOS g_some_car_array1 0x153bec, SE 0x509e68): the port runs the cars in index order
		replay_put32(r + i * 4, i);
		// 0x24 race order (g_some_car_array2)
		replay_put32(r + 0x24 + i * 4, g_some_car_array2[i]);
	}
	// 0x48, 0x4c: never written
	replay_put32(r + 0x50, other_cars);
	replay_put32(r + 0x54, g_ai_frame_counter);
	replay_put32(r + 0x58, g_police_on_chase);
	// 0x5c, 0x68: g_random_struct[0], [1] (12 bytes each: id, a, b, c, d, e; DOS 0x165120, SE 0x52b478, the Ghidra
	// labels g_random_struct DOS 0x16511e / SE DAT_0052b476 are 2 bytes before the array)
	for (i = 0; i < 2; i++) {
		r[0x5c + i * 12 + 0] = (unsigned char) g_random_struct[i].id;
		r[0x5c + i * 12 + 1] = (unsigned char) (g_random_struct[i].id >> 8);
		r[0x5c + i * 12 + 2] = (unsigned char) g_random_struct[i].a;
		r[0x5c + i * 12 + 3] = (unsigned char) (g_random_struct[i].a >> 8);
		r[0x5c + i * 12 + 4] = (unsigned char) g_random_struct[i].b;
		r[0x5c + i * 12 + 5] = (unsigned char) (g_random_struct[i].b >> 8);
		r[0x5c + i * 12 + 6] = (unsigned char) g_random_struct[i].c;
		r[0x5c + i * 12 + 7] = (unsigned char) (g_random_struct[i].c >> 8);
		r[0x5c + i * 12 + 8] = (unsigned char) g_random_struct[i].d;
		r[0x5c + i * 12 + 9] = (unsigned char) (g_random_struct[i].d >> 8);
		r[0x5c + i * 12 + 10] = (unsigned char) g_random_struct[i].e;
		r[0x5c + i * 12 + 11] = (unsigned char) (g_random_struct[i].e >> 8);
	}
	replay_put32(r + 0x74, g_police_chase_time);
	replay_put32(r + 0x78, 0); // siren sound state (DOS DAT_000fdcf4, SE DAT_004c5fa0), sound code
	replay_put32(r + 0x7c, g_lcg_random_mod);
	replay_put32(r + 0x80, g_cop_park_position);
	replay_put32(r + 0x84, g_random_direction[0]);
	replay_put32(r + 0x88, g_random_direction[1]);
	replay_put32(r + 0x8c, DAT_0016532c);
	replay_put32(r + 0x90, 0); // DOS DAT_00153b94 / SE DAT_00509e10, read by nothing else
	// 0x94: never written
	r[0x98] = 0; // DOS DAT_000f9e4c / SE DAT_004b9624 (bit 0 restored), read by nothing else
	r[0x99] = (unsigned char) g_police_speeding_ticket;
	replay_put32(r + 0x9a, 0); // split time checkpoint of the player (DOS DAT_001664fc, SE DAT_0051c970), HUD
	replay_put32(r + 0x9e, 0); // split time checkpoint of the opponent (DOS DAT_00166500, SE DAT_0051c974), HUD
}

/*
 * Restores keyframe k (Win95 SE tnfs_replay_reinstate 0x44e354, DOS 0x6e53b). k = tick / TNFS_REPLAY_KEYFRAME, or
 * param_2 + 16 (DOS + 30) for param_2 >= 0, which nothing calls. Camera and sound restarts are left out.
 */
void tnfs_replay_reinstate(int tick, int param_2) {
	unsigned char *r;
	int k;
	int i;
	int other_cars;

	if (param_2 >= 0) {
		iSimTimeClock = g_replay.highlights.return_clock[param_2];
		replay_set_flags();
		k = param_2 + 0x10;
	} else {
		k = tick / TNFS_REPLAY_KEYFRAME;
	}
	tnfs_sign_status_restore();
	tnfs_sign_status_replay_rewind(iSimTimeClock / TNFS_REPLAY_KEYFRAME);

	r = g_replay.world + REPLAY_WORLD_KEYFRAMES + k * 0xa2;
	for (i = 0; i < g_total_cars_in_scene; i++) {
		// handler order (r + i * 4): the port runs the cars in index order
		g_some_car_array2[i] = replay_get32(r + 0x24 + i * 4);
	}
	other_cars = replay_get32(r + 0x50);
	g_lcg_random_mod = replay_get32(r + 0x7c);
	g_police_chase_time = replay_get32(r + 0x74);
	g_police_speeding_ticket = r[0x99];
	// r + 0x78: siren sound state
	g_police_on_chase = replay_get32(r + 0x58);
	g_cop_park_position = replay_get32(r + 0x80);
	g_random_direction[0] = replay_get32(r + 0x84);
	g_random_direction[1] = replay_get32(r + 0x88);
	DAT_0016532c = replay_get32(r + 0x8c);
	// r + 0x90, r[0x98]: globals read by nothing else
	for (i = 0; i < 2; i++) {
		g_random_struct[i].id = (short) (r[0x5c + i * 12 + 0] | r[0x5c + i * 12 + 1] << 8);
		g_random_struct[i].a = (short) (r[0x5c + i * 12 + 2] | r[0x5c + i * 12 + 3] << 8);
		g_random_struct[i].b = (short) (r[0x5c + i * 12 + 4] | r[0x5c + i * 12 + 5] << 8);
		g_random_struct[i].c = (short) (r[0x5c + i * 12 + 6] | r[0x5c + i * 12 + 7] << 8);
		g_random_struct[i].d = (short) (r[0x5c + i * 12 + 8] | r[0x5c + i * 12 + 9] << 8);
		g_random_struct[i].e = (short) (r[0x5c + i * 12 + 10] | r[0x5c + i * 12 + 11] << 8);
	}
	for (i = 0; i < g_total_cars_in_scene; i++) {
		g_car_ptr_array[i]->road_object_index = 0;
		g_car_ptr_array[i]->road_object_slice = 0;
	}
	g_ai_frame_counter = replay_get32(r + 0x54);
	// r + 0x9a, r + 0x9e: split time checkpoints (HUD)

	for (i = 0; i < g_number_of_players && i < 2; i++) {
		if (param_2 >= 0) {
			g_replay.highlights.reinstating = 1;
		}
		tnfs_replay_car_keyframe_restore(1, k, i);
	}
	for (i = 0; i < other_cars; i++) {
		tnfs_replay_car_keyframe_restore(0, k, i);
	}
	g_replay.highlights.reinstating = 0;

	if (param_2 >= 0) {
		if (g_is_playing == 1) {
			tnfs_sign_status_save();
		}
		g_is_playing = 2;
		g_replay.highlights.camera_restart = 1;
	}
	// camera re-targeted unless recording, looped sounds restarted when sound_restart: front end
}

/*
 * Recording, once per control step in mode 1 (Win95 SE FUN_0044e8f8, DOS tnfs_control_0006ec66): latches the
 * control words (neutral before clock 0x78), then either stores a sample or, every TNFS_REPLAY_KEYFRAME clock units
 * (countdown of 0x1c2 control steps), saves a keyframe (sign status, race globals, sample, cars) and reinstates it.
 * Only the local player's word is read here (SE FUN_004397f4 pops every player's word from the input / network
 * queues); the words of other players are whatever the caller put in g_control_word.
 */
static void tnfs_replay_record() {
	int k;
	int i;
	int other_cars;
	int countdown;
	tnfs_car_data *car;

	g_control_word[g_player_id] = tnfs_control_word_get();

	if (iSimTimeClock < 0x78) {
		for (i = 0; i < g_number_of_players; i++) {
			g_control_word[i] = 0x7a0;
		}
	}

	k = g_replay_tick / TNFS_REPLAY_KEYFRAME;
	countdown = replay_get32(g_replay.world + REPLAY_WORLD_COUNTDOWN);
	if (countdown == 0) {
		tnfs_sign_status_save();
		replay_put32(g_replay.world + REPLAY_WORLD_COUNTDOWN, 0x1c2);
		other_cars = 0;
		tnfs_replay_store_controls();
		for (i = 0; i < g_total_cars_in_scene; i++) {
			car = g_car_ptr_array[i];
			tnfs_replay_car_keyframe_save(car, other_cars);
			if (g_number_of_players <= car->car_id) {
				other_cars++;
			}
		}
		tnfs_replay_world_save(k, other_cars);
		tnfs_replay_reinstate(g_replay_tick, -1);
	} else {
		tnfs_replay_store_controls();
	}
	replay_put32(g_replay.world + REPLAY_WORLD_COUNTDOWN, replay_get32(g_replay.world + REPLAY_WORLD_COUNTDOWN) - 1);
}

/*
 * Port extra: whether a player of the keyframe at clock "keyframe" runs the checkpoint flick, i.e.
 * tnfs_racer_crossed_finish_line != 0 (behind the start or past the finish), read from the keyframe's player record
 * (track_slice r + 0x54, lap_number r + 0x58) without that function's side effects.
 */
static int tnfs_replay_keyframe_flicking(int keyframe) {
	const unsigned char *r;
	int k = (keyframe % TNFS_REPLAY_TICKS) / TNFS_REPLAY_KEYFRAME;
	int i;
	int slice;
	int lap;

	for (i = 0; i < g_number_of_players && i < 2; i++) {
		r = g_replay.recording + REPLAY_PLAYER_FRAMES + i * (TNFS_REPLAY_KEYFRAMES * 0x1a4) + k * 0x1a4;
		slice = replay_get32(r + 0x54);
		lap = replay_get32(r + 0x58);
		if (g_is_closed_track == 0) {
			if (slice < 0xc || slice > g_road_finish_node) {
				return 1;
			}
		} else {
			if (lap < 1 || lap > g_track_laps) {
				return 1;
			}
		}
	}
	return 0;
}

/*
 * Port extra, not in any binary: the keyframe a seek to "clock" cues from. The original always takes the keyframe
 * before the clock. But while tnfs_racer_crossed_finish_line is nonzero, tnfs_controls_update runs
 * tnfs_driving_checkpoint_flick every control step, whose state is in player fields the keyframes don't hold:
 * field_4cd (0 none, 1 flicking, 2 turned), field_4c9 (counter), field_4c5 (lateral offset at the flick's start),
 * field_4d1 (steer). There is no room for them in the player record (one spare byte), and field_4c5 can't be derived.
 * So a seek to after the finish kept them from wherever playback was, and the player left the race's line at once.
 * Before the line every control step sets field_4cd = 0 and the flick's first step sets the others, so the seek goes
 * back to the last keyframe at which no player is past the line (or behind the start), as long as that keyframe is
 * still in the buffer (its slot not overwritten: keyframe > end - TNFS_REPLAY_TICKS). The original's seeks diverge.
 */
static int tnfs_replay_cue_keyframe(int clock) {
	int end = tnfs_replay_end_clock();
	int keyframe = (clock / TNFS_REPLAY_KEYFRAME) * TNFS_REPLAY_KEYFRAME;

	while (keyframe >= TNFS_REPLAY_KEYFRAME && keyframe - TNFS_REPLAY_KEYFRAME > end - TNFS_REPLAY_TICKS
			&& tnfs_replay_keyframe_flicking(keyframe)) {
		keyframe -= TNFS_REPLAY_KEYFRAME;
	}
	return keyframe;
}

/*
 * Moves the replay to a clock (Win95 SE FUN_0044e700, DOS tnfs_game_clock_0006e9e0), playing until play_to (capped at
 * the end of the recording). On a keyframe it plays at once (mode 2), otherwise it goes back to the keyframe before
 * and cues (mode 3: the game loop simulates without drawing until seek_target, the caller runs tnfs_update until the
 * mode is not 3, as SE tnfs_game_InitSim does). Leaving the race (mode 1) saves the sign status.
 * Front end, left out: sounds stopped, skid marks and HUD message timers reset, the race timers saved.
 * Port extra: past the finish line it cues from an earlier keyframe, also for a clock on a keyframe.
 */
void tnfs_replay_seek(int clock, int play_to) {
	int end = tnfs_replay_end_clock();
	int keyframe;

	if (end < play_to) {
		play_to = end;
	}
	g_replay_tick = clock % TNFS_REPLAY_TICKS;
	g_replay.highlights.play_to = play_to;
	// port extra: may go back further than the keyframe before the clock (tnfs_replay_cue_keyframe)
	keyframe = tnfs_replay_cue_keyframe(clock);
	if (keyframe == clock) {
		iSimTimeClock = clock;
		if (g_is_playing == 1) {
			tnfs_sign_status_save();
		}
		g_is_playing = 2;
		g_replay.highlights.sound_restart = 1;
		g_replay.highlights.camera_restart = 1;
	} else {
		iSimTimeClock = keyframe;
		g_replay.highlights.seek_target = clock;
		if (g_is_playing == 1) {
			tnfs_sign_status_save();
		}
		g_is_playing = 3;
		g_replay.highlights.camera_restart = 0;
	}
	g_replay_tick = iSimTimeClock % TNFS_REPLAY_TICKS;
	replay_set_flags();
	g_replay.highlights.reinstating = 1;
	tnfs_smoke_particles_reset(&g_smoke_particles);
	g_replay.highlights.sound_restart = 1;
}

/*
 * One played control step (Win95 SE FUN_0044ecc4, DOS 0x6f125): reinstates the keyframe at keyframe times and loads
 * the recorded control words. The camera for the clip (by its coolness, random), the TV camera change every 256 clock
 * units and the sounds are front end; the first played tick of a replay stores the clock to loop back to.
 */
void tnfs_replay_play_tick() {
	if (g_replay_tick % TNFS_REPLAY_KEYFRAME == 0) {
		tnfs_replay_reinstate(g_replay_tick, -1);
	}
	tnfs_replay_load_controls();
	if (g_replay.highlights.camera_restart != 0) {
		g_replay_clip_cut = 0;
		g_replay.highlights.camera_restart = 0;
		if (g_replay.highlights.return_pending != 0) {
			g_replay.highlights.return_pending = 0;
			g_replay.highlights.return_clock[0] = iSimTimeClock;
		}
	}
}

/*
 * Next highlight clip (Win95 SE FUN_0044fc74, DOS 0x702f3); after the last one the highlights loop, or end the replay
 * (DAT_00153B24 = 0x20, the game's exit code) in highlights-only mode. The random slow motion of cool clips (coolness
 * 0xdb..0xf7, the cop clip 0xe4 only near the other player) is front end.
 */
static void tnfs_replay_next_clip() {
	tnfs_replay_highlights *hl = &g_replay.highlights;

	g_replay_speed = 4;
	g_replay_speed_skip = 0;
	hl->clip++;
	if (hl->clip >= hl->clip_count) {
		if (g_replay_highlights_only == 0) {
			hl->clip = 0;
			tnfs_replay_seek(hl->clips[0][0], hl->clips[0][1]);
			tnfs_replay_play_tick();
			return;
		}
		DAT_00153B24 = 0x20;
		return;
	}
	tnfs_replay_seek(hl->clips[hl->clip][0], hl->clips[hl->clip][1]);
	tnfs_replay_play_tick();
}

/*
 * Playback, once per control step in mode 2 (Win95 SE FUN_0044fe84, DOS tnfs_replay_00070565). At the end of a
 * replay that is not a highlight clip it switches to g_replay_return_mode: back to the race (1, the race goes on from
 * the replayed state) or a seek back to where the replay started (4: it loops).
 */
static void tnfs_replay_playback() {
	tnfs_replay_highlights *hl = &g_replay.highlights;
	int end = tnfs_replay_end_clock();

	if (g_replay_speed == 0 && g_replay_frame_step < 1) {
		hl->panel_state = 5;
		replay_set_flags();
		return;
	}
	if (iSimTimeClock < hl->play_to && (g_replay_clip_cut == 0 || hl->clip < 0)) {
		tnfs_replay_play_tick();
		return;
	}
	if (hl->play_to == end && hl->clip < 0) {
		g_is_playing = g_replay_return_mode;
		// skid marks, camera, sounds: front end
		if (g_is_playing != 1) {
			iSimTimeClock = hl->return_clock[0];
			replay_set_flags();
			g_replay_tick = iSimTimeClock % TNFS_REPLAY_TICKS;
			tnfs_replay_seek(iSimTimeClock, end);
			tnfs_replay_play_tick();
		}
		// else the race timers are restored (front end)
		return;
	}
	if (hl->clip >= 0) {
		tnfs_replay_next_clip();
	}
}

/*
 * Replay part of the game loop, on control steps (Win95 SE FUN_00450020, DOS FUN_000706e2). The replay panel input
 * (SE FUN_0044f9b8, DOS tnfs_game_00070023) and the end of race check run before it in the original (front end).
 */
void tnfs_replay_update() {
	if ((general_flags & 4) != 0) {
		g_replay_tick = iSimTimeClock % TNFS_REPLAY_TICKS;
		if (g_is_playing == 1) {
			tnfs_replay_record();
			replay_put32(g_replay.world + REPLAY_WORLD_END_CLOCK, iSimTimeClock);
		} else if (g_is_playing == 2) {
			tnfs_replay_playback();
			g_replay.highlights.play_count++;
		} else if (g_is_playing == 3) {
			if (g_replay.highlights.seek_target <= iSimTimeClock) {
				g_is_playing = 2;
				g_replay.highlights.play_count++;
				g_replay.highlights.camera_restart = 1;
			}
			if (iSimTimeClock < g_replay.highlights.play_to) {
				tnfs_replay_play_tick();
			}
		} else if (g_is_playing == 4) {
			replay_set_flags();
		}
	}
}

/*
 * Starts a replay (Win95 SE FUN_0044f188, DOS 0x6f763): 2 the highlights (from the race: the camera mode is kept to
 * return to), 3 the whole buffer (the last 27000 clock units, TV cameras), else the last 600 clock units (10 s).
 * 3 and others loop (return mode 4).
 */
void tnfs_replay_start(int type) {
	tnfs_replay_highlights *hl = &g_replay.highlights;
	int end = tnfs_replay_end_clock();
	int clock;

	if (type == 2) {
		if (g_is_playing == 1) {
			hl->saved_camera = camera.id_user;
		}
		tnfs_replay_highlights_select();
		if (hl->clip_count > 0) {
			hl->clip = 0;
			tnfs_replay_seek(hl->clips[0][0], hl->clips[0][1]);
			g_replay_speed = 4;
			g_replay_speed_skip = 0;
		}
		return;
	}
	hl->clip = -2;
	g_replay_speed = 4;
	g_replay_speed_skip = 0;
	if (type == 3) {
		clock = end - 27000;
	} else {
		clock = end - 600;
	}
	if (clock < 0) {
		clock = 0;
	}
	hl->return_pending = 1;
	g_replay_return_mode = 4;
	tnfs_replay_seek(clock - clock % 4, end);
	if (type == 3) {
		hl->tv_camera = 1;
	}
	hl->clip = -1;
}

/*
 * Back to the race from a replay (Win95 SE FUN_0044f090, DOS 0x6f62d): plays the last 8 clock units, then
 * (return mode 1) the race goes on. The camera mode saved by tnfs_replay_start(2) is restored by the front end.
 */
void tnfs_replay_return_to_race() {
	int end = tnfs_replay_end_clock();
	int clock = end - 8;

	if (clock < 0) {
		clock = 0;
	}
	tnfs_replay_seek(clock - clock % 4, end);
	g_replay.highlights.clip = -2;
	g_replay_speed = 4;
	g_replay_speed_skip = 0;
	g_replay_return_mode = 1;
	g_replay.highlights.panel_state = 6;
}

/*
 * Replay panel time buttons (Win95 SE FUN_0044ef3c, DOS tnfs_game_clock_0006f408) while held (repeat = frames held):
 * 1 to the start of the buffer, 2 to the end (300 clock units before it), 3 back, 4 forward, by (repeat / 16) * 4
 * clock units (4 at first). Mode 4: the simulation stops; releasing (any tnfs_replay_speed_command) cues the clock.
 */
void tnfs_replay_scrub(int command, int repeat) {
	int end = tnfs_replay_end_clock();
	int step = (repeat / 16) * 4;
	int first = end - (TNFS_REPLAY_TICKS - TNFS_REPLAY_KEYFRAME / 2);

	if (end - 300 < iSimTimeClock) {
		iSimTimeClock = end - 300;
	}
	if (repeat == 0) {
		step = 4;
	}
	if (command == 1) {
		iSimTimeClock = first;
		if (first < 0) {
			iSimTimeClock = 0;
		}
	} else if (command == 2) {
		iSimTimeClock = end - 300;
		if (iSimTimeClock < first) {
			iSimTimeClock = first;
		}
		if (iSimTimeClock < 0) {
			iSimTimeClock = 0;
		}
	} else if (command == 3) {
		if (step < iSimTimeClock) {
			iSimTimeClock -= step;
		}
		if (iSimTimeClock < first) {
			iSimTimeClock = first;
		}
	} else if (command == 4) {
		if (iSimTimeClock < end - 300 - step) {
			iSimTimeClock += step;
		}
		if (iSimTimeClock < 0) {
			iSimTimeClock = 0;
		}
	}
	if (command >= 1 && command <= 4) {
		g_replay.highlights.tv_camera = 0;
	}
	iSimTimeClock -= iSimTimeClock % 4;
	if (g_is_playing == 1) {
		tnfs_sign_status_save();
	}
	g_is_playing = 4;
	g_replay.highlights.clip = -2;
	g_replay_speed = 4;
	g_replay_speed_skip = 0;
}

/*
 * Replay panel play buttons (Win95 SE FUN_0044f378, DOS 0x6f9bb), not while racing: 6 full speed, 7 half, 8 quarter,
 * 9 eighth (the play button cycles 6, 7, 8: SE table 0x4c5374), 5 pause / one frame step (repeat 0: two simulation
 * ticks, one control step). After scrubbing (mode 4) it cues the replay from the shown clock and loops from there.
 */
void tnfs_replay_speed_command(int command, int repeat) {
	if (g_is_playing == 1) {
		return;
	}
	if (command == 6) {
		g_replay_speed = 4;
		g_replay_speed_skip = 0;
	} else if (command == 7) {
		g_replay_speed = 3;
		g_replay_speed_skip = 2;
	} else if (command == 9) {
		g_replay_speed = 1;
		g_replay_speed_skip = 8;
	} else if (command == 8) {
		g_replay_speed = 2;
		g_replay_speed_skip = 4;
	} else {
		if (command != 5 || repeat != 0) {
			return;
		}
		if (g_is_playing != 3 && g_replay_frame_step == 0) {
			g_replay_frame_step = 2;
			g_replay_speed = 0;
		}
	}
	if (g_is_playing == 4) {
		g_replay_return_mode = 4;
		g_replay.highlights.return_pending = 1;
		tnfs_replay_seek(iSimTimeClock - iSimTimeClock % 4, tnfs_replay_end_clock());
		g_replay.highlights.tv_camera = 0;
	}
}

/*
 * Stats block of the file, 9 x 0x1d2 (Win95 SE 0x51cc5c; the original writes and reads its live g_stats_data):
 * 0x000 lap times[17], 0x044 split times[85] (not in the port), 0x198 best times, quarter mile, penalties, warnings,
 * 0x1b8 wrecks, 0x1bc finish time, 0x1c0 race distance, 0x1c4 top speed, 0x1c8.. end of race data (not in the port).
 * Port fields are mapped by what they hold: lap_time is the finish time (DOS +0x1bc), prev_lap_time DOS +0x1c0.
 */
void tnfs_replay_stats_save() {
	int i;
	int j;
	unsigned char *r;

	memset(g_replay.stats, 0, sizeof(g_replay.stats));
	for (i = 0; i < 8; i++) {
		r = g_replay.stats[i];
		for (j = 0; j < 17; j++) {
			replay_put32(r + j * 4, g_stats_data[i].lap_timer[j]);
		}
		replay_put32(r + 0x198, g_stats_data[i].best_accel_time_1);
		replay_put32(r + 0x19c, g_stats_data[i].best_accel_time_2);
		replay_put32(r + 0x1a0, g_stats_data[i].best_brake_time_2);
		replay_put32(r + 0x1a4, g_stats_data[i].best_brake_time_1);
		replay_put32(r + 0x1a8, g_stats_data[i].quarter_mile_speed);
		replay_put32(r + 0x1ac, g_stats_data[i].quarter_mile_time);
		replay_put32(r + 0x1b0, g_stats_data[i].penalty_count);
		replay_put32(r + 0x1b4, g_stats_data[i].warning_count);
		replay_put32(r + 0x1b8, g_stats_data[i].field_0x1b8);
		replay_put32(r + 0x1bc, g_stats_data[i].lap_time);
		replay_put32(r + 0x1c0, g_stats_data[i].prev_lap_time);
		replay_put32(r + 0x1c4, g_stats_data[i].top_speed);
	}
}

void tnfs_replay_stats_load() {
	int i;
	int j;
	unsigned char *r;

	for (i = 0; i < 8; i++) {
		r = g_replay.stats[i];
		for (j = 0; j < 17; j++) {
			g_stats_data[i].lap_timer[j] = replay_get32(r + j * 4);
		}
		g_stats_data[i].best_accel_time_1 = replay_get32(r + 0x198);
		g_stats_data[i].best_accel_time_2 = replay_get32(r + 0x19c);
		g_stats_data[i].best_brake_time_2 = replay_get32(r + 0x1a0);
		g_stats_data[i].best_brake_time_1 = replay_get32(r + 0x1a4);
		g_stats_data[i].quarter_mile_speed = replay_get32(r + 0x1a8);
		g_stats_data[i].quarter_mile_time = replay_get32(r + 0x1ac);
		g_stats_data[i].penalty_count = replay_get32(r + 0x1b0);
		g_stats_data[i].warning_count = replay_get32(r + 0x1b4);
		g_stats_data[i].field_0x1b8 = replay_get32(r + 0x1b8);
		g_stats_data[i].lap_time = replay_get32(r + 0x1bc);
		g_stats_data[i].prev_lap_time = replay_get32(r + 0x1c0);
		g_stats_data[i].top_speed = replay_get32(r + 0x1c4);
	}
}

/* common original TNFS functions */

void tnfs_initial_position(tnfs_car_data *car) {
	int iVar1;
	int iVar2;

	iVar1 = (track_data[10].num_lanes & 0xf) * 0x28 + (track_data[10].roadRightMargin << 5);

	iVar2 = g_race_positions[car->car_id2];

	(car->position).x = (iVar1 * -0x100) / 2 + (iVar2 % 2) * iVar1 * 0x100;

	if (((track_data[g_slice_mask & 0x14].num_lanes >> 4 == 2) //
		&& ((track_data[g_slice_mask & 0x14].num_lanes & 0xf) == 1)) //
		&& (iVar1 % 2 == 0)) {
		(car->position).x += iVar1 * -0x100;
	}

	if (g_racer_cars_in_scene < 3) {
		car->track_slice = (iVar2 / 2) * -4 + 0x20;
	} else {
		car->track_slice = iVar2 * -2 + 0x20;
	}

	car->track_center_distance = 0;
	(car->position).z = track_data[car->track_slice & g_slice_mask].pos.z;
}

int tnfs_racer_crossed_finish_line(tnfs_car_data *car) {
	if ((g_number_of_players > 1) //
			&& (car->track_slice < 0xe) //
			&& (car->lap_number == 1) //
			&& (car->position.z < 0x510000)) {
		car->position.z = 0x510000;
		car->speed_local_lat = car->speed_local_lat / 2;
		car->speed_local_lon = car->speed_local_lon / 2;
		car->speed_z = car->speed_z / 2;
		car->speed_x = car->speed_x / 2;
	}
	if (g_is_closed_track == 0) {
		if (car->track_slice < 0xc) {
			if (car->position.z < 0x120000) {
				car->position.z = 0x120000;
			}
			return 1;
		}
		if (car->track_slice > g_road_finish_node) {
			return 2;
		}
	} else {
		if (car->lap_number < 1) {
			return 1;
		}
		if (car->lap_number > g_track_laps) {
			return 2;
		}
	}
	return 0;
}

/*
 * how far a racer is past the finish: slices past the finish node (open road), the slice on the extra lap (closed track)
 * DOS 0x64c27
 */
int tnfs_racer_finish_distance(tnfs_car_data *car) {
	if (g_is_closed_track == 0) {
		if (car->track_slice > g_road_finish_node) {
			return car->track_slice - g_road_finish_node;
		}
	} else if (car->lap_number > g_track_laps) {
		return car->track_slice;
	}
	return 0;
}

extern int g_opponent_skill;
extern int DAT_800f62c0;

void tnfs_racer_clock_lap_time(tnfs_car_data *car) {
	int iVar1;
	int iVar2;

	iVar1 = car->car_id2;
	if (g_stats_data[iVar1].lap_time == 0) {
		iVar2 = tnfs_racer_crossed_finish_line(car);
		if (iVar2 != 0) {
			// start line: human racers only (DOS g_num_of_racers_00153b38, tnfs_game_reset_stats 0x637d8: 1 in time
			// trial, else g_number_of_players)
			if ((iVar2 == 1) && (iVar1 < (g_opponent_skill == 0 ? 1 : g_number_of_players))) {
				iVar2 = car->lap_number;
				g_stats_data[iVar1].lap_timer[iVar2 + 1] = -999;
				g_stats_data[iVar1].lap_time = g_stats_data[iVar1].lap_timer[iVar2 + 1];
				printf("car %d crossed Start Line at %d\n", iVar1, g_stats_data[iVar1].lap_time);
			} else if (iVar2 == 2) {
				iVar2 = car->lap_number;
				g_stats_data[iVar1].lap_timer[iVar2 - 1] = iSimTimeClock + -300;
				g_stats_data[iVar1].lap_time = g_stats_data[iVar1].lap_timer[iVar2 - 1];
				printf("car %d crossed Finish Line at %d\n", iVar1, g_stats_data[iVar1].lap_time);
			}
		}
	}
}

void tnfs_car_local_position_vector(tnfs_car_data *car, int *angle, int *length) {
	int x;
	int y;
	int z;
	int heading;

	x = car->position.x - camera.position.x;
	y = car->position.y - camera.position.y;
	z = car->position.z - camera.position.z;

	heading = camera.orientation.y;

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

int tnfs_track_node_find(tnfs_vec3 *p_position, int *current) {
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

			tracknode1 = &track_data[node & g_slice_mask];
			tracknode2 = &track_data[(node + 1) & g_slice_mask];
			position.x = (tracknode1->pos.x + tracknode2->pos.x) >> 1;
			position.z = (tracknode1->pos.z + tracknode2->pos.z) >> 1;
			dist1 = math_vec3_distance_squared_XZ(&position, p_position);

			tracknode1 = &track_data[(node + 1) & g_slice_mask];
			tracknode2 = &track_data[(node + 2) & g_slice_mask];
			position.x = (tracknode1->pos.x + tracknode2->pos.x) >> 1;
			position.z = (tracknode1->pos.z + tracknode2->pos.z) >> 1;
			dist2 = math_vec3_distance_squared_XZ(&position, p_position);

			if (dist2 < dist1) {
				changed = 1;
				*current = *current + 1;
			} else if (0 < *current || g_is_closed_track) {
				tracknode1 = &track_data[(node - 1) & g_slice_mask];
				tracknode2 = &track_data[node & g_slice_mask];
				position.x = (tracknode1->pos.x + tracknode2->pos.x) >> 1;
				position.z = (tracknode1->pos.z + tracknode2->pos.z) >> 1;
				dist2 = math_vec3_distance_squared_XZ(&position, p_position);

				if (dist2 < dist1) {
					changed = 1;
					*current = *current - 1;
				}
			}
		} while (node != *current);
	}
	if ((*current & ~g_slice_mask) != 0) {
		*current = *current & g_slice_mask;
		changed = 1;
	}
	return changed;
}

int tnfs_track_node_update(tnfs_car_data *car) {
	int changed;
	int node;
	int lastnode;

	node = lastnode = car->track_slice;
	changed = tnfs_track_node_find(&car->position, &node);
	car->track_slice = node;

	if (g_is_closed_track == 0) {
		car->track_slice_lap = car->track_slice;
	} else {
		if ((lastnode > g_road_finish_node - 2) && (node < 2)) {
			g_stats_data[car->car_id2].lap_timer[car->lap_number] = iSimTimeClock - 300;
			car->lap_number++;
			if (car->car_id2 == 0) {
				printf("NEW LAP %d\n", car->lap_number);
			}
		} else if ((node > g_road_finish_node - 2) && (lastnode < 2)) {
			car->lap_number--;
			if (car->car_id2 == 0) {
				printf("UN-LAP %d\n", car->lap_number);
			}
		}
		car->track_slice_lap = (car->lap_number - 1) * g_tri_num_chunks * 4 + car->track_slice;
	}

	return changed;
}

/*
 * #Traffic Speed Factors: Traffic Cars will select one of these 4 multipliers,
 * # and drive at the "Safe Speed" times this multiplier
 */
void tnfs_ai_get_speed_factor(tnfs_car_data *car) {
	int rnd;
	g_lcg_random_nbr = g_lcg_random_mod * g_lcg_random_seed;
	g_lcg_random_mod = g_lcg_random_nbr & 0xffff;
	rnd = (g_lcg_random_nbr & 0xffff00) >> 8 & 3;
	car->collision_data.traffic_speed_factor = g_ai_skill_cfg.traffic_speed_factors[rnd];
}

/*
 * #Lane Slack.  Determines how close the cars drive along the centre line.  Measured in units from the
 * # centre of a lane towards the centre of the road.
 */
void tnfs_ai_get_lane_slack(tnfs_car_data *car) {
	int rnd;
	g_lcg_random_nbr = g_lcg_random_mod * g_lcg_random_seed;
	g_lcg_random_mod = g_lcg_random_nbr & 0xffff;
	rnd = (g_lcg_random_nbr & 0xffff00) >> 8 & 3;
	if ((car->ai_state & 4) == 0) {
		car->lane_slack = g_ai_skill_cfg.lane_slack[rnd];
	} else {
		car->lane_slack = g_ai_opp_data[car->car_id2].lane_slack[rnd];
	}
}

void tnfs_track_update_vectors(tnfs_car_data *car) {
	int node;
	tnfs_vec3 heading;
	tnfs_vec3 wall_normal;
	int iVar9, uVar5;

	// current node
	node = car->track_slice & g_slice_mask;
	car->road_position.x = track_data[node].pos.x;
	car->road_position.y = track_data[node].pos.y;
	car->road_position.z = track_data[node].pos.z;

	wall_normal.x = track_data[node].side_normal_x << 1;
	wall_normal.y = track_data[node].side_normal_y << 1;
	wall_normal.z = track_data[node].side_normal_z << 1;

	// next node vector
	node = (car->track_slice + 1) & g_slice_mask;
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
		g_lcg_random_nbr = g_lcg_random_mod * g_lcg_random_seed;
		g_lcg_random_mod = g_lcg_random_nbr & 0xffff;
		if (iVar9 * ((g_lcg_random_nbr & 0xffff00) >> 8) >> 0x10 == 1) {
			tnfs_ai_get_lane_slack(car);
		}
		// also in PSX
		if (((g_game_settings & 2) != 0) && (DAT_8010d310 == 0)) {
			g_lcg_random_nbr = g_lcg_random_mod * g_lcg_random_seed;
			uVar5 = g_lcg_random_mod & 0xffff00;
			g_lcg_random_mod = g_lcg_random_mod & 0xffff;
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

void tnfs_car_update_center_line(tnfs_car_data *car) {
	int iVar1;
	int iVar2;
	int iVar3;
	iVar3 = track_data[car->track_slice & g_slice_mask].heading;
	iVar1 = math_mul(math_sin_2(iVar3 << 2), track_data[car->track_slice & g_slice_mask].pos.x - (car->position).x);
	iVar2 = math_mul(math_cos_2(iVar3 << 2), track_data[car->track_slice & g_slice_mask].pos.z - (car->position).z);
	car->track_center_distance = -iVar2 - iVar1;
}

/*
 * setup everything
 */
void tnfs_init_sim(char *trifile, char *pbsfile) {
	int i, j;

	g_pbs_file = pbsfile;

	iSimTimeClock = 0; // DOS InitSim 0x40da4
	g_is_playing = 0;
	cheat_crashing_cars = 0;
	g_game_settings = 0;
	sound_flag = 0;

	// DOS tnfs_car_Init_Collision 0x47d6c: AI lane change on the first tick
	g_ai_frame_counter = 1;

	//init track
	tnfs_init_track(trifile);

	// AI skill constants
	g_ai_skill_cfg.opp_desired_ahead = 0x140000;
	g_ai_skill_cfg.cop_warning_time = 400;
	g_ai_skill_cfg.max_player_runways = 2;
	g_ai_skill_cfg.field_028 = 0x3c;
	g_ai_skill_cfg.traffic_density = 0x1f4;
	g_ai_skill_cfg.number_of_traffic_cars = 4;
	g_ai_skill_cfg.traffic_speed_factors[0] = 0x10000;
	g_ai_skill_cfg.traffic_speed_factors[1] = 0xe665;
	g_ai_skill_cfg.traffic_speed_factors[2] = 0xb333;
	g_ai_skill_cfg.traffic_speed_factors[3] = 0x9999;
	g_ai_skill_cfg.opp_desired_speed_c = math_mul(0x960000, 0x471c); //0x29aa68;
	g_ai_skill_cfg.traffic_base_speed = 0xDE38D;
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

	// "player first moves" highlight done (DOS _DAT_00153b18, cleared in tnfs_game_reset_stats 0x637d8)
	DAT_800f62c0 = 0;
	for (i = 0; i < 8; i++) {
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
		g_stats_data[i].lap_time = 0;
		g_stats_data[i].top_speed = 0;
		g_stats_data[i].top_speed_2 = 0;
		for (j = 0; j < 17; j++) {
			g_stats_data[i].lap_timer[j] = 0;
		}
	}

	// init player car
	tnfs_init_car();

	// create AI car(s)
	if (g_is_closed_track) {
		g_number_of_traffic_cars = 0;
		g_number_of_cops = 0;
		g_racer_cars_in_scene = 7;
		g_total_cars_in_scene = 7;
	} else {
		g_number_of_cops = 1;
		g_number_of_traffic_cars = 4;
		g_racer_cars_in_scene = 2;
		g_total_cars_in_scene = 7;
	}
	tnfs_ai_init();

	// smoke particles (DOS 0x71ba8, SE 0x456a7c, minus the sprite loading)
	for (i = 0; i < 100; i++) {
		g_smoke_particles.draw_order[i] = i;
		g_smoke_particles.age[i] = 0xffff0000;
	}
	tnfs_smoke_particles_reset(&g_smoke_particles);

	tnfs_camera_init();

	// replay buffers (DOS tnfs_car_init_Mobile_InitCarData 0x6f28e), then the start of the simulation
	tnfs_replay_init();
	tnfs_sim_start();
}


/*
 * game loop, one simulation tick (DOS tnfs_game_loop_000650cb, Win95 SE tnfs_game_loop_00440ef0); the per car part is
 * the port's own (see tnfs_update)
 */
void tnfs_game_loop() {
	int i;
	tnfs_car_data *car;

	if (g_is_playing != 0) {
		tnfs_replay_update();
	}
	if (iSimTimeClock < 0x78) {
		for (i = 0; i < g_number_of_players; i++) {
			g_control_word[i] = 0x7a0;
		}
	}
	if (g_is_playing == 4 || (g_is_playing == 2 && g_replay_speed == 0 && g_replay_frame_step < 1)) {
		// scrubbing or paused: no simulation (the original only keeps the cars' last positions for the renderer)
		return;
	}

	// control step: the control handlers of the cars (players: tnfs_controls_update; AI: empty)
	if (general_flags & 4) {
		for (i = 0; i < g_total_cars_in_scene; i++) {
			car = g_car_ptr_array[i];
			if ((car->field_4e9 & 4) != 0 && car->car_id < g_number_of_players) {
				tnfs_controls_update(car);
			}
		}
	}

	if ((general_flags & 1) == 0) {
		return;
	}

	player_car_ptr->car_road_speed = tnfs_car_road_speed(player_car_ptr);

	// race times, while racing only (DOS tnfs_ai_finish_line_cross 0x64c8e), for the first g_num_of_racers_00153b34
	// cars (DOS 0x153b34 from tnfs_game_reset_stats 0x637d8: 1 in time trial, 2 head-to-head, else all racers)
	if (g_is_playing == 1) {
		for (i = 0; i < (g_opponent_skill == 0 ? 1 : (g_opponent_skill == 1 ? 2 : g_racer_cars_in_scene)); i++) {
			car = g_car_ptr_array[i];
			if ((car->field_4e9 & 4) != 0) {
				tnfs_racer_clock_lap_time(car);
			}
		}
	}

	// for each car
	for (i = 0; i < g_total_cars_in_scene; i++) {
		car = g_car_ptr_array[i];

		if ((car->field_4e9 & 4) == 0) {
			//disabled car
			continue;
		}

		if (car->crash_state == 6) {
			//hidden traffic car
			tnfs_ai_hidden_traffic_main(car);
			continue;
		}

		if (car->crash_state == 1) {
			tnfs_car_wait_after_busted(car);
		} else if (car->crash_state == 5) {
			tnfs_ai_wrecked_wait(car);
		} else if (car->crash_state != 4) {
			if (i < g_number_of_players) {
				// no matrix here: a driving player's matrix is the bare angles, rebuilt every 4th tick by
				// tnfs_ai_update_speed_targets (DOS 0x77d1c, PSX 0x80053d98, SE 0x46aba4); the body lean is added
				// only when drawing (DOS gfx_draw_car 0x74d73, PSX 0x80050584, SE 0x4591d4)
				tnfs_driving_main(car);
			} else {
				tnfs_ai_driving_main(car);
			}
		} else {
			tnfs_collision_main(car);
		}

		tnfs_car_update_center_line(car);
	}

	// DOS order: race order, collisions between the cars, race order again
	FUN_00064e46();
	tnfs_ai_collision_handler();
	FUN_00064e46();

	tnfs_camera_auto_change(player_car_ptr);
	tnfs_camera_update(&camera);
}

/*
 * main loop step (DOS tnfs_game_loop_00040c90, Win95 SE tnfs_game_loop_0045597c), called at 30 per second:
 * the clock counts 2 per tick (60 per second, the unit of the race times and of the replay), the flags are
 * 1 every tick, 4 every second tick (control step: controls, replay samples, gearbox), 0x10 every fourth (ABS/TCS).
 * The clock stops while a replay is scrubbed or paused; a frame step (g_replay_frame_step) runs while paused.
 */
void tnfs_update() {
	replay_set_flags();
	tnfs_game_loop();
	if (g_is_playing != 4 && (g_is_playing != 2 || g_replay_speed != 0 || g_replay_frame_step > 0)) {
		if (g_replay_frame_step > 0) {
			g_replay_frame_step--;
		}
		iSimTimeClock += 2;
	}
}

/*
 * Starts the race simulation after the setup (DOS tnfs_game_InitSim 0x40da4 "Starting Engine", Win95 SE 0x455a5c):
 * clock 0, all flags on, three game loop ticks without a replay mode (control words neutral) and without
 * advancing the clock, then recording (mode 1) with a keyframe on the first control step.
 * The first tick places the AI cars, which start with track_slice_lap 0 (tnfs_ai_init): racers on their grid slots,
 * with their road vectors, before the first recorded tick (race times count in mode 1 only). tnfs_init_sim ends with
 * it; a front end that sets the cars up again after tnfs_init_sim (tnfs_ai_init) calls it again afterwards.
 */
void tnfs_sim_start() {
	iSimTimeClock = 0;
	general_flags = 0xffffffff;
	g_is_playing = 0;
	tnfs_game_loop();
	tnfs_game_loop();
	tnfs_game_loop();
	g_is_playing = 1;
	replay_put32(g_replay.world + REPLAY_WORLD_COUNTDOWN, 0);
}
