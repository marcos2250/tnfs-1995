/*
 * tnfs_fiziks.h
 */

#ifndef TNFS_FIZIKS_H_
#define TNFS_FIZIKS_H_

void tnfs_driving_main(tnfs_car_data * car);
void tnfs_driving_checkpoint_flick(tnfs_car_data *car);
void tnfs_height_position(tnfs_car_data *car, int is_driving_mode);
void tnfs_load_torque_table(tnfs_car_specs *specs, int is_automatic);

#endif /* TNFS_FIZIKS_H_ */
