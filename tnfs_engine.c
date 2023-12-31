/*
 * tnfs_engine.c
 * Engine, transmission and clutch simulation
 */
#include "tnfs_math.h"
#include "tnfs_base.h"

void tnfs_engine_rev_limiter(tnfs_car_data *car) {
	tnfs_car_specs *specs;
	int max_rpm;

	specs = car->car_specs_ptr;

	if (car->is_engine_cutoff) {
		car->throttle = 0;
	}

	// speed to RPM
	car->rpm_vehicle = fixmul(fixmul(specs->gear_ratio_table[car->gear_selected + 2], specs->final_drive_speed_ratio), car->speed_drivetrain) >> 16;

	if (car->rpm_vehicle < 700)
		car->rpm_vehicle = 700;

	max_rpm = car->throttle * specs->rpm_redline >> 8;

	if (car->is_gear_engaged) {
		if (car->rpm_vehicle > car->rpm_engine) {
			if (car->rpm_vehicle + 500 <= car->rpm_engine) {
				car->rpm_engine += fixmul(g_gear_ratios[car->gear_selected], specs->rev_clutch_drop_rpm_inc);
			} else {
				car->rpm_engine += specs->rev_clutch_drop_rpm_inc;
			}
			if (car->rpm_engine > car->rpm_vehicle)
				car->rpm_engine = car->rpm_vehicle;
		} else {
			if ((car->tire_skid_rear & 2) && car->throttle > 220) {
				car->rpm_engine -= fix3(specs->rev_clutch_drop_rpm_dec);
			} else {
				car->rpm_engine -= specs->rev_clutch_drop_rpm_dec;
			}
			if (car->rpm_engine < car->rpm_vehicle)
				car->rpm_engine = car->rpm_vehicle;
		}
	} else if (car->throttle >= 13) {
		if (car->gear_selected == -1) {
			car->rpm_engine += car->car_specs_ptr->rev_speed_gas_inc * car->throttle / 2 >> 8;
		} else {
			car->rpm_engine += car->car_specs_ptr->rev_speed_gas_inc * car->throttle >> 8;
		}
		if (car->rpm_engine > max_rpm)
			car->rpm_engine = max_rpm;
	} else if (specs->rpm_idle > car->rpm_engine) {
		if (car->gear_selected == -1) {
			car->rpm_engine += car->car_specs_ptr->rev_speed_no_gas >> 1;
		} else {
			car->rpm_engine += car->car_specs_ptr->rev_speed_no_gas;
		}
		if (car->rpm_engine > specs->rpm_idle) {
			car->rpm_engine = specs->rpm_idle;
		}
	} else {
		if (car->gear_selected == -1) {
			car->rpm_engine -= car->car_specs_ptr->rev_speed_no_gas >> 1;
		} else {
			car->rpm_engine -= car->car_specs_ptr->rev_speed_no_gas >> 1;
		}
		if (car->rpm_engine < specs->rpm_idle) {
			car->rpm_engine = specs->rpm_idle;
		}
	}

	if (car->gear_auto_selected != 0 && car->gear_selected == 0 && car->rpm_engine < specs->rpm_idle) {
		car->rpm_engine = specs->rpm_idle;
	}
	if (car->rpm_engine > 11000) {
		car->rpm_engine = 11000;
		car->handbrake = 1;
	}
	if (car->rpm_vehicle > 11000) {
		car->rpm_vehicle = 11000;
	}
}

void tnfs_engine_auto_shift_change(tnfs_car_data *car_data, tnfs_car_specs *car_specs) {
	int gear;
	int rpm_vehicle;

	gear = car_data->gear_selected;

	// speed to RPM
	rpm_vehicle = fixmul(fixmul(car_specs->gear_ratio_table[gear + 2], car_specs->final_drive_speed_ratio), car_data->speed_local_lon) >> 16;

	if (gear < car_specs->number_of_gears && rpm_vehicle > car_specs->gear_upshift_rpm[gear]) {
		// upshift
		car_data->gear_selected++;
	} else if (gear > 0 && rpm_vehicle < car_specs->gear_upshift_rpm[gear] / 2) {
		// downshift
		car_data->gear_selected--;
	}
}

