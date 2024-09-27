/*
 * tnfs_base.h
 * w/ original comments from 3DO spec files
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

	/*
	 * #front drive percentage (amount of drive going to front tires) (.5 is 4 wheel drive
	 * #	0.0 is rear wheel drive)
	 */
	int front_drive_percentage; //0x018
	/*
	 * #front brake percentage (amount of brake force going to front tires) - normally 70% (0.7).
	 */
	int front_brake_percentage; //0x01C

	//  ...
	int centre_of_gravity_height; //0x024
	int max_brake_force_1; //0x028
	int max_brake_force_2; //0x02C
	//  ...

	/*
	 * #drag - 1/2 * density of air( 1.2kg/m3) * drag coefficent (.33) * frontal area (20sq feet=1.85806)
	 */
	int drag; //0x034

	int top_speed; //0x038
	int efficiency; //0x03c
	int wheelbase; //0x040

	/*
	 * # WST new  burnOutDiv..  reduce lateral acc if accel burnouts
	 */
	int burnOutDiv; //0x044

	int wheeltrack; //0x048
	//  ...

	/*
	 * #mps to rpm conversion factor: //WST checked Viper 7th aug 94
	 * #From R & T:
	 * # speed @ 1200 rpm in 6th gear: 60.0 mph = 26.79
	 * #conversion=rpm/speed/gear=1200/26.79/0.5=
	 */
	int mps_to_rpm_factor; //0x54
	int number_of_gears; //0x58
	int final_drive; //0x05c

	/*
	 * #rolling radius (metres) (lessoned a bit to compensate for tire deformation
	 * #this decreases our torque,  and increases our reversed engineered rpm.
	 */
	int wheel_roll_radius; //0x6c;

	int inverse_wheel_radius; //0x64;
	int gear_ratio_table[8]; //0x68
	//  ...
	int torque_table_entries; //0x088
	//  ...

	/*
	 * #Front roll stiffness - how many degrees does the car tilt for a given torque
	 * # given in N*m/mdeg (ie. torque/angle) in metric degrees
	 * # Quote "The suspension with the highest roll stiffness will receive the largest
	 * # 			portion of weight transfer caused by body roll"
	 */
	int front_roll_stiffness; //0x8c
	int rear_roll_stiffness; //0x90

	/*
	 * #Roll Axis Height - Height of axis through which the car rolls during cornering
	 * # this is usually a direct consequence of the suspension geometry/type, etc...
	 */
	int roll_axis_height; //0x94

	int weight_transfer_factor; //unused

	/*
	 * #cutoff-slip angles greater than this are assumed to
	 * # be equal to this value.  As slip angles become too great
	 * # our keeno-formula starts sliping below 0 (not good)
	 * # when in reality they usually level out. (say at 45 degrees)
	 */
	int cutoff_slip_angle; //0xa4

	int normal_coeff_loss; //unused
	int rpm_redline; //0xac
	int rpm_idle; //0xb0
	unsigned int torque_table[120]; //0xb4
	int gear_upshift_rpm[7]; //0x294
	int gear_efficiency[8]; //0x2b0

	/*
	 * # Wei's new fiziks inertia factor.
	 */
	int inertia_factor; //0x2D0
	/*
	 * # Wei's new g force to body roll factor.
	 */
	int body_roll_factor; //0x2D4
	/*
	 * # Wei's new g force to body pitch factor.
	 */
	int body_pitch_factor; //0x2D8
	/*
	 * # Wei's new front friction multiplier factor.
	 */
	int front_friction_factor; //0x2DC
	/*
	 * # Wei's new rear friction multiplier factor.
	 */
	int rear_friction_factor; //0x2E0

	int body_length; //0x2E4
	int body_width; //0x2E8
	//  ...

	/*
	 * # WST new lateral acc cutoff multiplier (in m/s/s)
	 */
	int lateral_accel_cutoff; //0x31C

	/// ...
	int final_drive_torque_ratio; //0x328
	int thrust_to_acc_factor; //0x32c
	int throttle_on_ramp; //unused
	int throttle_off_ramp; //unused
	int brake_on_ramp_1; //unused
	int brake_on_ramp_2; //unused
	int brake_off_ramp_1; //unused
	int brake_off_ramp_2; //unused
	int shift_timer; //0x354
	int noGasRpmDec; //0x358
	int gasRpmInc; //0x35C
	int clutchDropRpmDec; //0x360
	int clutchDropRpmInc; //0x364
	int negTorque; //0x368

	/*
	 * #WST Ride height
	 */
	int ride_height; //0x36c
	/*
	 * #and centre y for BRAD
	 */
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
	int mass; //0x60
	int moment_of_inertia; //0x64
	int linear_acc_factor; //0x68
	int angular_acc_factor; //0x6c
	int edge_length; //0x70
	tnfs_vec3 size; //0x74
	int state_timer; //0x80
	int field_084; //0x84
	int field_088; //0x88
	int field_08c; //0x8c
	int traffic_speed_factor; //0x90
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
	int road_segment_a; //0x048 "track slice"
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

	int field_158; //0x158 random group index
	int car_road_speed; //0x15c
	int speed_target; //0x160
	int target_center_line; //0x164
	int field_168;
	int field_16c;
	int field_170;
	int ai_state; //0x174
	int power_curve[100]; //0x178
	// ...

	/*
	 * #top speeds per gear (mph) must be 6 figures, if less than 6 gears set first to 0
	 */
	int top_speed_per_gear[6]; //0x308

	int ai_gear_ratios[6]; //0x320

	int pdn_max_rpm; //0x338
	int lane_slack; //0x33c
	// ...
	int angle_dy; //0x354
	// ...
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
	int pdn_number_of_gears; //0x46c
	struct tnfs_car_specs *car_specs_ptr; //0x471
	int car_id2; //0x475
	// ...
	int car_model_id; //0x48d //0 TSupra, 1 LDiabl, 2 P911, 3 CZR1, 4 F512TR, 5 DVIPER, 6 ANSX, 7 MRX7 .. 11 Traffc
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
	int field_4e9; //0x4e9 0x528 //0-parked police //4-enable swerve, 5/6/7-enable collision
} tnfs_car_data;

