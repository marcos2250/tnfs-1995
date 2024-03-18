/*
 * tnfs_collision_3d.h
 */

#ifndef TNFS_COLLISION_3D_H_
#define TNFS_COLLISION_H_

void tnfs_collision_main(tnfs_car_data *car);

void tnfs_collision_rollover_start(tnfs_car_data *car, int force_z, int force_y, int force_x);

void tnfs_collision_carcar();

#endif /* TNFS_COLLISION_3D_H_ */
