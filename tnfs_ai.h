/*
 * tnfs_ai.h
 */

#ifndef TNFS_AI_H_
#define TNFS_AI_H_


extern int g_is_playing;
void tnfs_ai_init();
void tnfs_ai_driving_main(tnfs_car_data * car);
void tnfs_ai_collision_handler();
void tnfs_ai_respawn_main(tnfs_car_data *car);
void tnfs_ai_police_reset_state(int flag);
void tnfs_ai_hidden_traffic_main(tnfs_car_data *car);
void tnfs_ai_respawn_0007d647();
void tnfs_car_wait_after_busted(tnfs_car_data *car);
int tnfs_ai_car_in_slice_window(tnfs_car_data *car1, tnfs_car_data *car2);
int tnfs_ai_car_near_player(tnfs_car_data *car);
void tnfs_ai_wrecked_wait(tnfs_car_data *car);
int tnfs_ai_lane_table(int lanes, int margin);

#endif /* TNFS_AI_H_ */
