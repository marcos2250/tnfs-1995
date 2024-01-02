/*
 * tnfs_collision.c
 * 3D Rigid body dynamics
 */
#include "tnfs_math.h"
#include "tnfs_base.h"

// globals
int g_collision_force;
int g_surf_distance;
int g_surf_clipping = 1;
tnfs_vec3 g_collision_speed;
int g_const_7333 = 0x7333;
int g_const_CCCC = 0xCCCC;
int DAT_800eae0c = 0x10000;
int DAT_800eae10 = 0x34000;
int DAT_800eae14 = 10;
int DAT_800eae18 = 0x8000;
tnfs_car_data * tnfs_car_data_ptr = 0;

void math_matrix_create_from_vec3(tnfs_vec9 *result, int amount, tnfs_vec3 *direction) {
	int angle_a;
	int angle_b;
	int h_length;
	tnfs_vec9 mRotZ;
	tnfs_vec9 mRotY;
	tnfs_vec9 mRotX;
	tnfs_vec9 mRotYZ;
	tnfs_vec9 mInv;

	angle_a = math_atan2(direction->x, direction->z);
	math_matrix_set_rot_Y(&mRotY, angle_a);

	h_length = math_vec3_length_XZ(direction);
	angle_b = math_atan2(h_length, direction->y);
	math_matrix_set_rot_Z(&mRotZ, -angle_b);
	math_matrix_multiply(&mRotYZ, &mRotY, &mRotZ);

	math_matrix_set_rot_X(&mRotX, amount);
	math_matrix_transpose(&mInv, &mRotYZ);
	math_matrix_multiply(result, &mRotYZ, &mRotX);
	math_matrix_multiply(result, result, &mInv);
}

