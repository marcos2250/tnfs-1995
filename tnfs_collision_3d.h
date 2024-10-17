/*
 * tnfs_collision_3d.h
 */

#ifndef TNFS_COLLISION_3D_H_
#define TNFS_COLLISION_H_

void tnfs_collision_main(tnfs_car_data *car);
void tnfs_collision_data_get(tnfs_car_data *car, int crash_state);
void tnfs_collision_data_set(tnfs_car_data *car);
void tnfs_collision_rollover_start(tnfs_car_data *car, int force_z, int force_y, int force_x);
int tnfs_collision_carcar(tnfs_car_data *car1, tnfs_car_data *car2);
void tnfs_collision_off();
void tnfs_collision_on();

#endif /* TNFS_COLLISION_3D_H_ */
