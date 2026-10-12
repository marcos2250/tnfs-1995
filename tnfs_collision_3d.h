/*
 * tnfs_collision_3d.h
 */

#ifndef TNFS_COLLISION_3D_H_
#define TNFS_COLLISION_H_

extern tnfs_smoke_particles g_smoke_particles;

void tnfs_smoke_particles_reset(tnfs_smoke_particles *smoke);
void tnfs_smoke_particles_emit(tnfs_smoke_particles *smoke, tnfs_car_data *car, int amount);
int tnfs_smoke_particles_update(tnfs_smoke_particles *smoke, int delta_time);
void tnfs_collision_main(tnfs_car_data *car);
void tnfs_collision_data_get(tnfs_car_data *car, int crash_state);
void tnfs_collision_data_set(tnfs_car_data *car);
void tnfs_collision_recover_car(tnfs_car_data *car);
void tnfs_collision_align_up_vector(tnfs_collision_data *body, tnfs_vec3 *up);
void tnfs_collision_rollover_start(tnfs_car_data *car, int force_z, int force_y, int force_x);
int tnfs_collision_carcar(tnfs_car_data *car1, tnfs_car_data *car2);
void tnfs_collision_off();
void tnfs_collision_on();

#endif /* TNFS_COLLISION_3D_H_ */