void tnfs_collision_rebound(tnfs_collision_data *body, tnfs_vec3 *l_edge, tnfs_vec3 *speed, tnfs_vec3 *normal) {
	tnfs_vec3 cross_prod;
	tnfs_vec3 normal_accel;
	tnfs_vec3 accel;
	tnfs_vec3 accel_edge;
	tnfs_vec3 accel_scale;
	int force;
	int length;
	int rebound, dampening;
	int dampen;
	int aux;
	int iX, iY, iZ;

	accel.x = speed->x;
	accel.y = speed->y;
	accel.z = speed->z;

	// cross product of surface normal and contact point
	cross_prod.x = fixmul(normal->z, l_edge->y) - fixmul(normal->y, l_edge->z);
	cross_prod.y = fixmul(normal->x, l_edge->z) - fixmul(normal->z, l_edge->x);
	cross_prod.z = fixmul(normal->y, l_edge->x) - fixmul(normal->x, l_edge->y);

	rebound = 0;

	length = math_vec3_length_squared(&cross_prod);
	length = (body->linear_acc_factor >> 1) + (math_mul(length, body->angular_acc_factor) >> 1);

	force = fixmul(normal->x, body->speed.x) //
			+ fixmul(normal->y, body->speed.y) //
			+ fixmul(normal->z, body->speed.z) //
			+ fixmul(body->angular_speed.x, cross_prod.x) //
			+ fixmul(body->angular_speed.y, cross_prod.y) //
			+ fixmul(body->angular_speed.z, cross_prod.z);

	force = math_div(-force, length);
	force = math_mul(force, g_const_7333);

	if (accel.x != 0 || accel.y != 0 || accel.z != 0) {

		// dot product to reflect bounce off vector
		aux = fixmul(accel.x, normal->x) + fixmul(accel.y, normal->y) + fixmul(accel.z, normal->z);
		accel.x = accel.x - fixmul(aux, normal->x);
		accel.y = accel.y - fixmul(aux, normal->y);
		accel.z = accel.z - fixmul(aux, normal->z);

		aux = fixmul(accel.x, accel.x) + fixmul(accel.y, accel.y) + fixmul(accel.z, accel.z);
		rebound = math_sqrt(aux);

		// inverse square root to normalize vector
		aux = -math_inverse_value(rebound);
		accel.x = math_mul(aux, accel.x);
		accel.y = math_mul(aux, accel.y);
		accel.z = math_mul(aux, accel.z);
	}

	// soften the collision a bit
	dampen = math_mul(g_const_CCCC, force);
	accel_scale.x = math_mul(math_mul(dampen, body->linear_acc_factor), accel.x);
	accel_scale.y = math_mul(math_mul(dampen, body->linear_acc_factor), accel.y);
	accel_scale.z = math_mul(math_mul(dampen, body->linear_acc_factor), accel.z);

	aux = fixmul(l_edge->y, accel.x) - fixmul(l_edge->x, accel.y);
	iX = fixmul(l_edge->z, fixmul(l_edge->x, accel.z) - fixmul(l_edge->z, accel.x)) - fixmul(l_edge->y, aux);
	iY = fixmul(l_edge->x, aux) - fixmul(l_edge->z, iX);
	iZ = fixmul(l_edge->y, iX) - fixmul(l_edge->x, iY);

	aux = math_mul(dampen, body->linear_acc_factor);
	accel_edge.x = math_mul(aux, iX);
	accel_edge.y = math_mul(aux, iY);
	accel_edge.z = math_mul(aux, iZ);

	dampening = fixmul(accel_scale.x + accel_edge.x, accel.x) //
			+ fixmul(accel_scale.y + accel_edge.y, accel.y) //
			+ fixmul(accel_scale.z + accel_edge.z, accel.z);

	if (dampening > rebound) {
		dampen = math_mul(dampen, math_div(rebound, dampening));
	}

	accel.x = math_mul(dampen, accel.x);
	accel.y = math_mul(dampen, accel.y);
	accel.z = math_mul(dampen, accel.z);

	// change linear and rotation speeds
	if (force > 0) {
		g_collision_force = force;

		// force vector
		normal_accel.x = math_mul(force, normal->x) + accel.x;
		normal_accel.y = math_mul(force, normal->y) + accel.y;
		normal_accel.z = math_mul(force, normal->z) + accel.z;

		body->speed.x += math_mul(body->linear_acc_factor, normal_accel.x);
		body->speed.y += math_mul(body->linear_acc_factor, normal_accel.y);
		body->speed.z += math_mul(body->linear_acc_factor, normal_accel.z);

		// cross product of point length and force vector
		body->angular_speed.x += math_mul(body->angular_acc_factor, fixmul(l_edge->y, normal_accel.z) - fixmul(l_edge->z, normal_accel.y));
		body->angular_speed.y += math_mul(body->angular_acc_factor, fixmul(l_edge->z, normal_accel.x) - fixmul(l_edge->x, normal_accel.z));
		body->angular_speed.z += math_mul(body->angular_acc_factor, fixmul(l_edge->x, normal_accel.y) - fixmul(l_edge->y, normal_accel.x));
	}
}