void tnfs_engine_auto_shift_control(tnfs_car_data *car) {
	int gear;
	if (car->is_shifting_gears > 0) {
		car->is_shifting_gears--;
	}
	if (car->is_shifting_gears == 0) {
		car->throttle = car->throttle_previous_pos;
		car->is_engine_cutoff = 0;
		car->is_gear_engaged = 1;
		car->is_shifting_gears = -1;
	}
	if (car->is_shifting_gears < 0) {
		gear = car->gear_selected;
		tnfs_engine_auto_shift_change(car, car->car_specs_ptr);
		if (car->gear_selected != gear) {
			car->throttle_previous_pos = car->throttle;
			car->is_engine_cutoff = 1;
			car->is_gear_engaged = 0;
			if (car->unknown_flag_479 == 4 && (g_game_time & 0x31) == 16) {
				car->is_shifting_gears = car->car_specs_ptr->gear_shift_delay + 3;
			} else {
				car->is_shifting_gears = car->car_specs_ptr->gear_shift_delay;
			}
		}
	}
}

int tnfs_engine_torque(tnfs_car_specs *specs, int rpm) {
	int offset;

	//offset = (((rpm - (rpm + 100) % 200) + 100) - specs->torque_table[0]) / 200;
	if (rpm < specs->torque_table[0]) {
		offset = 0;
	} else {
		offset = (rpm - specs->torque_table[0]) / 200;
	}

	if (offset < 0) {
		offset = 0;
	}
	if (offset >= specs->torque_table_entries) {
		offset = specs->torque_table_entries - 1;
	}
	return specs->torque_table[offset * 2 + 1];
}

int tnfs_engine_thrust(tnfs_car_data *car) {
	int torque;
	int thrust;
	int max_rpm;
	int tireslip;
	int gear;
	tnfs_car_specs *specs;

	specs = car->car_specs_ptr;

	gear = car->gear_selected + 2;

	if (car->is_gear_engaged) {
		//if (car->rpm_engine >= car->rpm_vehicle) {
		if (car->throttle > 0) { //FIXME
			// acceleration
			max_rpm = car->throttle * specs->rpm_redline >> 8;

			if (car->rpm_engine <= max_rpm) {
				// engine rpm range
				torque = tnfs_engine_torque(specs, car->rpm_engine);
				torque = fixmul(torque, specs->gear_ratio_table[gear]);
				torque = torque * specs->gear_torque_table[gear] >> 8;

				if (torque > 0x70000)
					torque = 0x70000;

				tireslip = abs((car->rpm_engine << 16) / (fixmul(specs->gear_ratio_table[gear], specs->final_drive_speed_ratio) >> 16) - car->speed_local_lon);

				if (tireslip > 0x30000 && car->throttle > 250 && car->gear_selected == 0) {
					// engine overpower, doing burnouts
					// guessed thrust => 1.25 * tire slip speed
					thrust = tireslip + fix2(tireslip);
					is_drifting = 1;
				} else {
					thrust = (car->throttle * torque) >> 8;
				}

			} else {
				// out of rpm range
				torque = abs((car->rpm_engine - max_rpm) * specs->negative_torque);
				thrust = abs(fixmul(specs->gear_ratio_table[gear], torque));

				if (abs(car->speed_local_lon) * 16 >= thrust) {
					if (car->speed_local_lon == 0) {
						thrust = 0;
					} else if (car->speed_local_lon < 0) {
						thrust = -thrust;
					}
				} else {
					thrust = car->speed_local_lon * -16;
				}
			}

		} else {
			// decceleration
			//thrust = specs->gear_ratio_table[gear] * (car->rpm_vehicle - car->rpm_engine) * specs->negative_torque * -8;
			thrust = specs->gear_ratio_table[gear] * specs->negative_torque * -0x10000;
		}
	} else {
		// neutral
		thrust = 0;
	}

	// final ratio
	thrust = fix2(thrust * (specs->final_drive_torque_ratio >> 6));

	if ((car->throttle > 0xf0) // full throttle
			&& (abs(thrust) > car->tire_grip_rear - car->slide) // tire grip slipping
			&& (car->rpm_engine < car->car_specs_ptr->gear_upshift_rpm[0] - 500) // before cut off
			&& (car->car_specs_ptr->front_drive_percentage == 0)) { // RWD car

		// RPM to speed
		car->speed_drivetrain = 0x100000000 / fixmul(specs->gear_ratio_table[gear], specs->final_drive_speed_ratio) * car->rpm_engine;

		// wheel spin faster than car speed
		if (car->speed_drivetrain > abs(car->speed_local_lon)) {
			return thrust;
		}
	}

	car->speed_drivetrain = car->speed_local_lon;
	return thrust;
}

