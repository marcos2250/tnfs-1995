/*
 * tnfs_engine.h
 */

#ifndef TNFS_ENGINE_H_
#define TNFS_ENGINE_H_

extern void tnfs_engine_rev_limiter(tnfs_car_data *car);
extern int tnfs_engine_thrust(tnfs_car_data *car);
extern void tnfs_engine_auto_shift_control(tnfs_car_data *car);

#endif /* TNFS_ENGINE_H_ */