void tnfs_collision_detect(tnfs_collision_data *body, tnfs_vec3 *surf_normal, tnfs_vec3 *surf_pos) {
	int trespass;
	int sideX;
	int sideY;
	int sideZ;
	int aux;
	tnfs_vec3 v_length;
	tnfs_vec3 v_speed;
	tnfs_vec3 l_edge;
	tnfs_vec3 g_edge;
	tnfs_vec3 backoff;

	// zero normal check
	if (((surf_normal->x == 0) && (surf_normal->y == 0)) && (surf_normal->z == 0)) {
		surf_normal->y = 0x10000;
	}

	// closest edge point
	v_length.x = fixmul( //
			fixmul(surf_normal->x, body->matrix.ax)//
			+ fixmul(surf_normal->y, body->matrix.ay)//
			+ fixmul(surf_normal->z, body->matrix.az),//
			body->size.x);

	v_length.y = fixmul( //
			fixmul(surf_normal->x, body->matrix.bx)//
			+ fixmul(surf_normal->y, body->matrix.by)//
			+ fixmul(surf_normal->z, body->matrix.bz),//
			body->size.y);//

	v_length.z = fixmul( //
			fixmul(surf_normal->x, body->matrix.cx)//
			+ fixmul(surf_normal->y, body->matrix.cy)//
			+ fixmul(surf_normal->z, body->matrix.cz),//
			body->size.z);

	g_collision_force = 0;

	// negate to find lowest point
	sideX = -1;
	if (v_length.x < 0) {
		sideX = 1;
	}
	sideY = -1;
	if (v_length.y < 0) {
		sideY = 1;
	}
	sideZ = -1;
	if (v_length.z < 0) {
		sideZ = 1;
	}

	// distance between surface and body closest point
	g_surf_distance = sideX * v_length.x + sideY * v_length.y + sideZ * v_length.z //
			+ fixmul(surf_normal->x, body->position.x - surf_pos->x) //
			+ fixmul(surf_normal->y, body->position.y - surf_pos->y) //
			+ fixmul(surf_normal->z, body->position.z - surf_pos->z);

	// if collided
	if (g_surf_distance < 0) {

		// point of contact
		g_edge.x = sideX * fixmul(body->matrix.ax, body->size.x) //
				+ sideY * fixmul(body->matrix.bx, body->size.y) //
				+ sideZ * fixmul(body->matrix.cx, body->size.z) //
				+ body->position.x;
		g_edge.y = sideX * fixmul(body->matrix.ay, body->size.x) //
				+ sideY * fixmul(body->matrix.by, body->size.y) //
				+ sideZ * fixmul(body->matrix.cy, body->size.z) //
				+ body->position.y;
		g_edge.z = sideX * fixmul(body->matrix.az, body->size.x) //
				+ sideY * fixmul(body->matrix.bz, body->size.y) //
				+ sideZ * fixmul(body->matrix.cz, body->size.z) //
				+ body->position.z;


		// do not allow the body go through surface
		trespass = 0;
		if (g_surf_distance < 0) {
			trespass = -g_surf_distance;
		}
		if (g_surf_clipping && (trespass != 0)) {
			// gently put the body above the surface
			aux = trespass >> 1;
			backoff.x = math_mul(aux, surf_normal->x);
			backoff.y = math_mul(aux, surf_normal->y);
			backoff.z = math_mul(aux, surf_normal->z);
			body->position.x += backoff.x;
			body->position.y += backoff.y;
			body->position.z += backoff.z;
		}

		// local point of contact
		l_edge.x = g_edge.x - body->position.x;
		l_edge.y = g_edge.y - body->position.y;
		l_edge.z = g_edge.z - body->position.z;

		// speed vector
		v_speed.x = fixmul(body->angular_speed.y, l_edge.z) - fixmul(body->angular_speed.z, l_edge.y) + body->speed.x;
		v_speed.y = fixmul(body->angular_speed.z, l_edge.x) - fixmul(body->angular_speed.x, l_edge.z) + body->speed.y;
		v_speed.z = fixmul(body->angular_speed.x, l_edge.y) - fixmul(body->angular_speed.y, l_edge.x) + body->speed.z;


		// check if vectors are in opposite directions
		if (fixmul(surf_normal->x, v_speed.x) + fixmul(surf_normal->y, v_speed.y) + fixmul(surf_normal->z, v_speed.z) < 0) {

			// call bounce off calculation
			tnfs_collision_rebound(body, &l_edge, &v_speed, surf_normal);

			// never used values
			g_collision_speed.x = fixmul(body->angular_speed.y, l_edge.z) - fixmul(body->angular_speed.z, l_edge.y) + body->speed.x;
			g_collision_speed.y = fixmul(body->angular_speed.z, l_edge.x) - fixmul(body->angular_speed.x, l_edge.z) + body->speed.y;
			g_collision_speed.z = fixmul(body->angular_speed.x, l_edge.y) - fixmul(body->angular_speed.y, l_edge.x) + body->speed.z;
			v_speed.x = g_collision_speed.x;
			v_speed.y = g_collision_speed.y;
			v_speed.z = g_collision_speed.z;
		}

		// adjust height position
		if (g_surf_clipping && (trespass != 0) && (backoff.y > 0)) {
			trespass = math_mul(0x9cf5c, backoff.y) * 2;
			aux = math_mul(body->speed.y, body->speed.y);
			if (trespass > aux) {
				body->speed.y = 0;
			} else {
				aux -= trespass;
				trespass = math_sqrt(aux);
				if (body->speed.y > 0) {
					body->speed.y = trespass;
				} else {
					body->speed.y = -trespass;
				}
			}
		}

	}
}

