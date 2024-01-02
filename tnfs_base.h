/*
 * tnfs_base.h
 */
#ifndef TNFS_BASE_H_
#define TNFS_BASE_H_

typedef struct {
	tnfs_vec9 matrix; //0
	tnfs_vec3 position; //24
	tnfs_vec3 speed; //30
	tnfs_vec3 field4_0x48;
	// ...
	int linear_acc_factor;
	int angular_acc_factor;
	// ...
	int field6_0x60;
	// ...
	tnfs_vec3 size; //74
	int crashed_time; //80
	// ...
	tnfs_vec3 angular_speed;
} tnfs_collision_data;

typedef struct tnfs_car_specs {
	//  ...
	int front_drive_percentage; //00000018
	int front_brake_percentage; //0000001C
	//  ...
	int max_brake_force_1; //00000028
	int max_brake_force_2; //0000002C
	//  ...
	int unknown_const_drag; //00000034
	int max_speed; //00000038
	//  ...
	int final_drive_speed_ratio; //00000054
	int number_of_gears;
	//  ...
	int gear_ratio_table[10]; //00000070
	int gear_efficiency[10];
	//  ...
	int torque_table_entries; //0x88
	//  ...
	int max_slip_angle; //000000a4
	// ...
	int rpm_redline; //000000ac
	int rpm_idle; //000000b0
	unsigned int torque_table[120]; //000000b4
	//  ...
	int gear_upshift_rpm[10]; // 0x290
	//  ...
	int body_roll_factor; //000002D4
	int body_pitch_factor; //000002D8
	//  ...
	int max_tire_lateral_force; //0000031C
	//  ...
	int final_drive_torque_ratio; //00000328
	int thrust_to_acc_factor; //0000032C
	//  ...
	int gear_shift_delay; //00000354
	int rev_speed_no_gas; //00000358
	int rev_speed_gas_inc; //0000035C
	int rev_clutch_drop_rpm_dec; //00000360
	int rev_clutch_drop_rpm_inc; //00000364
	int negative_torque; //00000368
	// ...
	unsigned char grip_table[1024]; //00000374
} tnfs_car_specs;

typedef struct tnfs_car_data {
	tnfs_vec3 position; //00000000
	int angle_x; //0000000C
	int angle_y; //00000010
	int angle_z; //00000014
	int steer_angle; //00000018
	// ...
	int is_crashed;
	tnfs_vec9 matrix;
	int road_segment_a; //00000048
	int road_segment_b; //0000004C
	int lap_number; //00000050
	int speed_x; //00000054
	int speed_y; //00000058
	int speed_z; //0000005C
	int speed_local_lat; //00000060
	int speed_local_vert; //00000064
	int speed_local_lon; //00000068
	int speed_drivetrain; //0000006C
	int speed; //00000070
	int angular_speed; //00000074
	int car_length; //00000078
	int car_width; //0000007C
	// ...
	struct tnfs_car_data * car_data_ptr; //00000088
	tnfs_vec3 road_fence_normal;
	tnfs_vec3 road_surface_normal;
	tnfs_vec3 road_heading;
	tnfs_vec3 road_position;
	// ...
	int collision_height_offset;
	tnfs_collision_data collision_data;
	int field203_0x174;
	// ...
	int angle_dx;
	// ...
	tnfs_vec3 world_position;
	int body_roll; //00000365
	int body_pitch; //00000369
	tnfs_vec3 road_ground_position;
	tnfs_vec3 front_edge;
	tnfs_vec3 side_edge;
	// ...
	int throttle; //000003B1
	int throttle_previous_pos; //000003B5
	int brake; //000003B9
	int is_shifting_gears; //000003BD
	short rpm_engine; //000003C1
	short rpm_vehicle; //000003C2
	short rpm_redline; //000003C2
	int road_grip_increment; //000003C9
	int tire_grip_rear; //000003CD
	int tire_grip_front; //000003D1
	// ...
	int slope_force_lat; //000003D9
	int unknown_flag_3DD; //000003DD
	// ...
	int slope_force_lon; //000003E1
	int thrust; //000003E5
	int gear_auto_selected; //000003E9
	int gear_selected; //000003ED
	int is_gear_engaged; //000003F1
	int handbrake; //000003F5
	int is_engine_cutoff; //000003F9
	char wheels_on_ground;
	int time_off_ground; //00000401
	int slide_front; //00000405
	int slide_rear; //00000409
	int slide; //0000040D
	int susp_incl_lat; //00000411
	int susp_incl_lon; //00000415
	int gear_shift_current; //00000419
	int gear_shift_previous; //0000041D
	int gear_shift_interval; //00000421
	char tire_skid_front; //00000425
	char tire_skid_rear; //00000426
	// ...
	char collision_a; //collision flags added in PSX version
	char collision_b;
	int collision_x;
	int collision_y;
	// ...
	int weight_distribution; //00000435
	// ...
	int weight_transfer_factor; //00000445
	int rear_friction_factor; //00000449
	int front_friction_factor; //0000044D
	int wheel_base; //00000451
	int wheel_track; //00000455
	int front_yaw_factor; //00000459
	int rear_yaw_factor; //0000045D
	// ...
	struct tnfs_car_specs *car_specs_ptr; //00000471
	int unknown_flag_475; //00000475
	int unknown_flag_479; //00000479
	int unknown_flag_480; //00000480
	// ...
	int tcs_enabled; //00000491
	int abs_enabled; //00000495
	// ...
	int surface_type_a; //0000049D
	int surface_type_b; //000004A1
	//..
	int tcs_on; //000004AD
	int abs_on; //000004B1
	// ...
	int scale_a; //000004D5
	int scale_b; //000004D9
	//..
	int is_wrecked;
	int field444_0x520;
	int field445_0x524;
} tnfs_car_data;


typedef struct tnfs_track_data {
	// ...
	tnfs_vec3 pos;
	int slope;
	int slant;
	int heading;
	tnfs_vec3 side_point;
	tnfs_vec3 wall_normal;
	// ...
	int roadLeftMargin;
	int roadRightMargin;
	int roadLeftFence;
	int roadRightFence;
	//...
	// added
	vector3f vf_margin_L;
	vector3f vf_margin_R;
	vector3f vf_fence_L;
	vector3f vf_fence_R;
} tnfs_track_data;

// global variables
extern struct tnfs_car_specs car_specs;
extern struct tnfs_car_data car_data;
extern struct tnfs_track_data track_data[2400];
extern int road_surface_type_array[10];

extern char is_drifting;
extern int g_game_time;
extern char roadConstantA;
extern char roadConstantB;
extern int road_segment_count;
extern int sound_flag;
extern int cheat_mode;
extern int DAT_8010d1c4;

extern int selected_camera;
extern tnfs_vec3 camera_position;

// common functions
void tnfs_replay_highlight_000502AB(char a);
void tnfs_init_track();
void tnfs_reset_car();
void tnfs_update();
void tnfs_crash_car();
void tnfs_sfx_play(int a, int b, int c, int d, int e, int f);
void tnfs_car_size_vector(tnfs_car_data * car_data, int * angle, int * length);
int tnfs_road_segment_find(tnfs_car_data *car_data, int *current);
int tnfs_road_segment_update(tnfs_car_data *car);
void tnfs_track_update_vectors(tnfs_car_data *car);
void tnfs_change_camera();
void tnfs_change_gear_up();
void tnfs_change_gear_down();
void tnfs_change_traction();
void tnfs_change_transmission_type();
void tnfs_abs();
void tnfs_tcs();
void tnfs_cheat_mode();

#endif /* TNFS_BASE_H_ */
