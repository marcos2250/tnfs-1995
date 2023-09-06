/*
 * tnfs_collision.h
 */

#ifndef TNFS_COLLISION_H_
#define TNFS_COLLISION_H_

void tnfs_collision_main(tnfs_car_data *car);

void tnfs_collision_rollover_start(tnfs_car_data *car, int force_z, int force_y, int force_x);

#endif /* TNFS_COLLISION_H_ */