void tnfs_collision_update_vectors(tnfs_collision_data *body) {
	int aux;
	tnfs_vec9 matrix;

	// update position and speed
	body->position.x += math_mul(0x888, body->speed.x);
	body->position.y += math_mul(0x888, body->speed.y);
	body->position.z += math_mul(0x888, body->speed.z);

	/* reduce speeds */
	body->speed.x -= body->speed.x >> 6;
	body->speed.y -= body->speed.y >> 6;
	body->speed.z -= body->speed.z >> 6;

	/* apply gravity */
	body->speed.y -= 0x53b6;

	// check angular speed
	if (body->angular_speed.x > 0xf0000) {
		body->angular_speed.x = 0xf0000;
	}
	if (body->angular_speed.y > 0xf0000) {
		body->angular_speed.y = 0xf0000;
	}
	if (body->angular_speed.z > 0xf0000) {
		body->angular_speed.z = 0xf0000;
	}
	if (body->angular_speed.x < -0xf0000) {
		body->angular_speed.x = -0xf0000;
	}
	if (body->angular_speed.y < -0xf0000) {
		body->angular_speed.y = -0xf0000;
	}
	if (body->angular_speed.z < -0xf0000) {
		body->angular_speed.z = -0xf0000;
	}

	// update matrix
	aux = math_vec3_length(&body->angular_speed);
	aux = math_mul(aux, 0x888);
	aux = math_mul(aux, 0x28be63);
	math_matrix_create_from_vec3(&matrix, aux, &body->angular_speed);
	math_matrix_multiply(&body->matrix, &body->matrix, &matrix);
}

