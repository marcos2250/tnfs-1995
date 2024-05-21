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
	car->rpm_vehicle = fixmul(fixmul(specs->gear_ratio_table[car->gear_selected + 2], specs->mps_to_rpm_factor), car->speed_drivetrain) >> 16;

	if (car->rpm_vehicle < 700)
		car->rpm_vehicle = 700;

	max_rpm = specs->rpm_redline * car->throttle >> 8;

	if (car->is_gear_engaged) {
		if (car->rpm_vehicle > car->rpm_engine) {
			if (car->rpm_vehicle + 500 <= car->rpm_engine) {
				car->rpm_engine += fix8(car->car_specs_ptr->gear_ratio_table[car->gear_selected] * specs->clutchDropRpmInc);
			} else {
				car->rpm_engine += specs->clutchDropRpmInc;
			}
			if (car->rpm_engine > car->rpm_vehicle)
				car->rpm_engine = car->rpm_vehicle;
		} else {
			if ((car->tire_skid_rear & 2) && car->throttle > 220) {
				car->rpm_engine -= fix3(specs->clutchDropRpmDec);
			} else {
				car->rpm_engine -= specs->clutchDropRpmDec;
			}
			if (car->rpm_engine < car->rpm_vehicle)
				car->rpm_engine = car->rpm_vehicle;
		}
	} else if (car->throttle >= 13) {
		if (car->gear_selected == -1) {
			car->rpm_engine += car->car_specs_ptr->gasRpmInc * car->throttle / 2 >> 8;
		} else {
			car->rpm_engine += car->car_specs_ptr->gasRpmInc * car->throttle >> 8;
		}
		if (car->rpm_engine > max_rpm)
			car->rpm_engine = max_rpm;
	} else if (specs->rpm_idle > car->rpm_engine) {
		if (car->gear_selected == -1) {
			car->rpm_engine += car->car_specs_ptr->noGasRpmDec >> 1;
		} else {
			car->rpm_engine += car->car_specs_ptr->noGasRpmDec;
		}
		if (car->rpm_engine > specs->rpm_idle) {
			car->rpm_engine = specs->rpm_idle;
		}
	} else {
		if (car->gear_selected == -1) {
			car->rpm_engine -= car->car_specs_ptr->noGasRpmDec >> 1;
		} else {
			car->rpm_engine -= car->car_specs_ptr->noGasRpmDec >> 1;
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
	rpm_vehicle = fixmul(fixmul(car_specs->gear_ratio_table[gear + 2], car_specs->mps_to_rpm_factor), car_data->speed_local_lon) >> 16;

	if (gear < (car_specs->number_of_gears - 3) && rpm_vehicle > car_specs->gear_upshift_rpm[gear]) {
		// upshift
		car_data->gear_selected++;
	} else if (gear > 0 && rpm_vehicle < car_specs->gear_upshift_rpm[gear - 1] / 2) {
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
			if (car->unknown_0x498 == 4 && (g_game_time & 0x31) == 16) {
				car->is_shifting_gears = car->car_specs_ptr->shift_timer + 3;
			} else {
				car->is_shifting_gears = car->car_specs_ptr->shift_timer;
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
		offset = (rpm - specs->torque_table[0]) / 400;
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
		if (car->rpm_engine >= car->rpm_vehicle) {
			// acceleration
			max_rpm = car->throttle * specs->rpm_redline >> 8;

			if (car->rpm_engine <= max_rpm) {
				// engine rpm range
				torque = tnfs_engine_torque(specs, car->rpm_engine);
				torque = fixmul(torque, specs->gear_ratio_table[gear]);
				torque = torque * specs->gear_efficiency[gear] >> 8;

				if (torque > 0x70000)
					torque = 0x70000;

				tireslip = abs((car->rpm_engine << 16) / (fixmul(specs->gear_ratio_table[gear], specs->mps_to_rpm_factor) >> 16) - car->speed_local_lon);

				if (tireslip > 0x30000 && car->throttle > 250 && car->gear_selected == 0) {
					// engine overpower, doing burnouts
					// guessed thrust => 1.25 * tire slip speed
					thrust = tireslip + fix2(tireslip);
					is_drifting = 1;
				} else {
					thrust = (car->throttle * torque) >> 8;
				}

			} else {
				// decceleration
				torque = abs((car->rpm_engine - max_rpm) * specs->negTorque);
				thrust = -fixmul(specs->gear_ratio_table[gear], torque);

				if (car->speed_local_lon == 0) {
					thrust = 0;
				} else if (abs(thrust) > abs(car->speed_local_lon) * 16) {
					thrust = car->speed_local_lon * -16;
				} else if (car->speed_local_lon < 0) {
					thrust = -thrust;
				}
			}
		} else {
			// decceleration
			thrust = fixmul(specs->gear_ratio_table[gear],  specs->negTorque) * (car->rpm_vehicle - car->rpm_engine) * -8;
		}
	} else {
		// neutral
		thrust = 0;
	}

	// final ratio
	thrust = fix2(thrust * fix6(specs->final_drive_torque_ratio));

	// rally mode
	if (((cheat_code_8010d1c4 & 0x20) != 0) // rally mode enabled
			&& (car->throttle > 0xfa) // full throttle
			&& (car->gear_selected > 0) // forward gear
			&& (car->speed_local_lon < 0x190000)) { // low speeds
		thrust = thrust << 1;
	}

	// tire slip
	if ((car->throttle > 0xf0) // full throttle
			&& (abs(thrust) > car->tire_grip_rear - car->tire_grip_loss) // tire grip slipping
			&& (car->rpm_engine < car->car_specs_ptr->gear_upshift_rpm[0] - 500)) { // before cut off
			//&& (car->car_specs_ptr->front_drive_percentage == 0)) { // RWD car ??

		// RPM to speed
		car->speed_drivetrain = 0x100000000 / fixmul(specs->gear_ratio_table[gear], specs->mps_to_rpm_factor) * car->rpm_engine;

		// wheel spin faster than car speed
		if (car->speed_drivetrain > abs(car->speed_local_lon)) {
			return thrust;
		}
	}

	car->speed_drivetrain = car->speed_local_lon;
	return thrust;
}