typedef struct tnfs_track_data {
	unsigned char roadLeftMargin; // 0
	unsigned char roadRightMargin; // 1
	unsigned char roadLeftFence; // 2
	unsigned char roadRightFence; // 3
	unsigned char num_lanes; // 4
	unsigned char fence_flag; // 5
	unsigned char shoulder_surface_type; // 6
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

typedef struct tnfs_track_speed {
	char top_speed;
	char legal_speed;
	char safe_speed;
} tnfs_track_speed;

typedef struct tnfs_ai_skill_cfg {
	// ...

	/*
	 * #dpjl 94.07.11: desired speed ahead
	 * # desired speed at least = players speed + desired speed ahead
	 */
	int opp_desired_ahead; //0x18

	/*
	 * #player must pull over before this time to get warning and not ticket
	 * #cop_warning_time: (ticks)
	 */
	int cop_warning_time; //0x1c

	/*
	 * #If player runs away from cops (doesn't pull over) more times than this
	 * #when (if) they are finally caught they will get arrested (go to jail
	 * # don't pass go)
	 */
	int max_player_runways; //0x20
	// ...

	/*
	 * #Traffic Density (larger number creates less density)
	 */
	int traffic_density; //0x30

	/*
	 * #Number of Traffic Cars
	 */
	int number_of_traffic_cars; //0x34

	/*
	 * #Traffic Speed Factors: Traffic Cars will select one of these 4 multipliers,
	 * # and drive at the "Safe Speed" times this multiplier
	 */
	int traffic_speed_factors[4]; //0x38

	/*
	 * #dpjl 94.07.11: desired speed constant of opponent (c) such that:
	 * #desired speed = c/2 + players speed/2
	 * #opp_desired_speed_c:
	 */
	int opp_desired_speed_c; //0x48

	/*
	 * #Lane Slack.  Determines how close the cars drive along the centre line.  Measured in units from the
	 * # centre of a lane towards the centre of the road.
	 */
	int lane_slack[4]; //0x4c

	/*
	 * #Opponent Glue:
	 */
	int opponent_glue_0[21]; //0x5c
	int opponent_glue_1[21]; //0xb0
	int opponent_glue_2[21]; //0x104
	int opponent_glue_3[21]; //0x158
} tnfs_ai_skill_cfg;

typedef struct tnfs_ai_opp_data {
	int id; //0x0
	int opponent_glue_factors[21]; //0x4
	int field_0x55; //0x55
	int field_0x59; //0x59

	/*
	 * #dpjl 94.07.07: how far the opponent needs to be from you before he starts
	 * # to try to block you (s.slices)
	 */
	int opp_block_look_behind; //0x5d

	int opp_block_behind_distance; //0x61

	/*
	 * #dpjl 94.07.08: how fast the opponent changes lanes
	 */
	int opp_lane_change_speeds; //0x65


	int field_0x69; //0x69

	/*
	 * #dpjl 94.07.08 Limit how far ahead opponents look ahead for oncoming traffic
	 */
	int opp_oncoming_look_ahead; //0x6d

	/*
	 * #dpjl 94.07.10ish Opponent will swerve into oncoming lane on tight curves
	 * #left lane merit = oncoming_corner_swerve[skill]*delta yaw
	 */
	int opp_oncoming_corner_swerve; //0x71

	/*
	 * #dpjl 94.07.10: Opponent will cut corners (go into opposing lanes) on CCW turns
	 */
	int opp_cut_corners; //0x75

	int lane_slack[4];
	int field_0x89; //0x89
} tnfs_ai_opp_data;

typedef struct tnfs_stats_data {
	int lap_timer[17]; //0x0
	// ...
	int best_accel_time_1; //0x198
	int best_accel_time_2; //0x19c
	int best_brake_time_1; //0x1a0
	int best_brake_time_2; //0x1a4
	int quarter_mile_speed; //0x1a8
	int quarter_mile_time; //0x1ac
	int penalty_count; //0x1b0
	int warning_count; //0x1b4
	int field_0x1b8; //0x1b8
	// ...
	int prev_lap_time; //0x1bc
	// ...
	int field412_0x1c0; //0x1c0
	int top_speed; //0x1c4
	int field_0x1c8; //0x1c8
} tnfs_stats_data;

typedef struct tnfs_random_struct {
	short id; // 0
	short a;  // 1
	short b;  // 2
	short c;  // 3
	short d;  // 4
	short e;  // 5
} tnfs_random_struct;

typedef struct tnfs_camera {
	int id; //0 in_car; 1 tail cam; 2 chase cam
	tnfs_vec3 position;
	int car_id;
} tnfs_camera;

// global variables
extern struct tnfs_track_data track_data[2400];
extern struct tnfs_surface_type road_surface_type_array[3];
extern struct tnfs_track_speed g_track_speed[600]; // 000FDB8C road speed limit array

extern struct tnfs_car_specs car_specs;
extern struct tnfs_car_data g_car_array[8];
extern tnfs_car_data *g_car_ptr_array[8];
extern tnfs_car_data *player_car_ptr;
extern tnfs_ai_skill_cfg *g_ai_opp_data_ptr[8];
extern tnfs_ai_skill_cfg g_ai_skill_cfg;
extern tnfs_ai_opp_data g_ai_opp_data[8];
extern tnfs_stats_data g_stats_data[8];

extern int g_total_cars_in_scene;
extern int g_racer_cars_in_scene;
extern int g_number_of_cops; //DAT_8010d1d0
extern int g_number_of_traffic_cars;

extern int g_police_on_chase;
extern char is_drifting;
extern int g_game_time;
extern int road_segment_count;
extern int sound_flag;
extern int cheat_crashing_cars;
extern int g_game_settings;
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
extern int g_police_speeding_ticket;
extern int DAT_00165148;
extern int g_police_chase_time;
extern int DAT_0016707C;


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
int tnfs_car_road_speed_2(tnfs_car_data *car);
int tnfs_car_road_speed(tnfs_car_data *car);
void tnfs_change_camera();
void tnfs_change_gear_up();
void tnfs_change_gear_down();
void tnfs_change_traction();
void tnfs_change_transmission_type();
void tnfs_abs();
void tnfs_tcs();
void tnfs_cheat_mode();
void tnfs_ai_get_speed_factor(tnfs_car_data *car);
void tnfs_ai_get_lane_slack(tnfs_car_data *car);

#endif /* TNFS_BASE_H_ */