void tnfs_collision_main(tnfs_car_data *car) {
	tnfs_collision_data *collision_data;
	tnfs_vec3 roadNormal;
	tnfs_vec3 fenceNormal;
	tnfs_vec3 fencePosition;
	tnfs_vec3 roadPosition;
	tnfs_vec3 fenceDistance;
	tnfs_vec3 roadHeading;
	int roadWidth;
	int iVar4 = 0;
	int local_24 = 0;
	int local_28 = 0;
	int aux;

	// ...

	car->is_crashed = 1;
	collision_data = &car->collision_data;

	if (car->car_data_ptr == (tnfs_car_data*) 0x0) {
		aux = -car->rpm_engine + 1500;
		if (aux < 0) {
			aux += 1507;
		}
		car->rpm_engine += (aux >> 3);
	}

	if (tnfs_road_segment_update(car)) {
		tnfs_track_update_vectors(car);
	}

	roadNormal.x = car->road_surface_normal.x;
	roadNormal.y = car->road_surface_normal.y;
	roadNormal.z = -car->road_surface_normal.z;
	fenceNormal.x = car->road_fence_normal.x;
	fenceNormal.y = car->road_fence_normal.y;
	fenceNormal.z = -car->road_fence_normal.z;
	roadPosition.x = car->road_ground_position.x;
	roadPosition.y = car->road_ground_position.y;
	roadPosition.z = -car->road_ground_position.z;
	roadHeading.x = car->road_heading.x;
	roadHeading.y = car->road_heading.y;
	roadHeading.z = -car->road_heading.z;

	tnfs_collision_update_vectors(collision_data);

	/* get track fences */
	fenceDistance.x = collision_data->position.x - roadPosition.x;
	fenceDistance.y = collision_data->position.y - roadPosition.y;
	fenceDistance.z = collision_data->position.z - roadPosition.z;

	// check if left or right fence
	if (fixmul(fenceDistance.x, fenceNormal.x) + fixmul(fenceDistance.y, fenceNormal.y) + fixmul(fenceDistance.z, fenceNormal.z) < 1) {
		aux = DAT_800eae0c;
		//if ((bRam00000005 >> 4 != 0) && (cRam00000007 != '\x05')) {
		//  aux = DAT_800eae10;
		//}
		roadWidth = (track_data[car_data.road_segment_a].roadLeftFence * -0x2000 - aux) >> 16;

		fencePosition.y = roadWidth * fenceNormal.y + roadPosition.y;
		fencePosition.x = roadWidth * fenceNormal.x + roadPosition.x;
		fencePosition.z = roadWidth * fenceNormal.z + roadPosition.z;
	} else {
		aux = DAT_800eae0c;
		//if (((bRam00000005 & 0xf) != 0) && (cRam00000007 != '\x05')) {
		//  aux = DAT_800eae10;
		//}
		roadWidth = (track_data[car_data.road_segment_a].roadRightFence * -0x2000 - aux) >> 16;

		fenceNormal.x = -fenceNormal.x;
		fenceNormal.z = -fenceNormal.z;
		fenceNormal.y = -fenceNormal.y;
		fencePosition.y = roadWidth * fenceNormal.y + roadPosition.y;
		fencePosition.x = roadWidth * fenceNormal.x + roadPosition.x;
		fencePosition.z = roadWidth * fenceNormal.z + roadPosition.z;
	}

	/* car colliding to track fence */
	tnfs_collision_detect(collision_data, &fenceNormal, &fencePosition);

	/* car collision to ground */
	tnfs_collision_detect(collision_data, &roadNormal, &roadPosition);

	/* ... lots of code goes here -- crash recovery ... */


 	// play crashing sounds
	iVar4 = 2;
	if (DAT_800eae18 > 0) {
		if (sound_flag == 0) {
			tnfs_car_size_vector(car->car_data_ptr, &local_28, &local_24);
		}
		if (car->collision_data.field6_0x60 > 0x80000) {
			tnfs_car_size_vector(car->car_data_ptr, &local_28, &local_24);
			if (car->unknown_flag_480 == 0) {
				local_24 = 1;
				local_28 = 0x400000;
			} else {
				local_24 = 1;
				local_28 = 0xc00000;
			}
			tnfs_sfx_play(-1, iVar4, 1, 0, local_24, local_28);
		}
		DAT_800eae18 = 0x8000;
		DAT_800eae14 = 10;
	}

	// update new position, speed and orientation
	car->position.x = collision_data->position.x;
	car->position.y = collision_data->position.y;
	car->position.z = -collision_data->position.z;
	car->position.x -= fixmul(collision_data->matrix.bx, car->collision_height_offset);
	car->position.y -= fixmul(collision_data->matrix.by, car->collision_height_offset);
	car->position.z += fixmul(collision_data->matrix.bz, car->collision_height_offset);

	car->speed_x = collision_data->speed.x;
	car->speed_y = collision_data->speed.y;
	car->speed_z = collision_data->speed.z;
	car->speed_z = -car->speed_z;
	car->speed_x = -car->speed_x;

	// ...

	memcpy(&car->matrix, &car->collision_data.matrix, 0x24u);

	car->matrix.az = -car->matrix.az;
	car->matrix.bz = -car->matrix.bz;
	car->matrix.cx = -car->matrix.cx;
	car->matrix.cy = -car->matrix.cy;
}

