/*
 * tnfs_base.h
 */
#ifndef TNFS_BASE_H_
#define TNFS_BASE_H_

/*
 * Uncompressed PBS car specs file
 */
typedef struct tnfs_car_specs {
	int mass_front; //0x000
	int mass_rear; //0x004
	int mass_total; //0x008
	int inverse_mass_front; //0x00C
	int inverse_mass_rear; //0x010
	int inverse_mass; //0x014
	int front_drive_percentage; //0x018
	int front_brake_percentage; //0x01C
	//  ...
	int centre_of_gravity_height; //0x024
	int max_brake_force_1; //0x028
	int max_brake_force_2; //0x02C
	//  ...
	int drag; //0x034
	int top_speed; //0x038
	int efficiency; //0x03c
	int wheelbase; //0x040
	int burnOutDiv; //0x044
	int wheeltrack; //0x048
	//  ...
	int mps_to_rpm_factor; //0x54
	int number_of_gears; //0x58
	int final_drive; //0x05c
	int wheel_roll_radius; //0x6c;
	int inverse_wheel_radius; //0x64;
	int gear_ratio_table[8]; //0x68
	//  ...
	int torque_table_entries; //0x088
	//  ...
	int front_roll_stiffness; //0x8c
	int rear_roll_stiffness; //0x90
	int roll_axis_height; //0x94
	// ...
	int cutoff_slip_angle; //0xa4
	// ...
	int rpm_redline; //0xac
	int rpm_idle; //0xb0
	unsigned int torque_table[120]; //0xb4
	//  ...
	int gear_upshift_rpm[7]; //0x294
	int gear_efficiency[8]; //0x2b0
	int inertia_factor; //0x2D0
	int body_roll_factor; //0x2D4
	int body_pitch_factor; //0x2D8
	int front_friction_factor; //0x2DC
	int rear_friction_factor; //0x2E0
	int body_length; //0x2E4
	int body_width; //0x2E8
	//  ...
	int lateral_accel_cutoff; //0x31C
	/// ...
	int final_drive_torque_ratio; //0x328
	int thrust_to_acc_factor; //0x32c
	//  ...
	int shift_timer; //0x354
	int noGasRpmDec; //0x358
	int gasRpmInc; //0x35C
	int clutchDropRpmDec; //0x360
	int clutchDropRpmInc; //0x364
	int negTorque; //0x368
	int ride_height; //0x36c
	int centre_y; //0x370
	unsigned char grip_table[1024]; //0x374
} tnfs_car_specs;

typedef struct {
	tnfs_vec9 matrix; //0
	tnfs_vec3 position; //0x24
	tnfs_vec3 speed; //0x30
	tnfs_vec3 angular_speed; //0x3c
	tnfs_vec3 field4_0x48;
	// ...
	int field6_0x60; //0x60
	// ...
	int linear_acc_factor; //0x68
	int angular_acc_factor; //0x6c
	int edge_length; //0x70
	tnfs_vec3 size; //0x74
	int crash_time_ai_state; //0x80
	// ...
	int field_088; //0x88
	int field_08c; //0x8c
	int field16_0x90; //0x90
	// ...
} tnfs_collision_data;

