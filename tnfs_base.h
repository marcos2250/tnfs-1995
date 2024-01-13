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


/*
 * Uncompressed PBS car specs file
 */
typedef struct tnfs_car_specs {
	int mass_front; //0x000
	int mass_rear; //0x000
	int mass_total; //0x000
	//  ...
	int unknown_const; //0x014
	int front_drive_percentage; //0x018
	int front_brake_percentage; //0x01C
	//  ...
	int centre_of_gravity_height;
	int max_brake_force_1; //0x028
	int max_brake_force_2; //0x02C
	//  ...
	int drag; //0x034
	int top_speed; //0x038
	//  ...
	int wheelbase; //0x040
	int burnOutDiv; //0x044
	int wheeltrack; //0x048
	//  ...
	int mps_to_rpm_factor; //0x054
	int number_of_gears;
	//  ...
	int gear_ratio_table[8]; //0x070
	//  ...
	int torque_table_entries; //0x088
	//  ...
	int front_roll_stiffness;
	int rear_roll_stiffness;
	int roll_axis_height;
	// ...
	int cutoff_slip_angle; //0x0a4
	// ...
	int rpm_redline; //0x0ac
	int rpm_idle; //0x0b0
	unsigned int torque_table[120]; //0x0b4
	//  ...
	int gear_upshift_rpm[7]; //0x290
	int gear_efficiency[8]; //0x2B8
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
	int garRpmInc; //0x35C
	int clutchDropRpmDec; //0x360
	int clutchDropRpmInc; //0x364
	int negTorque; //0x368
	int ride_height; //0x36c
	int centre_y; //0x370
	unsigned char grip_table[1024]; //0x374
} tnfs_car_specs;

typedef struct tnfs_car_data {
	tnfs_vec3 position; //0x000
	int angle_x; //0x00C
	int angle_y; //0x010
	int angle_z; //0x014
	int steer_angle; //0x018
	// ...
	int is_crashed;
	tnfs_vec9 matrix;
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
	// ...
	struct tnfs_car_data * car_data_ptr; //0x088
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
	int body_roll; //0x365
	int body_pitch; //0x369
	tnfs_vec3 road_ground_position;
	tnfs_vec3 front_edge;
	tnfs_vec3 side_edge;
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
	// ...
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
	int slide; //0x40D
	int susp_incl_lat; //0x411
	int susp_incl_lon; //0x415
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
	// ...
	int weight_transfer_factor; //0x445
	int rear_friction_factor; //0x449
	int front_friction_factor; //0x44D
	int wheel_base; //0x451
	// ...
	int front_yaw_factor; //0x459
	int rear_yaw_factor; //0x45D
	// ...
	struct tnfs_car_specs *car_specs_ptr; //0x471
	int unknown_flag_475; //0x475
	int unknown_flag_479; //0x479
	int unknown_flag_480; //0x480
	// ...
	int tcs_enabled; //0x491
	int abs_enabled; //0x495
	// ...
	int surface_type; //0x49D
	int surface_type_b; //0x4A1
	//..
	int tcs_on; //0x4AD
	int abs_on; //0x4B1
	// ...
	int delta_time; //0x4D5
	int fps; //0x4D9
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

typedef struct tnfs_surface_type {
	int friction_factor;
	int drag_factor;
	int add_drag;
	int is_not_asphalt;
};

// global variables
extern struct tnfs_car_specs car_specs;
extern struct tnfs_car_data car_data;
extern struct tnfs_track_data track_data[2400];
extern struct tnfs_surface_type road_surface_type_array[3];

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