void tnfs_collision_rollover_start_3(tnfs_car_data *car) {
	car->collision_data.position.x = car->position.x;
	car->collision_data.position.y = car->position.y;
	car->collision_data.position.z = -car->position.z;
	car->collision_data.speed.x = car->speed_x;
	car->collision_data.speed.y = car->speed_y;
	car->collision_data.speed.z = car->speed_z;
	car->collision_data.field4_0x48.x = 0;
	car->collision_data.field4_0x48.y = 0;
	car->collision_data.field4_0x48.z = 0;
	car->collision_data.field4_0x48.y = -0x9cf5c;
	car->collision_data.angular_speed.x = 0;
	car->collision_data.angular_speed.y = 0;
	car->collision_data.angular_speed.z = 0;

	car->collision_data.angular_speed.y = -math_mul(car->angular_speed, 0x648);

	car->collision_data.speed.z = -car->collision_data.speed.z;
	car->collision_data.speed.x = -car->collision_data.speed.x;

	if (car->is_crashed == 0) {
		matrix_create_from_pitch_yaw_roll(&car->collision_data.matrix, -car->angle_x, -car->angle_y, car->angle_z);
	} else {
		memcpy(&car->collision_data.matrix, &car->matrix, 0x24);

		car->collision_data.matrix.az = -car->collision_data.matrix.az;
		car->collision_data.matrix.bz = -car->collision_data.matrix.bz;
		car->collision_data.matrix.cx = -car->collision_data.matrix.cx;
		car->collision_data.matrix.cy = -car->collision_data.matrix.cy;
	}

	// height offset
	car->collision_data.position.x += fixmul(car->collision_data.matrix.bx, car->collision_height_offset);
	car->collision_data.position.y += fixmul(car->collision_data.matrix.by, car->collision_height_offset);
	car->collision_data.position.z += fixmul(car->collision_data.matrix.bz, car->collision_height_offset);
}

void tnfs_collision_rollover_start_2(tnfs_car_data *param_1) {
	tnfs_collision_rollover_start_3(param_1);
	param_1->is_wrecked = 1;
	param_1->field444_0x520 = 4;
	//FUN_8004ce14((tnfs_car_data *)&PTR_80103660);
	param_1->field203_0x174 = param_1->field203_0x174 & 0xfffffdff;
	param_1->collision_data.crashed_time = 300;
	//tnfs_unecessary_800489a4(0x5c);
	if (sound_flag == 0) {
		if (param_1 != tnfs_car_data_ptr) {
			return;
		}
	} else if (1 < param_1->field445_0x524) {
		return;
	}
	//FUN_8003c09c(param_1);
}

void tnfs_collision_rollover_start(tnfs_car_data *car, int force_z, int force_y, int force_x) {
	tnfs_collision_rollover_start_2(car);

	car->collision_data.angular_speed.x += math_mul(force_x, car->collision_data.matrix.ax);
	car->collision_data.angular_speed.y += math_mul(force_x, car->collision_data.matrix.ay);
	car->collision_data.angular_speed.z += math_mul(force_x, car->collision_data.matrix.az);
	car->collision_data.angular_speed.x += math_mul(force_y, car->collision_data.matrix.cx);
	car->collision_data.angular_speed.y += math_mul(force_y, car->collision_data.matrix.cy);
	car->collision_data.angular_speed.z += math_mul(force_y, car->collision_data.matrix.cz);
	car->collision_data.angular_speed.x -= math_mul(force_z, car->collision_data.matrix.bx);
	car->collision_data.angular_speed.y -= math_mul(force_z, car->collision_data.matrix.by);
	car->collision_data.angular_speed.z -= math_mul(force_z, car->collision_data.matrix.bz);
	if ((car->field203_0x174 & 4U) != 0) {
		//  FUN_800534e0(0);
	}
}