typedef struct tnfs_car_data {
	tnfs_vec3 position; //0x000
	int angle_x; //0x00C
	int angle_y; //0x010
	int angle_z; //0x014
	int steer_angle; //0x018
	int target_angle; //0x1c
	int is_crashed; //0x20
	tnfs_vec9 matrix; //0x24
	int road_segment_a; //0x048
	int road_segment_b; //0x04C
	int lap_number; //0x050
	int speed_x; //0x054
	int speed_y; //0x058
	int speed_z; //0x05C
	int speed_local_lat; //0x060
	int speed_local_vert; //0x064
	int speed_local_lon; //0x068
	int speed_drivetrain; //0x06C
	int speed; //0x070
	int angular_speed; //0x074
	int car_length; //0x078
	int car_width; //0x07C
	int center_line_distance; //0x80
	int side_width; //0x84
	struct tnfs_car_data * car_data_ptr; //0x088
	tnfs_vec3 road_fence_normal; //0x08C
	tnfs_vec3 road_surface_normal;
	tnfs_vec3 road_heading;
	tnfs_vec3 road_position;
	// ...
	int collision_height_offset;
	tnfs_collision_data collision_data;

	int car_road_speed; //0x15c
	int speed_target; //0x160
	int target_center_line; //0x164
	// ...
	int ai_state; //0x174
	int power_curve[100]; //0x178
	// ...
	int field_33c; //0x33c
	// ...
	int angle_dx; //0x354
	int angle_dy; //0x358
	tnfs_vec3 world_position; //0x35c
	int body_roll; //0x365
	int body_pitch; //0x369
	tnfs_vec3 front_edge;
	tnfs_vec3 side_edge;
	tnfs_vec3 road_ground_position;
	// ...
	int throttle; //0x3B1
	int throttle_previous_pos; //0x3B5
	int brake; //0x3B9
	int is_shifting_gears; //0x3BD
	short rpm_engine; //0x3C1
	short rpm_vehicle; //0x3C2
	short rpm_redline; //0x3C2
	int road_grip_increment; //0x3C9
	int tire_grip_rear; //0x3CD
	int tire_grip_front; //0x3D1
	// ...
	int slope_force_lat; //0x3D9
	int unknown_flag_3DD; //0x3DD
	int slope_force_lon; //0x3E1
	int thrust; //0x3E5
	int gear_auto_selected; //0x3E9
	int gear_selected; //0x3ED
	int is_gear_engaged; //0x3F1
	int handbrake; //0x3F5
	int is_engine_cutoff; //0x3F9
	char wheels_on_ground;
	int time_off_ground; //0x401
	int slide_front; //0x405
	int slide_rear; //0x409
	int tire_grip_loss; //0x40D
	int accel_lat; //0x411
	int accel_lon; //0x415
	int gear_shift_current; //0x419
	int gear_shift_previous; //0x41D
	int gear_shift_interval; //0x421
	char tire_skid_front; //0x425
	char tire_skid_rear; //0x426
	// ...
	char collision_a; //collision flags added in PSX version
	char collision_b;
	int collision_x;
	int collision_y;
	// ...
	int weight_distribution_front; //0x435
	int weight_distribution_rear; //0x439
	int mass_front;
	int mass_rear;
	int weight_transfer_factor; //0x445
	int rear_friction_factor; //0x449
	int front_friction_factor; //0x44D
	int wheel_base; //0x451
	int moment_of_inertia;
	int front_yaw_factor; //0x459
	int rear_yaw_factor; //0x45D
	// ...
	int field_461; //0x461
	// ...
	struct tnfs_car_specs *car_specs_ptr; //0x471
	int car_id2; //0x475
	// ...
	int unknown_0x498;
	int tcs_enabled; //0x491
	int abs_enabled; //0x495
	// ...
	int drag_const_0x4a8; //0x4a8
	int drag_const_0x4aa; //0x4aa
	short drag_const_0x4ac; //0x4ac
	short drag_const_0x4ae; //0x4ae
	int surface_type; //0x49D
	int surface_type_b; //0x4A1
	// ...
	int tcs_on; //0x4AD
	int abs_on; //0x4B1
	// ...
	int is_wrecked;
	// ...
	int delta_time; //0x4D5
	int fps; //0x4D9
	// ...

	int crash_state; //0x4e1 0x520 // 2-normal/player 3-normal/opponent 4-wrecked 6-inactive
	int car_id; //0x4e5 0x524 //car id number 0..7
	int field_4e9; //0x4e9 0x528 //4-player 7-opponent
} tnfs_car_data;


typedef struct tnfs_track_data {
	unsigned char roadLeftMargin; // 0
	unsigned char roadRightMargin; // 1
	unsigned char roadLeftFence; // 2
	unsigned char roadRightFence; // 3
	unsigned char num_lanes; // 4
	unsigned char fence_flag; // 5
	unsigned char verge_slide; // 6
	unsigned char item_mode; // 7
	tnfs_vec3 pos; // 8
	short slope; // 20
	short slant; // 22
	short heading; // 24
	// blank
	short side_normal_x; // 28
	short side_normal_y; // 30
	short side_normal_z; // 32
	// blank
	// added for renderer
	vector3f vf_margin_L;
	vector3f vf_margin_R;
	vector3f vf_fence_L;
	vector3f vf_fence_R;
} tnfs_track_data;

