/*
 * tnfs_file.c
 * Readers for TNFS files
 */
#include <stdio.h>
#include "tnfs_math.h"
#include "tnfs_base.h"

int readFixed32(unsigned char *buffer, int pos) {
	return (int)(buffer[pos + 3]) << 24 //
		| (int)(buffer[pos + 2]) << 16 //
		| (int)(buffer[pos + 1]) << 8 //
		| buffer[pos];
}

int readSigned16(unsigned char *buffer, int pos) {
	return (signed short)(buffer[pos + 1]) << 8 | buffer[pos];
}

/*
 * Import a TNFSSE TRI track file
 */
int read_tri_file(char * file) {
	unsigned char buffer[36];
	FILE *ptr;
	int i;

	ptr = fopen(file,"rb");
	if (!ptr) {
		printf("File not found: %s\n", file);
		return 0;
	}

	road_segment_count = 0;
	for (i = 0; i < 2400; i++) {
		fseek(ptr, i * 36 + 2444, SEEK_SET);
		fread(buffer, 36, 1, ptr);

		track_data[i].roadLeftMargin = buffer[0];
		track_data[i].roadRightMargin = buffer[1];
		track_data[i].roadLeftFence = buffer[2];
		track_data[i].roadRightFence = buffer[3];

		track_data[i].num_lanes = buffer[4];
		track_data[i].fence_flag = buffer[5];
		track_data[i].verge_slide = buffer[6];
		track_data[i].item_mode = buffer[7];

		track_data[i].pos.x = readFixed32(buffer, 8);
		track_data[i].pos.y = readFixed32(buffer, 12);
		track_data[i].pos.z = readFixed32(buffer, 16);

		// 14-bit angles: unsigned 0 to 0x4000 (360)
		track_data[i].slope = readSigned16(buffer, 20);
		track_data[i].slant = readSigned16(buffer, 22);
		track_data[i].heading = readSigned16(buffer, 24);

		// 16-bit normals: signed -32768 to +32768
		track_data[i].segment_cos = readSigned16(buffer, 28);
		track_data[i].segment_tan = readSigned16(buffer, 30);
		track_data[i].segment_sin = readSigned16(buffer, 32);

		if (i > 0 && track_data[i].pos.x == 0 && track_data[i].pos.y == 0 && track_data[i].pos.z == 0) {
			break;
		}
		road_segment_count++;
	}
	fclose(ptr);
	printf("Loaded track %s with %d segments.\n", file, road_segment_count);
	return 1;
}

/*
 * Import an uncompressed PBS file
 */
int read_pbs_file(char * file) {
	unsigned char buffer[2048];
	FILE *ptr;
	int i;

	ptr = fopen(file,"rb");
	if (!ptr) {
		printf("File not found: %s\n", file);
		return 0;
	}

	fread(buffer, 2048, 1, ptr);

	car_specs.mass_front = readFixed32(buffer, 0);
	car_specs.mass_rear = readFixed32(buffer, 4);
	car_specs.mass_total = readFixed32(buffer, 8);
	car_specs.inverse_mass_front = readFixed32(buffer, 0xC);
	car_specs.inverse_mass_rear = readFixed32(buffer, 0x10);
	car_specs.inverse_mass = readFixed32(buffer, 0x14);
	car_specs.front_drive_percentage = readFixed32(buffer, 0x18);
	car_specs.front_brake_percentage = readFixed32(buffer, 0x1c);
	car_specs.centre_of_gravity_height = readFixed32(buffer, 0x24);
	car_specs.max_brake_force_1 = readFixed32(buffer, 0x28);
	car_specs.max_brake_force_2 = readFixed32(buffer, 0x2c);
	car_specs.drag = readFixed32(buffer, 0x34);
	car_specs.top_speed = readFixed32(buffer, 0x38);
	car_specs.efficiency = readFixed32(buffer, 0x3c);
	car_specs.wheelbase = readFixed32(buffer, 0x40);
	car_specs.burnOutDiv = readFixed32(buffer, 0x44);
	car_specs.wheeltrack = readFixed32(buffer, 0x48);
	car_specs.mps_to_rpm_factor = readFixed32(buffer, 0x54);
	car_specs.number_of_gears = readFixed32(buffer, 0x58);
	car_specs.final_drive = readFixed32(buffer, 0x5c);
	car_specs.inverse_wheel_radius = readFixed32(buffer, 0x64);

	for (i = 0; i < 8; i++) {
		car_specs.gear_ratio_table[i] = readFixed32(buffer, i * 4 + 0x68);
	}

	car_specs.torque_table_entries = readFixed32(buffer, 0x88);
	car_specs.front_roll_stiffness = readFixed32(buffer, 0x8c);
	car_specs.rear_roll_stiffness = readFixed32(buffer, 0x90);
	car_specs.roll_axis_height = readFixed32(buffer, 0x94);
	car_specs.cutoff_slip_angle = readFixed32(buffer, 0xa4);
	car_specs.rpm_redline = readFixed32(buffer, 0xac);
	car_specs.rpm_idle = readFixed32(buffer, 0xb0);

	for (i = 0; i < (car_specs.torque_table_entries * 2); i++) {
		car_specs.torque_table[i] = readFixed32(buffer, i * 4 + 0xb4);
	}
	for (i = 0; i < 7; i++) {
		car_specs.gear_upshift_rpm[i] = readFixed32(buffer, i * 4 + 0x294);
	}
	for (i = 0; i < 8; i++) {
		car_specs.gear_efficiency[i] = readFixed32(buffer, i * 4 + 0x2b0);
	}

	car_specs.inertia_factor = readFixed32(buffer, 0x2d0);
	car_specs.body_roll_factor = readFixed32(buffer, 0x2d4);
	car_specs.body_pitch_factor = readFixed32(buffer, 0x2d8);
	car_specs.front_friction_factor = readFixed32(buffer, 0x2dc);
	car_specs.rear_friction_factor = readFixed32(buffer, 0x2e0);
	car_specs.body_length = readFixed32(buffer, 0x2e4);
	car_specs.body_width = readFixed32(buffer, 0x2e8);
	car_specs.lateral_accel_cutoff = readFixed32(buffer, 0x31c);
	car_specs.final_drive_torque_ratio = readFixed32(buffer, 0x328);
	car_specs.thrust_to_acc_factor = readFixed32(buffer, 0x32c);
	car_specs.shift_timer = readFixed32(buffer, 0x354);
	car_specs.noGasRpmDec = readFixed32(buffer, 0x358);
	car_specs.garRpmInc = readFixed32(buffer, 0x35c);
	car_specs.clutchDropRpmDec = readFixed32(buffer, 0x360);
	car_specs.clutchDropRpmInc = readFixed32(buffer, 0x364);
	car_specs.negTorque = readFixed32(buffer, 0x368);
	car_specs.ride_height = readFixed32(buffer, 0x36c);
	car_specs.centre_y = readFixed32(buffer, 0x370);

	memcpy(car_specs.grip_table, &buffer[0x374], 1024);

	fclose(ptr);
	printf("Loaded car file %s.\n", file);
	return 1;
}
