/*
 * tnfs_ai.h
 */

#ifndef TNFS_AI_H_
#define TNFS_AI_H_


void tnfs_ai_init();
void tnfs_ai_driving_main(tnfs_car_data * car);
void tnfs_ai_collision_handler();
void tnfs_ai_respawn_main(tnfs_car_data *car);
void tnfs_ai_police_reset_state(int flag);
void tnfs_ai_hidden_traffic_main(tnfs_car_data *car);
void tnfs_ai_respawn_0007d647();

#endif /* TNFS_AI_H_ */