typedef struct tnfs_surface_type {
	int roadFriction;
	int velocity_drag;
	int surface_drag;
	int is_unpaved;
} tnfs_surface_type;

typedef struct tnfs_speed_presets {
	char ai_speed_1;
	char traffic_speed_limit;
	char ai_speed_2;
} tnfs_track_speed;

typedef struct tnfs_traffic_cfg {
	int field_0x8;
	int field_0x79;
	int field_0x65;
	int field_0x69;
	int field_0x6d;
} tnfs_traffic_cfg;

typedef struct tnfs_unk_struct {
	int DAT_001654f4;
	int DAT_001654f8;
	int DAT_00165500;
	int DAT_00165504;
	int DAT_00165508;
} tnfs_unk_struct;

typedef struct tnfs_camera {
	int id; //0 in_car; 1 tail cam; 2 chase cam
	tnfs_vec3 position;
} tnfs_camera;

// global variables
extern struct tnfs_car_specs car_specs;
extern struct tnfs_track_data track_data[2400];
extern struct tnfs_surface_type road_surface_type_array[3];
extern struct tnfs_speed_presets g_track_speed[900]; // 000FDB8C road speed limit array

extern struct tnfs_car_data g_car_array[8];
extern tnfs_car_data *g_car_ptr_array[8];
extern tnfs_car_data *player_car_ptr;
extern tnfs_traffic_cfg *g_traffic_cfg_ptr[8];
extern tnfs_unk_struct g_unk_struct[8];

extern int g_total_cars_in_scene;
extern int g_racer_cars_in_scene;

extern char is_drifting;
extern int g_game_time;
extern int road_segment_count;
extern int sound_flag;
extern int cheat_crashing_cars;
extern int cheat_code_8010d1c4;
extern char g_control_throttle;
extern char g_control_brake;
extern signed char g_control_steer;
extern int g_number_of_players;
extern int g_selected_cheat;
extern int selected_camera;
extern tnfs_camera camera;


extern int DAT_000F9BB0;
extern int DAT_000f99e4;
extern int DAT_000f99e8;
extern int DAT_000f99ec; //800eae14
extern int DAT_000f99f0;
extern int DAT_000FDB94;
extern int DAT_000FDCEC;
extern int DAT_000FDCF0;
extern int DAT_000f9A70;
extern int DAT_001039d4; //800db6bc
extern int DAT_001039d8; //800db6c0
extern int DAT_001039dc;
extern int DAT_00144914;
extern int DAT_00143844;
extern int DAT_0014DCC4;
extern int DAT_0014dccc; // segment id mask
extern int DAT_00153B20;
extern int DAT_00153B24;
extern tnfs_car_data * DAT_00153BC4;
extern int DAT_001651c0[4];
extern int DAT_0016513C;
extern int DAT_00165144;
extern int DAT_00165148;
extern int DAT_00165190;
extern int DAT_00165320;
extern tnfs_car_data *DAT_00165334;
extern int DAT_0016533C;
extern int DAT_00165340;
extern int DAT_0016707C;
extern int DAT_001670B3; //DAT_8010d1d0
extern int DAT_001670BB;


// common functions
void tnfs_init_sim(char * trifile);
void tnfs_replay_highlight_record(char a);
void tnfs_camera_set(tnfs_camera * camera, int id);
void tnfs_reset_car(tnfs_car_data *car);
void tnfs_update();
void tnfs_crash_car();
void tnfs_sfx_play(int a, int b, int c, int d, int e, int f);
void tnfs_car_local_position_vector(tnfs_car_data * car_data, int * angle, int * length);
int tnfs_road_segment_find(tnfs_car_data *car_data, int *current);
int tnfs_road_segment_update(tnfs_car_data *car);
void tnfs_track_update_vectors(tnfs_car_data *car);
int tnfs_car_road_speed_collided(tnfs_car_data *car);
int tnfs_car_road_speed(tnfs_car_data *car);
void tnfs_change_camera();
void tnfs_change_gear_up();
void tnfs_change_gear_down();
void tnfs_change_traction();
void tnfs_change_transmission_type();
void tnfs_abs();
void tnfs_tcs();
void tnfs_cheat_mode();

#endif /* TNFS_BASE_H_ */
