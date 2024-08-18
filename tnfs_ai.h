/*
 * tnfs_ai.h
 */

#ifndef TNFS_AI_H_
#define TNFS_AI_H_


void tnfs_ai_init();
void tnfs_ai_driving_main(tnfs_car_data * car);
void tnfs_ai_collision_handler();
void tnfs_ai_respawn_main(tnfs_car_data *car);

#endif /* TNFS_AI_H_ */
