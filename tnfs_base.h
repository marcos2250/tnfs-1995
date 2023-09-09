/*
 * tnfs_base.h
 *
 *  Created on: 6 de set. de 2023
 *      Author: m3
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
	int max_speed; //00000038
	//  ...
	int diff_ratio; //00000054
	//  ...
	int gear_ratio_table; //00000070
	//  ...
	int max_slip_angle; //000000a4
	int rpm_redline; //000000ac
	int rpm_idle; //000000b0
	int torque_table_r01; //000000b4
	int torque_table_t01; //000000b8
	//  ...
	int torque_table_r10; //000000fc
	int torque_table_t10; //00000100
	//  ...
	int gear_ratio_table_2; //000002b8
	//  ...
	int body_roll_factor; //000002D4
	int body_pitch_factor; //000002D8
	//  ...
	int max_tire_lateral_force; //0000031C
	//  ...
	int thrust_to_acc_factor; //0000032C
	//  ...
	int diff_ratio_2; //00000368
	// ...
	unsigned char slide_table[1024]; //00000374
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
	int game_status; //00000050
	int speed_x; //00000054
	int speed_y; //00000058
	int speed_z; //0000005C
	int speed_local_lat; //00000060
	int speed_local_vert; //00000064
	int speed_local_lon; //00000068
	int speed_drivetrain; //0000006C
	int speed; //00000070
	int angular_speed; //00000074
	// ...
	int * unknown_const_88; //00000088
	tnfs_vec3 road_fence_normal;
	tnfs_vec3 road_normal;
	tnfs_vec3 road_heading;
	tnfs_vec3 road_position;

	// ...
	int collision_height_offset;
	tnfs_collision_data collision_data;
	int field203_0x174;
	// ...
	int body_roll; //00000365
	int body_pitch; //00000369
	// ...
	int throttle; //000003B1
	// ...
	int brake; //000003B9
	int is_shifting_gears; //000003BD
	short rpm_engine; //000003C1
	short rpm_idle; //000003C2
	int road_grip_increment; //000003C9
	int tire_grip_rear; //000003CD
	int tire_grip_front; //000003D1
	// ...
	int slope_force_lat; //000003D9
	// ...
	int slope_force_lon; //000003E1
	int thrust; //000003E5
	int gear_RND; //000003E9
	int gear_speed; //000003ED
	// ...
	int handbrake; //000003F5
	// ...
	int wheels_on_ground; //00000401
	int slide_front; //00000405
	int slide_rear; //00000409
	int slide; //0000040D
	int susp_incl_lat; //00000411
	int susp_incl_lon; //00000415
	int gear_speed_selected; //00000419
	int gear_speed_previous; //0000041D
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
	//...
	int car_flag_0x480;
	// ...
	int tcs_enabled; //00000491
	int abs_enabled; //00000495
	// ...
	int surface_type; //0000049D
	// ...
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


// global variables
extern struct tnfs_car_data car_data;
extern struct tnfs_car_specs car_specs;
extern tnfs_vec3 camera_position;
extern int selected_camera;
extern int cheat_mode;
extern int road_surface_type_array[10];
extern int roadLeftMargin;
extern int roadRightMargin;
extern int roadLeftFence;
extern int roadRightFence;
extern char roadConstantA;
extern char roadConstantB;
extern int road_segment_pos_x;
extern int road_segment_pos_z;
extern int road_segment_slope;
extern int road_segment_heading;
extern int sound_flag;

// common functions
void tnfs_reset();
void tnfs_update();
void tnfs_cheat_crash_cars();
void tnfs_debug_00034309(int a, int b, int x, int y);
void tnfs_debug_000343C2(int a, int b);
void tnfs_sfx_play(int a, int b, int c, int d, int e, int f);
void tnfs_physics_car_vector(tnfs_car_data * car_data, int * angle, int * length);
int tnfs_collision_car_size(tnfs_car_data * car_data, int a);
void tnfs_collision_debug(char a);
void tnfs_change_camera();
void tnfs_change_gear_up();
void tnfs_change_gear_down();
void tnfs_abs();
void tnfs_tcs();
void tnfs_cheat_mode();

#endif /* TNFS_BASE_H_ */
