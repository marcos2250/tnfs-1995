/*
 * tnfs_camera.h
 */

#ifndef TNFS_CAMERA_H_
#define TNFS_CAMERA_H_

void tnfs_camera_init();
void tnfs_camera_set(tnfs_camera * camera, int id);
void tnfs_camera_update(tnfs_camera *cam);
void tnfs_camera_auto_change(tnfs_car_data *car);

#endif /* TNFS_CAMERA_H_ */
