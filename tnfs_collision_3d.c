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
	// simplified version
	if (car->collision_data.crashed_time > 0) {
		car->collision_data.crashed_time--;
	} else {
		tnfs_reset_car(car);
		return;
	}


 	// play crashing sounds
	iVar4 = 2;
	if (DAT_800eae18 > 0) {
		if (sound_flag == 0) {
			tnfs_car_local_position_vector(car->car_data_ptr, &local_28, &local_24);
		}
		if (car->collision_data.field6_0x60 > 0x80000) {
			tnfs_car_local_position_vector(car->car_data_ptr, &local_28, &local_24);
			if (car->unknown_flag_475 == 0) {
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

/*
 * in the beginning of a crash, fill collision data structure from car data
 */
void tnfs_collision_data_set(tnfs_car_data *car) {
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
		math_matrix_from_pitch_yaw_roll(&car->collision_data.matrix, -car->angle_x, -car->angle_y, car->angle_z);
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

/*
 * after a crash, move updated collision_data back to car_data
 */
void tnfs_collision_data_get(tnfs_car_data *car) {
	tnfs_collision_data *body;

	// ... some unknown functions here ...

	body = &car->collision_data;
	//car->unknown_0x520 = param_2;
	car->position.x = body->position.x;
	car->position.y = body->position.y;
	car->position.z = body->position.z;
	car->position.x = car->position.x - fixmul(body->matrix.bx, car->collision_height_offset);
	car->position.y = car->position.y - fixmul(body->matrix.by, car->collision_height_offset);
	car->position.z = (body->matrix.bz >> 8) * (car->collision_height_offset >> 8) - car->position.z;
	car->speed_x = body->speed.x;
	car->speed_y = body->speed.y;
	car->speed_z = body->speed.z;
	car->speed_z = -car->speed_z;
	car->speed_x = -car->speed_x;
	car->angular_speed = ((-car->angular_speed - math_mul((body->angular_speed).y, 0x28be63)) >> 1) + car->angular_speed;
	body->crashed_time = 0;
	car->slide_front = 0;
	car->slide_rear = 0;
	//if ((-1 < car->unknown_flag_475) && (car->unknown_flag_475 < DAT_8010d1cc)) {
	//  FUN_80030fe0(car);
	//}
}

void tnfs_collision_rollover_start_2(tnfs_car_data *car) {
	tnfs_collision_data_set(car);
	car->is_wrecked = 1;
	car->field444_0x520 = 4;
	//FUN_8004ce14((tnfs_car_data *)&PTR_80103660);
	car->field203_0x174 = car->field203_0x174 & 0xfffffdff;
	car->collision_data.crashed_time = 300;
	tnfs_replay_highlight_record(0x5c);
	if (sound_flag == 0) {
		if (car != tnfs_car_data_ptr) {
			return;
		}
	} else if (1 < car->field445_0x524) {
		return;
	}
	//FUN_8003c09c(param_1);
}


/*
 * initiate a car crash rollover
 */
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


int _DAT_001449cc;

/*
 * huge function calculates rebound vectors for a car-to-car collision
 * directly extracted from Ghidra/PSX version
 */
int tnfs_collision_carcar_huge_func(tnfs_collision_data *body1, tnfs_collision_data *body2,
		tnfs_vec3 *col_position, tnfs_vec3 *col_direction, tnfs_vec9 *param_4, tnfs_vec9 *param_5, tnfs_vec9 *param_6) {

	int iVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	int local_10c;
	int local_108;
	int local_104;
	int local_100;
	int local_fc;
	int local_f8;
	int local_f4;
	int local_f0;
	int local_ec;
	int local_e8;
	int local_e4;
	int local_e0;
	int local_dc;
	int local_d8;
	int local_d4;
	int local_b8;
	int local_a8;
	int local_a4;
	int local_a0;
	int local_9c;
	int local_98;
	int local_94;
	int local_90;
	int local_8c;
	int local_88;
	int local_84;
	int local_80;
	int local_7c;
	int local_78;
	int local_74;
	int local_70;
	int local_6c;
	int local_68;
	int local_64;
	int local_48;
	int local_38;
	int local_34;
	int local_30;
	int local_1c;
	int local_18;
	int local_14;

	iVar2 = param_4->ax + param_6->ax;
	iVar3 = param_4->bx + param_6->ay;
	iVar4 = param_4->cx + param_6->az;
	if ((((((body1->size).z < iVar4) || (iVar1 = (body1->size).z, -iVar4 != iVar1 && iVar4 <= -iVar1)) || ((body1->size).x < iVar2))
			|| ((iVar4 = (body1->size).x, -iVar2 != iVar4 && iVar2 <= -iVar4 || ((body1->size).y < iVar3)))) || (iVar2 = (body1->size).y, -iVar3 != iVar2 && iVar3 <= -iVar2)) {
		iVar2 = param_4->ay + param_6->ax;
		iVar3 = param_4->by + param_6->ay;
		iVar4 = param_4->cy + param_6->az;
		if ((((body1->size).z < iVar4) || (iVar1 = (body1->size).z, -iVar4 != iVar1 && iVar4 <= -iVar1))
				|| (((body1->size).x < iVar2
						|| (((iVar4 = (body1->size).x, -iVar2 != iVar4 && iVar2 <= -iVar4 || ((body1->size).y < iVar3))
								|| (iVar2 = (body1->size).y, -iVar3 != iVar2 && iVar3 <= -iVar2)))))) {
			iVar2 = param_4->az + param_6->ax;
			iVar3 = param_4->bz + param_6->ay;
			iVar4 = param_4->cz + param_6->az;
			if ((((body1->size).z < iVar4) || (iVar1 = (body1->size).z, -iVar4 != iVar1 && iVar4 <= -iVar1))
					|| (((body1->size).x < iVar2
							|| (((iVar4 = (body1->size).x, -iVar2 != iVar4 && iVar2 <= -iVar4 || ((body1->size).y < iVar3))
									|| (iVar2 = (body1->size).y, -iVar3 != iVar2 && iVar3 <= -iVar2)))))) {
				iVar2 = param_6->ax - param_4->ax;
				iVar3 = param_6->ay - param_4->bx;
				iVar4 = param_6->az - param_4->cx;
				if ((((((body1->size).z < iVar4) || (iVar1 = (body1->size).z, -iVar4 != iVar1 && iVar4 <= -iVar1)) || ((body1->size).x < iVar2))
						|| ((iVar4 = (body1->size).x, -iVar2 != iVar4 && iVar2 <= -iVar4 || ((body1->size).y < iVar3))))
						|| (iVar2 = (body1->size).y, -iVar3 != iVar2 && iVar3 <= -iVar2)) {
					iVar2 = param_6->ax - param_4->ay;
					iVar3 = param_6->ay - param_4->by;
					iVar4 = param_6->az - param_4->cy;
					if ((((body1->size).z < iVar4) || (iVar1 = (body1->size).z, -iVar4 != iVar1 && iVar4 <= -iVar1))
							|| (((body1->size).x < iVar2 || (((iVar4 = (body1->size).x, -iVar2 != iVar4 && iVar2 <= -iVar4 || ((body1->size).y < iVar3)) || (iVar2 =
									(body1->size).y, -iVar3 != iVar2 && iVar3 <= -iVar2)))))) {
						iVar2 = param_6->ax - param_4->az;
						iVar3 = param_6->ay - param_4->bz;
						iVar4 = param_6->az - param_4->cz;
						if ((((((body1->size).z < iVar4) || (iVar1 = (body1->size).z, -iVar4 != iVar1 && iVar4 <= -iVar1)) || ((body1->size).x < iVar2))
								|| ((iVar4 = (body1->size).x, -iVar2 != iVar4 && iVar2 <= -iVar4 || ((body1->size).y < iVar3))))
								|| (iVar2 = (body1->size).y, -iVar3 != iVar2 && iVar3 <= -iVar2)) {
							iVar2 = param_5->ax - param_6->bx;
							iVar3 = param_5->ay - param_6->by;
							iVar4 = param_5->az - param_6->bz;
							if ((((((body2->size).z < iVar4) || (iVar1 = (body2->size).z, -iVar4 != iVar1 && iVar4 <= -iVar1)) || ((body2->size).x < iVar2))
									|| ((iVar4 = (body2->size).x, -iVar2 != iVar4 && iVar2 <= -iVar4 || ((body2->size).y < iVar3))))
									|| (iVar2 = (body2->size).y, -iVar3 != iVar2 && iVar3 <= -iVar2)) {
								iVar2 = param_5->bx - param_6->bx;
								iVar3 = param_5->by - param_6->by;
								iVar4 = param_5->bz - param_6->bz;
								if ((((body2->size).z < iVar4) || (iVar1 = (body2->size).z, -iVar4 != iVar1 && iVar4 <= -iVar1))
										|| (((body2->size).x < iVar2 || (((iVar4 = (body2->size).x, -iVar2 != iVar4 && iVar2 <= -iVar4 || ((body2->size).y < iVar3)) || (iVar2 =
												(body2->size).y, -iVar3 != iVar2 && iVar3 <= -iVar2)))))) {
									iVar2 = param_5->cx - param_6->bx;
									iVar3 = param_5->cy - param_6->by;
									iVar4 = param_5->cz - param_6->bz;
									if ((((((body2->size).z < iVar4) || (iVar1 = (body2->size).z, -iVar4 != iVar1 && iVar4 <= -iVar1)) || ((body2->size).x < iVar2)) || ((iVar4 =
											(body2->size).x, -iVar2 != iVar4 && iVar2 <= -iVar4 || ((body2->size).y < iVar3))))
											|| (iVar2 = (body2->size).y, -iVar3 != iVar2 && iVar3 <= -iVar2)) {
										iVar2 = -param_6->bx - param_5->ax;
										iVar3 = -param_6->by - param_5->ay;
										iVar4 = -param_6->bz - param_5->az;
										if (((((body2->size).z < iVar4) || (iVar1 = (body2->size).z, -iVar4 != iVar1 && iVar4 <= -iVar1)) || ((body2->size).x < iVar2))
												|| (((iVar4 = (body2->size).x, -iVar2 != iVar4 && iVar2 <= -iVar4 || ((body2->size).y < iVar3))
														|| (iVar2 = (body2->size).y, -iVar3 != iVar2 && iVar3 <= -iVar2)))) {
											iVar2 = -param_6->bx - param_5->bx;
											iVar3 = -param_6->by - param_5->by;
											iVar4 = -param_6->bz - param_5->bz;
											if ((((body2->size).z < iVar4) || (iVar1 = (body2->size).z, -iVar4 != iVar1 && iVar4 <= -iVar1))
													|| (((body2->size).x < iVar2
															|| (((iVar4 = (body2->size).x, -iVar2 != iVar4 && iVar2 <= -iVar4 || ((body2->size).y < iVar3)) || (iVar2 =
																	(body2->size).y, -iVar3 != iVar2 && iVar3 <= -iVar2)))))) {
												iVar2 = -param_6->bx - param_5->cx;
												iVar3 = -param_6->by - param_5->cy;
												iVar4 = -param_6->bz - param_5->cz;
												if ((((body2->size).z < iVar4) || (iVar1 = (body2->size).z, -iVar4 != iVar1 && iVar4 <= -iVar1))
														|| (((body2->size).x < iVar2
																|| (((iVar4 = (body2->size).x, -iVar2 != iVar4 && iVar2 <= -iVar4 || ((body2->size).y < iVar3))
																		|| (iVar2 = (body2->size).y, -iVar3 != iVar2 && iVar3 <= -iVar2)))))) {
													local_1c = param_6->ax + param_4->ax + param_4->ay + param_4->az;
													local_18 = param_6->ay + param_4->bx + param_4->by + param_4->bz;
													local_14 = param_6->az + param_4->cx + param_4->cy + param_4->cz;
													if ((((((body1->size).z < local_14) || (iVar2 = (body1->size).z, -local_14 != iVar2 && local_14 <= -iVar2))
															|| ((body1->size).x < local_1c))
															|| ((iVar2 = (body1->size).x, -local_1c != iVar2 && local_1c <= -iVar2 || ((body1->size).y < local_18)))) || (iVar2 =
															(body1->size).y, -local_18 != iVar2 && local_18 <= -iVar2)) {
														local_1c = param_6->ax + ((param_4->ax + param_4->ay) - param_4->az);
														local_18 = param_6->ay + ((param_4->bx + param_4->by) - param_4->bz);
														local_14 = param_6->az + ((param_4->cx + param_4->cy) - param_4->cz);
														if ((((body1->size).z < local_14) || (iVar2 = (body1->size).z, -local_14 != iVar2 && local_14 <= -iVar2))
																|| (((body1->size).x < local_1c
																		|| (((iVar2 = (body1->size).x, -local_1c != iVar2 && local_1c <= -iVar2 || ((body1->size).y < local_18))
																				|| (iVar2 = (body1->size).y, -local_18 != iVar2 && local_18 <= -iVar2)))))) {
															local_1c = param_6->ax + (param_4->ax - param_4->ay) + param_4->az;
															local_18 = param_6->ay + (param_4->bx - param_4->by) + param_4->bz;
															local_14 = param_6->az + (param_4->cx - param_4->cy) + param_4->cz;
															if (((((body1->size).z < local_14) || (iVar2 = (body1->size).z, -local_14 != iVar2 && local_14 <= -iVar2))
																	|| ((body1->size).x < local_1c))
																	|| (((iVar2 = (body1->size).x, -local_1c != iVar2 && local_1c <= -iVar2 || ((body1->size).y < local_18))
																			|| (iVar2 = (body1->size).y, -local_18 != iVar2 && local_18 <= -iVar2)))) {
																local_1c = param_6->ax + ((param_4->ax - param_4->ay) - param_4->az);
																local_18 = param_6->ay + ((param_4->bx - param_4->by) - param_4->bz);
																local_14 = param_6->az + ((param_4->cx - param_4->cy) - param_4->cz);
																if ((((((body1->size).z < local_14) || (iVar2 = (body1->size).z, -local_14 != iVar2 && local_14 <= -iVar2))
																		|| ((body1->size).x < local_1c))
																		|| ((iVar2 = (body1->size).x, -local_1c != iVar2 && local_1c <= -iVar2 || ((body1->size).y < local_18))))
																		|| (iVar2 = (body1->size).y, -local_18 != iVar2 && local_18 <= -iVar2)) {
																	local_1c = param_6->ax + (param_4->ay - param_4->ax) + param_4->az;
																	local_18 = param_6->ay + (param_4->by - param_4->bx) + param_4->bz;
																	local_14 = param_6->az + (param_4->cy - param_4->cx) + param_4->cz;
																	if ((((body1->size).z < local_14) || (iVar2 = (body1->size).z, -local_14 != iVar2 && local_14 <= -iVar2))
																			|| (((body1->size).x < local_1c
																					|| (((iVar2 = (body1->size).x, -local_1c != iVar2 && local_1c <= -iVar2
																							|| ((body1->size).y < local_18))
																							|| (iVar2 = (body1->size).y, -local_18 != iVar2 && local_18 <= -iVar2)))))) {
																		local_1c = param_6->ax + ((param_4->ay - param_4->ax) - param_4->az);
																		local_18 = param_6->ay + ((param_4->by - param_4->bx) - param_4->bz);
																		local_14 = param_6->az + ((param_4->cy - param_4->cx) - param_4->cz);
																		if ((((((body1->size).z < local_14) || (iVar2 = (body1->size).z, -local_14 != iVar2 && local_14 <= -iVar2))
																				|| ((body1->size).x < local_1c))
																				|| ((iVar2 = (body1->size).x, -local_1c != iVar2 && local_1c <= -iVar2
																						|| ((body1->size).y < local_18))))
																				|| (iVar2 = (body1->size).y, -local_18 != iVar2 && local_18 <= -iVar2)) {
																			local_1c = param_6->ax + (-param_4->ay - param_4->ax) + param_4->az;
																			local_18 = param_6->ay + (-param_4->by - param_4->bx) + param_4->bz;
																			local_14 = param_6->az + (-param_4->cy - param_4->cx) + param_4->cz;
																			if ((((((body1->size).z < local_14)
																					|| (iVar2 = (body1->size).z, -local_14 != iVar2 && local_14 <= -iVar2))
																					|| ((body1->size).x < local_1c))
																					|| ((iVar2 = (body1->size).x, -local_1c != iVar2 && local_1c <= -iVar2
																							|| ((body1->size).y < local_18))))
																					|| (iVar2 = (body1->size).y, -local_18 != iVar2 && local_18 <= -iVar2)) {
																				local_1c = param_6->ax + ((-param_4->ay - param_4->ax) - param_4->az);
																				local_18 = param_6->ay + ((-param_4->by - param_4->bx) - param_4->bz);
																				local_14 = param_6->az + ((-param_4->cy - param_4->cx) - param_4->cz);
																				if ((((body1->size).z < local_14)
																						|| (iVar2 = (body1->size).z, -local_14 != iVar2 && local_14 <= -iVar2))
																						|| (((body1->size).x < local_1c
																								|| (((iVar2 = (body1->size).x, -local_1c != iVar2 && local_1c <= -iVar2
																										|| ((body1->size).y < local_18))
																										|| (iVar2 = (body1->size).y, -local_18 != iVar2 && local_18 <= -iVar2)))))) {
																					local_1c = (param_5->ax + param_5->bx + param_5->cx) - param_6->bx;
																					local_18 = (param_5->ay + param_5->by + param_5->cy) - param_6->by;
																					local_14 = (param_5->az + param_5->bz + param_5->cz) - param_6->bz;
																					if ((((((body2->size).z < local_14)
																							|| (iVar2 = (body2->size).z, -local_14 != iVar2 && local_14 <= -iVar2))
																							|| ((body2->size).x < local_1c))
																							|| ((iVar2 = (body2->size).x, -local_1c != iVar2 && local_1c <= -iVar2
																									|| ((body2->size).y < local_18))))
																							|| (iVar2 = (body2->size).y, -local_18 != iVar2 && local_18 <= -iVar2)) {
																						local_1c = ((param_5->ax + param_5->bx) - param_5->cx) - param_6->bx;
																						local_18 = ((param_5->ay + param_5->by) - param_5->cy) - param_6->by;
																						local_14 = ((param_5->az + param_5->bz) - param_5->cz) - param_6->bz;
																						if ((((((body2->size).z < local_14)
																								|| (iVar2 = (body2->size).z, -local_14 != iVar2 && local_14 <= -iVar2))
																								|| ((body2->size).x < local_1c))
																								|| ((iVar2 = (body2->size).x, -local_1c != iVar2 && local_1c <= -iVar2
																										|| ((body2->size).y < local_18))))
																								|| (iVar2 = (body2->size).y, -local_18 != iVar2 && local_18 <= -iVar2)) {
																							local_1c = ((param_5->ax - param_5->bx) + param_5->cx) - param_6->bx;
																							local_18 = ((param_5->ay - param_5->by) + param_5->cy) - param_6->by;
																							local_14 = ((param_5->az - param_5->bz) + param_5->cz) - param_6->bz;
																							if ((((body2->size).z < local_14)
																									|| (iVar2 = (body2->size).z, -local_14 != iVar2 && local_14 <= -iVar2))
																									|| (((body2->size).x < local_1c
																											|| (((iVar2 = (body2->size).x, -local_1c != iVar2 && local_1c <= -iVar2
																													|| ((body2->size).y < local_18))
																													|| (iVar2 = (body2->size).y, -local_18 != iVar2
																															&& local_18 <= -iVar2)))))) {
																								local_1c = ((param_5->ax - param_5->bx) - param_5->cx) - param_6->bx;
																								local_18 = ((param_5->ay - param_5->by) - param_5->cy) - param_6->by;
																								local_14 = ((param_5->az - param_5->bz) - param_5->cz) - param_6->bz;
																								if ((((body2->size).z < local_14)
																										|| (iVar2 = (body2->size).z, -local_14 != iVar2 && local_14 <= -iVar2))
																										|| (((body2->size).x < local_1c
																												|| (((iVar2 = (body2->size).x, -local_1c != iVar2
																														&& local_1c <= -iVar2 || ((body2->size).y < local_18))
																														|| (iVar2 = (body2->size).y, -local_18 != iVar2
																																&& local_18 <= -iVar2)))))) {
																									local_1c = ((param_5->bx - param_5->ax) + param_5->cx) - param_6->bx;
																									local_18 = ((param_5->by - param_5->ay) + param_5->cy) - param_6->by;
																									local_14 = ((param_5->bz - param_5->az) + param_5->cz) - param_6->bz;
																									if ((((((body2->size).z < local_14)
																											|| (iVar2 = (body2->size).z, -local_14 != iVar2 && local_14 <= -iVar2))
																											|| ((body2->size).x < local_1c))
																											|| ((iVar2 = (body2->size).x, -local_1c != iVar2 && local_1c <= -iVar2
																													|| ((body2->size).y < local_18))))
																											|| (iVar2 = (body2->size).y, -local_18 != iVar2 && local_18 <= -iVar2)) {
																										local_1c = ((param_5->bx - param_5->ax) - param_5->cx) - param_6->bx;
																										local_18 = ((param_5->by - param_5->ay) - param_5->cy) - param_6->by;
																										local_14 = ((param_5->bz - param_5->az) - param_5->cz) - param_6->bz;
																										if ((((body2->size).z < local_14)
																												|| (iVar2 = (body2->size).z, -local_14 != iVar2
																														&& local_14 <= -iVar2))
																												|| (((body2->size).x < local_1c
																														|| (((iVar2 = (body2->size).x, -local_1c != iVar2
																																&& local_1c <= -iVar2
																																|| ((body2->size).y < local_18))
																																|| (iVar2 = (body2->size).y, -local_18 != iVar2
																																		&& local_18 <= -iVar2)))))) {
																											local_1c = ((-param_5->bx - param_5->ax) + param_5->cx) - param_6->bx;
																											local_18 = ((-param_5->by - param_5->ay) + param_5->cy) - param_6->by;
																											local_14 = ((-param_5->bz - param_5->az) + param_5->cz) - param_6->bz;
																											if ((((body2->size).z < local_14)
																													|| (iVar2 = (body2->size).z, -local_14 != iVar2
																															&& local_14 <= -iVar2))
																													|| (((body2->size).x < local_1c
																															|| (((iVar2 = (body2->size).x, -local_1c != iVar2
																																	&& local_1c <= -iVar2
																																	|| ((body2->size).y < local_18))
																																	|| (iVar2 = (body2->size).y, -local_18 != iVar2
																																			&& local_18 <= -iVar2)))))) {
																												local_1c = ((-param_5->bx - param_5->ax) - param_5->cx)
																														- param_6->bx;
																												local_18 = ((-param_5->by - param_5->ay) - param_5->cy)
																														- param_6->by;
																												local_14 = ((-param_5->bz - param_5->az) - param_5->cz)
																														- param_6->bz;
																												if ((((((body2->size).z < local_14)
																														|| (iVar2 = (body2->size).z, -local_14 != iVar2
																																&& local_14 <= -iVar2))
																														|| ((body2->size).x < local_1c))
																														|| ((iVar2 = (body2->size).x, -local_1c != iVar2
																																&& local_1c <= -iVar2
																																|| ((body2->size).y < local_18))))
																														|| (iVar2 = (body2->size).y, -local_18 != iVar2
																																&& local_18 <= -iVar2)) {
																													return 0;
																												}
																												col_position->x = (((body1->position).x
																														- ((body1->matrix).ax >> 8) * ((body1->size).x >> 8))
																														- ((body1->matrix).bx >> 8) * ((body1->size).y >> 8))
																														- ((body1->size).z >> 8) * ((body1->matrix).cx >> 8);
																												col_position->y = (((body1->position).y
																														- ((body1->size).x >> 8) * ((body1->matrix).ay >> 8))
																														- ((body1->size).y >> 8) * ((body1->matrix).by >> 8))
																														- ((body1->size).z >> 8) * ((body1->matrix).cy >> 8);
																												col_position->z = (((body1->position).z
																														- ((body1->size).x >> 8) * ((body1->matrix).az >> 8))
																														- ((body1->size).y >> 8) * ((body1->matrix).bz >> 8))
																														- ((body1->matrix).cz >> 8) * ((body1->size).z >> 8);
																											} else {
																												col_position->x = (((body1->position).x
																														- ((body1->matrix).ax >> 8) * ((body1->size).x >> 8))
																														- ((body1->matrix).bx >> 8) * ((body1->size).y >> 8))
																														+ ((body1->size).z >> 8) * ((body1->matrix).cx >> 8);
																												col_position->y = (((body1->position).y
																														- ((body1->size).x >> 8) * ((body1->matrix).ay >> 8))
																														- ((body1->size).y >> 8) * ((body1->matrix).by >> 8))
																														+ ((body1->size).z >> 8) * ((body1->matrix).cy >> 8);
																												col_position->z = (((body1->position).z
																														- ((body1->size).x >> 8) * ((body1->matrix).az >> 8))
																														- ((body1->size).y >> 8) * ((body1->matrix).bz >> 8))
																														+ ((body1->matrix).cz >> 8) * ((body1->size).z >> 8);
																											}
																										} else {
																											col_position->x = (((body1->position).x
																													- ((body1->matrix).ax >> 8) * ((body1->size).x >> 8))
																													+ ((body1->matrix).bx >> 8) * ((body1->size).y >> 8))
																													- ((body1->size).z >> 8) * ((body1->matrix).cx >> 8);
																											col_position->y = (((body1->position).y
																													- ((body1->size).x >> 8) * ((body1->matrix).ay >> 8))
																													+ ((body1->size).y >> 8) * ((body1->matrix).by >> 8))
																													- ((body1->size).z >> 8) * ((body1->matrix).cy >> 8);
																											col_position->z = (((body1->size).y >> 8) * ((body1->matrix).bz >> 8)
																													+ ((body1->position).z
																															- ((body1->size).x >> 8) * ((body1->matrix).az >> 8)))
																													- ((body1->matrix).cz >> 8) * ((body1->size).z >> 8);
																										}
																									} else {
																										col_position->x = ((body1->position).x
																												- ((body1->matrix).ax >> 8) * ((body1->size).x >> 8))
																												+ ((body1->matrix).bx >> 8) * ((body1->size).y >> 8)
																												+ ((body1->size).z >> 8) * ((body1->matrix).cx >> 8);
																										col_position->y = ((body1->position).y
																												- ((body1->size).x >> 8) * ((body1->matrix).ay >> 8))
																												+ ((body1->size).y >> 8) * ((body1->matrix).by >> 8)
																												+ ((body1->size).z >> 8) * ((body1->matrix).cy >> 8);
																										col_position->z = ((body1->size).y >> 8) * ((body1->matrix).bz >> 8)
																												+ ((body1->position).z
																														- ((body1->size).x >> 8) * ((body1->matrix).az >> 8))
																												+ ((body1->matrix).cz >> 8) * ((body1->size).z >> 8);
																									}
																								} else {
																									col_position->x = (((body1->position).x
																											+ ((body1->matrix).ax >> 8) * ((body1->size).x >> 8))
																											- ((body1->matrix).bx >> 8) * ((body1->size).y >> 8))
																											- ((body1->size).z >> 8) * ((body1->matrix).cx >> 8);
																									col_position->y = (((body1->position).y
																											+ ((body1->size).x >> 8) * ((body1->matrix).ay >> 8))
																											- ((body1->size).y >> 8) * ((body1->matrix).by >> 8))
																											- ((body1->size).z >> 8) * ((body1->matrix).cy >> 8);
																									col_position->z = (((body1->position).z
																											+ ((body1->size).x >> 8) * ((body1->matrix).az >> 8))
																											- ((body1->size).y >> 8) * ((body1->matrix).bz >> 8))
																											- ((body1->matrix).cz >> 8) * ((body1->size).z >> 8);
																								}
																							} else {
																								col_position->x = (((body1->position).x
																										+ ((body1->matrix).ax >> 8) * ((body1->size).x >> 8))
																										- ((body1->matrix).bx >> 8) * ((body1->size).y >> 8))
																										+ ((body1->size).z >> 8) * ((body1->matrix).cx >> 8);
																								col_position->y = (((body1->position).y
																										+ ((body1->size).x >> 8) * ((body1->matrix).ay >> 8))
																										- ((body1->size).y >> 8) * ((body1->matrix).by >> 8))
																										+ ((body1->size).z >> 8) * ((body1->matrix).cy >> 8);
																								col_position->z = (((body1->position).z
																										+ ((body1->size).x >> 8) * ((body1->matrix).az >> 8))
																										- ((body1->size).y >> 8) * ((body1->matrix).bz >> 8))
																										+ ((body1->matrix).cz >> 8) * ((body1->size).z >> 8);
																							}
																						} else {
																							col_position->x = ((body1->position).x + ((body1->matrix).ax >> 8) * ((body1->size).x >> 8)
																									+ ((body1->matrix).bx >> 8) * ((body1->size).y >> 8))
																									- ((body1->size).z >> 8) * ((body1->matrix).cx >> 8);
																							col_position->y = ((body1->position).y + ((body1->size).x >> 8) * ((body1->matrix).ay >> 8)
																									+ ((body1->size).y >> 8) * ((body1->matrix).by >> 8))
																									- ((body1->size).z >> 8) * ((body1->matrix).cy >> 8);
																							col_position->z = (((body1->size).y >> 8) * ((body1->matrix).bz >> 8) + (body1->position).z
																									+ ((body1->size).x >> 8) * ((body1->matrix).az >> 8))
																									- ((body1->matrix).cz >> 8) * ((body1->size).z >> 8);
																						}
																					} else {
																						col_position->x = (body1->position).x + ((body1->matrix).ax >> 8) * ((body1->size).x >> 8)
																								+ ((body1->matrix).bx >> 8) * ((body1->size).y >> 8)
																								+ ((body1->size).z >> 8) * ((body1->matrix).cx >> 8);
																						col_position->y = (body1->position).y + ((body1->size).x >> 8) * ((body1->matrix).ay >> 8)
																								+ ((body1->size).y >> 8) * ((body1->matrix).by >> 8)
																								+ ((body1->size).z >> 8) * ((body1->matrix).cy >> 8);
																						col_position->z = ((body1->size).y >> 8) * ((body1->matrix).bz >> 8) + (body1->position).z
																								+ ((body1->size).x >> 8) * ((body1->matrix).az >> 8)
																								+ ((body1->matrix).cz >> 8) * ((body1->size).z >> 8);
																					}
																					iVar2 = (body2->speed).x;
																					iVar3 = (body1->speed).x;
																					if (iVar2 == iVar3 || iVar2 - iVar3 < 0) {
																						local_a8 = -((body2->speed).x - (body1->speed).x);
																					} else {
																						local_a8 = (body2->speed).x - (body1->speed).x;
																					}
																					iVar2 = (body2->speed).y;
																					iVar3 = (body1->speed).y;
																					if (iVar2 == iVar3 || iVar2 - iVar3 < 0) {
																						local_a4 = -((body2->speed).y - (body1->speed).y);
																					} else {
																						local_a4 = (body2->speed).y - (body1->speed).y;
																					}
																					iVar2 = (body2->speed).z;
																					iVar3 = (body1->speed).z;
																					if (iVar2 == iVar3 || iVar2 - iVar3 < 0) {
																						local_a0 = -((body2->speed).z - (body1->speed).z);
																					} else {
																						local_a0 = (body2->speed).z - (body1->speed).z;
																					}
																					local_b8 = local_a8;
																					if (local_a8 < local_a4) {
																						local_b8 = local_a4;
																					}
																					if (local_b8 < local_a0) {
																						local_b8 = local_a0;
																					}
																					_DAT_001449cc = (uint) (0xf0000 < local_b8);
																					if (_DAT_001449cc == 0) {
																						if (local_1c < 0) {
																							local_10c = (body2->size).x + local_1c;
																						} else {
																							local_10c = (body2->size).x - local_1c;
																						}
																						if (local_18 < 0) {
																							local_108 = (body2->size).y + local_18;
																						} else {
																							local_108 = (body2->size).y - local_18;
																						}
																						if (local_14 < 0) {
																							local_104 = (body2->size).z + local_14;
																						} else {
																							local_104 = (body2->size).z - local_14;
																						}
																						if ((local_10c < local_108) && (local_10c < local_104)) {
																							col_direction->x = (body2->matrix).ax;
																							col_direction->y = (body2->matrix).ay;
																							col_direction->z = (body2->matrix).az;
																							if (local_1c < 0) {
																								col_direction->x = -col_direction->x;
																								col_direction->y = -col_direction->y;
																								col_direction->z = -col_direction->z;
																							}
																						} else if (local_108 < local_104) {
																							col_direction->x = (body2->matrix).bx;
																							col_direction->y = (body2->matrix).by;
																							col_direction->z = (body2->matrix).bz;
																							if (local_18 < 0) {
																								col_direction->x = -col_direction->x;
																								col_direction->y = -col_direction->y;
																								col_direction->z = -col_direction->z;
																							}
																						} else {
																							col_direction->x = (body2->matrix).cx;
																							col_direction->y = (body2->matrix).cy;
																							col_direction->z = (body2->matrix).cz;
																							if (local_14 < 0) {
																								col_direction->x = -col_direction->x;
																								col_direction->y = -col_direction->y;
																								col_direction->z = -col_direction->z;
																							}
																						}
																					} else {
																						local_dc = (body2->matrix).ax;
																						local_d8 = (body2->matrix).ay;
																						local_d4 = (body2->matrix).az;
																						local_e8 = (body2->matrix).bx;
																						local_e4 = (body2->matrix).by;
																						local_e0 = (body2->matrix).bz;
																						local_f4 = (body2->matrix).cx;
																						local_f0 = (body2->matrix).cy;
																						local_ec = (body2->matrix).cz;
																						if (local_1c < 0) {
																							local_dc = -local_dc;
																							local_d8 = -local_d8;
																							local_d4 = -local_d4;
																						}
																						if (local_18 < 0) {
																							local_e8 = -local_e8;
																							local_e4 = -local_e4;
																							local_e0 = -local_e0;
																						}
																						if (local_14 < 0) {
																							local_f4 = -local_f4;
																							local_f0 = -local_f0;
																							local_ec = -local_ec;
																						}
																						iVar2 = (body1->speed).y >> 8;
																						iVar3 = (body1->speed).x >> 8;
																						iVar4 = (body1->speed).z >> 8;
																						local_100 = (local_d8 >> 8) * iVar2 + iVar3 * (local_dc >> 8) + iVar4 * (local_d4 >> 8);
																						local_fc = (local_e4 >> 8) * iVar2 + iVar3 * (local_e8 >> 8) + iVar4 * (local_e0 >> 8);
																						local_f8 = (local_f0 >> 8) * iVar2 + iVar3 * (local_f4 >> 8) + iVar4 * (local_ec >> 8);
																						if (local_100 < 1) {
																							local_100 = -local_100;
																						}
																						if (local_fc < 1) {
																							local_fc = -local_fc;
																						}
																						if (local_f8 < 1) {
																							local_f8 = -local_f8;
																						}
																						if ((local_fc < local_100) && (local_f8 < local_100)) {
																							col_direction->x = local_dc;
																							col_direction->y = local_d8;
																							col_direction->z = local_d4;
																						} else if (local_f8 < local_fc) {
																							col_direction->x = local_e8;
																							col_direction->y = local_e4;
																							col_direction->z = local_e0;
																						} else {
																							col_direction->x = local_f4;
																							col_direction->y = local_f0;
																							col_direction->z = local_ec;
																						}
																					}
																					return 1;
																				}
																				col_position->x = (((body2->position).x - ((body2->matrix).ax >> 8) * ((body2->size).x >> 8))
																						- ((body2->matrix).bx >> 8) * ((body2->size).y >> 8))
																						- ((body2->size).z >> 8) * ((body2->matrix).cx >> 8);
																				col_position->y = (((body2->position).y - ((body2->size).x >> 8) * ((body2->matrix).ay >> 8))
																						- ((body2->size).y >> 8) * ((body2->matrix).by >> 8))
																						- ((body2->size).z >> 8) * ((body2->matrix).cy >> 8);
																				col_position->z = (((body2->position).z - ((body2->size).x >> 8) * ((body2->matrix).az >> 8))
																						- ((body2->size).y >> 8) * ((body2->matrix).bz >> 8))
																						- ((body2->matrix).cz >> 8) * ((body2->size).z >> 8);
																			} else {
																				col_position->x = (((body2->position).x - ((body2->matrix).ax >> 8) * ((body2->size).x >> 8))
																						- ((body2->matrix).bx >> 8) * ((body2->size).y >> 8))
																						+ ((body2->size).z >> 8) * ((body2->matrix).cx >> 8);
																				col_position->y = (((body2->position).y - ((body2->size).x >> 8) * ((body2->matrix).ay >> 8))
																						- ((body2->size).y >> 8) * ((body2->matrix).by >> 8))
																						+ ((body2->size).z >> 8) * ((body2->matrix).cy >> 8);
																				col_position->z = (((body2->position).z - ((body2->size).x >> 8) * ((body2->matrix).az >> 8))
																						- ((body2->size).y >> 8) * ((body2->matrix).bz >> 8))
																						+ ((body2->matrix).cz >> 8) * ((body2->size).z >> 8);
																			}
																		} else {
																			col_position->x = (((body2->position).x - ((body2->matrix).ax >> 8) * ((body2->size).x >> 8))
																					+ ((body2->matrix).bx >> 8) * ((body2->size).y >> 8))
																					- ((body2->size).z >> 8) * ((body2->matrix).cx >> 8);
																			col_position->y = (((body2->position).y - ((body2->size).x >> 8) * ((body2->matrix).ay >> 8))
																					+ ((body2->size).y >> 8) * ((body2->matrix).by >> 8))
																					- ((body2->size).z >> 8) * ((body2->matrix).cy >> 8);
																			col_position->z = (((body2->size).y >> 8) * ((body2->matrix).bz >> 8)
																					+ ((body2->position).z - ((body2->size).x >> 8) * ((body2->matrix).az >> 8)))
																					- ((body2->matrix).cz >> 8) * ((body2->size).z >> 8);
																		}
																	} else {
																		col_position->x = ((body2->position).x - ((body2->matrix).ax >> 8) * ((body2->size).x >> 8))
																				+ ((body2->matrix).bx >> 8) * ((body2->size).y >> 8)
																				+ ((body2->size).z >> 8) * ((body2->matrix).cx >> 8);
																		col_position->y = ((body2->position).y - ((body2->size).x >> 8) * ((body2->matrix).ay >> 8))
																				+ ((body2->size).y >> 8) * ((body2->matrix).by >> 8)
																				+ ((body2->size).z >> 8) * ((body2->matrix).cy >> 8);
																		col_position->z = ((body2->size).y >> 8) * ((body2->matrix).bz >> 8)
																				+ ((body2->position).z - ((body2->size).x >> 8) * ((body2->matrix).az >> 8))
																				+ ((body2->matrix).cz >> 8) * ((body2->size).z >> 8);
																	}
																} else {
																	col_position->x = (((body2->position).x + ((body2->matrix).ax >> 8) * ((body2->size).x >> 8))
																			- ((body2->matrix).bx >> 8) * ((body2->size).y >> 8))
																			- ((body2->size).z >> 8) * ((body2->matrix).cx >> 8);
																	col_position->y = (((body2->position).y + ((body2->size).x >> 8) * ((body2->matrix).ay >> 8))
																			- ((body2->size).y >> 8) * ((body2->matrix).by >> 8))
																			- ((body2->size).z >> 8) * ((body2->matrix).cy >> 8);
																	col_position->z = (((body2->position).z + ((body2->size).x >> 8) * ((body2->matrix).az >> 8))
																			- ((body2->size).y >> 8) * ((body2->matrix).bz >> 8))
																			- ((body2->matrix).cz >> 8) * ((body2->size).z >> 8);
																}
															} else {
																col_position->x = (((body2->position).x + ((body2->matrix).ax >> 8) * ((body2->size).x >> 8))
																		- ((body2->matrix).bx >> 8) * ((body2->size).y >> 8)) + ((body2->size).z >> 8) * ((body2->matrix).cx >> 8);
																col_position->y = (((body2->position).y + ((body2->size).x >> 8) * ((body2->matrix).ay >> 8))
																		- ((body2->size).y >> 8) * ((body2->matrix).by >> 8)) + ((body2->size).z >> 8) * ((body2->matrix).cy >> 8);
																col_position->z = (((body2->position).z + ((body2->size).x >> 8) * ((body2->matrix).az >> 8))
																		- ((body2->size).y >> 8) * ((body2->matrix).bz >> 8)) + ((body2->matrix).cz >> 8) * ((body2->size).z >> 8);
															}
														} else {
															col_position->x = ((body2->position).x + ((body2->matrix).ax >> 8) * ((body2->size).x >> 8)
																	+ ((body2->matrix).bx >> 8) * ((body2->size).y >> 8)) - ((body2->size).z >> 8) * ((body2->matrix).cx >> 8);
															col_position->y = ((body2->position).y + ((body2->size).x >> 8) * ((body2->matrix).ay >> 8)
																	+ ((body2->size).y >> 8) * ((body2->matrix).by >> 8)) - ((body2->size).z >> 8) * ((body2->matrix).cy >> 8);
															col_position->z = (((body2->size).y >> 8) * ((body2->matrix).bz >> 8) + (body2->position).z
																	+ ((body2->size).x >> 8) * ((body2->matrix).az >> 8)) - ((body2->matrix).cz >> 8) * ((body2->size).z >> 8);
														}
													} else {
														col_position->x = (body2->position).x + ((body2->matrix).ax >> 8) * ((body2->size).x >> 8)
																+ ((body2->matrix).bx >> 8) * ((body2->size).y >> 8) + ((body2->size).z >> 8) * ((body2->matrix).cx >> 8);
														col_position->y = (body2->position).y + ((body2->size).x >> 8) * ((body2->matrix).ay >> 8)
																+ ((body2->size).y >> 8) * ((body2->matrix).by >> 8) + ((body2->size).z >> 8) * ((body2->matrix).cy >> 8);
														col_position->z = ((body2->size).y >> 8) * ((body2->matrix).bz >> 8) + (body2->position).z
																+ ((body2->size).x >> 8) * ((body2->matrix).az >> 8) + ((body2->matrix).cz >> 8) * ((body2->size).z >> 8);
													}
													iVar2 = (body2->speed).x;
													iVar3 = (body1->speed).x;
													if (iVar2 == iVar3 || iVar2 - iVar3 < 0) {
														local_38 = -((body2->speed).x - (body1->speed).x);
													} else {
														local_38 = (body2->speed).x - (body1->speed).x;
													}
													iVar2 = (body2->speed).y;
													iVar3 = (body1->speed).y;
													if (iVar2 == iVar3 || iVar2 - iVar3 < 0) {
														local_34 = -((body2->speed).y - (body1->speed).y);
													} else {
														local_34 = (body2->speed).y - (body1->speed).y;
													}
													iVar2 = (body2->speed).z;
													iVar3 = (body1->speed).z;
													if (iVar2 == iVar3 || iVar2 - iVar3 < 0) {
														local_30 = -((body2->speed).z - (body1->speed).z);
													} else {
														local_30 = (body2->speed).z - (body1->speed).z;
													}
													local_48 = local_38;
													if (local_38 < local_34) {
														local_48 = local_34;
													}
													if (local_48 < local_30) {
														local_48 = local_30;
													}
													_DAT_001449cc = (uint) (0xf0000 < local_48);
													if (_DAT_001449cc == 0) {
														if (local_1c < 0) {
															local_9c = (body1->size).x + local_1c;
														} else {
															local_9c = (body1->size).x - local_1c;
														}
														if (local_18 < 0) {
															local_98 = (body1->size).y + local_18;
														} else {
															local_98 = (body1->size).y - local_18;
														}
														if (local_14 < 0) {
															local_94 = (body1->size).z + local_14;
														} else {
															local_94 = (body1->size).z - local_14;
														}
														if ((local_9c < local_98) && (local_9c < local_94)) {
															col_direction->x = (body1->matrix).ax;
															col_direction->y = (body1->matrix).ay;
															col_direction->z = (body1->matrix).az;
															if (0 < local_1c) {
																col_direction->x = -col_direction->x;
																col_direction->y = -col_direction->y;
																col_direction->z = -col_direction->z;
															}
														} else if (local_98 < local_94) {
															col_direction->x = (body1->matrix).bx;
															col_direction->y = (body1->matrix).by;
															col_direction->z = (body1->matrix).bz;
															if (0 < local_18) {
																col_direction->x = -col_direction->x;
																col_direction->y = -col_direction->y;
																col_direction->z = -col_direction->z;
															}
														} else {
															col_direction->x = (body1->matrix).cx;
															col_direction->y = (body1->matrix).cy;
															col_direction->z = (body1->matrix).cz;
															if (0 < local_14) {
																col_direction->x = -col_direction->x;
																col_direction->y = -col_direction->y;
																col_direction->z = -col_direction->z;
															}
														}
													} else {
														local_6c = (body1->matrix).ax;
														local_68 = (body1->matrix).ay;
														local_64 = (body1->matrix).az;
														local_78 = (body1->matrix).bx;
														local_74 = (body1->matrix).by;
														local_70 = (body1->matrix).bz;
														local_84 = (body1->matrix).cx;
														local_80 = (body1->matrix).cy;
														local_7c = (body1->matrix).cz;
														if (0 < local_1c) {
															local_6c = -local_6c;
															local_68 = -local_68;
															local_64 = -local_64;
														}
														if (0 < local_18) {
															local_78 = -local_78;
															local_74 = -local_74;
															local_70 = -local_70;
														}
														if (0 < local_14) {
															local_84 = -local_84;
															local_80 = -local_80;
															local_7c = -local_7c;
														}
														iVar2 = (body2->speed).y >> 8;
														iVar3 = (body2->speed).x >> 8;
														iVar4 = (body2->speed).z >> 8;
														local_90 = (local_68 >> 8) * iVar2 + iVar3 * (local_6c >> 8) + iVar4 * (local_64 >> 8);
														local_8c = (local_74 >> 8) * iVar2 + iVar3 * (local_78 >> 8) + iVar4 * (local_70 >> 8);
														local_88 = (local_80 >> 8) * iVar2 + iVar3 * (local_84 >> 8) + iVar4 * (local_7c >> 8);
														if (local_90 < 1) {
															local_90 = -local_90;
														}
														if (local_8c < 1) {
															local_8c = -local_8c;
														}
														if (local_88 < 1) {
															local_88 = -local_88;
														}
														if ((local_8c < local_90) && (local_88 < local_90)) {
															col_direction->x = local_6c;
															col_direction->y = local_68;
															col_direction->z = local_64;
														} else if (local_88 < local_8c) {
															col_direction->x = local_78;
															col_direction->y = local_74;
															col_direction->z = local_70;
														} else {
															col_direction->x = local_84;
															col_direction->y = local_80;
															col_direction->z = local_7c;
														}
													}
												} else {
													col_direction->x = -(body1->matrix).cx;
													col_direction->x = -col_direction->x;
													col_direction->y = -(body1->matrix).cy;
													col_direction->y = -col_direction->y;
													col_direction->z = -(body1->matrix).cz;
													col_direction->z = -col_direction->z;
													col_position->x = (body1->position).x - ((body1->matrix).cx >> 8) * ((body1->size).z >> 8);
													col_position->y = (body1->position).y - ((body1->matrix).cy >> 8) * ((body1->size).z >> 8);
													col_position->z = (body1->position).z - ((body1->size).z >> 8) * ((body1->matrix).cz >> 8);
												}
											} else {
												col_direction->x = -(body1->matrix).bx;
												col_direction->x = -col_direction->x;
												col_direction->y = -(body1->matrix).by;
												col_direction->y = -col_direction->y;
												col_direction->z = -(body1->matrix).bz;
												col_direction->z = -col_direction->z;
												col_position->x = (body1->position).x - ((body1->matrix).bx >> 8) * ((body1->size).y >> 8);
												col_position->y = (body1->position).y - ((body1->matrix).by >> 8) * ((body1->size).y >> 8);
												col_position->z = (body1->position).z - ((body1->matrix).bz >> 8) * ((body1->size).y >> 8);
											}
										} else {
											col_direction->x = -(body1->matrix).ax;
											col_direction->x = -col_direction->x;
											col_direction->y = -(body1->matrix).ay;
											col_direction->y = -col_direction->y;
											col_direction->z = -(body1->matrix).az;
											col_direction->z = -col_direction->z;
											col_position->x = (body1->position).x - ((body1->size).x >> 8) * ((body1->matrix).ax >> 8);
											col_position->y = (body1->position).y - ((body1->matrix).ay >> 8) * ((body1->size).x >> 8);
											col_position->z = (body1->position).z - ((body1->matrix).az >> 8) * ((body1->size).x >> 8);
										}
									} else {
										col_direction->x = (body1->matrix).cx;
										col_direction->x = -col_direction->x;
										col_direction->y = (body1->matrix).cy;
										col_direction->y = -col_direction->y;
										col_direction->z = (body1->matrix).cz;
										col_direction->z = -col_direction->z;
										col_position->x = (body1->position).x + ((body1->matrix).cx >> 8) * ((body1->size).z >> 8);
										col_position->y = (body1->position).y + ((body1->matrix).cy >> 8) * ((body1->size).z >> 8);
										col_position->z = (body1->position).z + ((body1->size).z >> 8) * ((body1->matrix).cz >> 8);
									}
								} else {
									col_direction->x = (body1->matrix).bx;
									col_direction->x = -col_direction->x;
									col_direction->y = (body1->matrix).by;
									col_direction->y = -col_direction->y;
									col_direction->z = (body1->matrix).bz;
									col_direction->z = -col_direction->z;
									col_position->x = ((body1->matrix).bx >> 8) * ((body1->size).y >> 8) + (body1->position).x;
									col_position->y = ((body1->matrix).by >> 8) * ((body1->size).y >> 8) + (body1->position).y;
									col_position->z = (body1->position).z + ((body1->matrix).bz >> 8) * ((body1->size).y >> 8);
								}
							} else {
								col_direction->x = (body1->matrix).ax;
								col_direction->x = -col_direction->x;
								col_direction->y = (body1->matrix).ay;
								col_direction->y = -col_direction->y;
								col_direction->z = (body1->matrix).az;
								col_direction->z = -col_direction->z;
								col_position->x = ((body1->size).x >> 8) * ((body1->matrix).ax >> 8) + (body1->position).x;
								col_position->y = ((body1->matrix).ay >> 8) * ((body1->size).x >> 8) + (body1->position).y;
								col_position->z = (body1->position).z + ((body1->matrix).az >> 8) * ((body1->size).x >> 8);
							}
						} else {
							col_direction->x = (body2->matrix).cx;
							col_direction->x = -col_direction->x;
							col_direction->y = (body2->matrix).cy;
							col_direction->y = -col_direction->y;
							col_direction->z = (body2->matrix).cz;
							col_direction->z = -col_direction->z;
							col_position->x = (body2->position).x - ((body2->matrix).cx >> 8) * ((body2->size).z >> 8);
							col_position->y = (body2->position).y - ((body2->matrix).cy >> 8) * ((body2->size).z >> 8);
							col_position->z = (body2->position).z - ((body2->size).z >> 8) * ((body2->matrix).cz >> 8);
						}
					} else {
						col_direction->x = -(body2->matrix).bx;
						col_direction->y = -(body2->matrix).by;
						col_direction->z = -(body2->matrix).bz;
						col_position->x = (body2->position).x - ((body2->matrix).bx >> 8) * ((body2->size).y >> 8);
						col_position->y = (body2->position).y - ((body2->matrix).by >> 8) * ((body2->size).y >> 8);
						col_position->z = (body2->position).z - ((body2->matrix).bz >> 8) * ((body2->size).y >> 8);
					}
				} else {
					col_direction->x = -(body2->matrix).ax;
					col_direction->y = -(body2->matrix).ay;
					col_direction->z = -(body2->matrix).az;
					col_position->x = (body2->position).x - ((body2->size).x >> 8) * ((body2->matrix).ax >> 8);
					col_position->y = (body2->position).y - ((body2->matrix).ay >> 8) * ((body2->size).x >> 8);
					col_position->z = (body2->position).z - ((body2->matrix).az >> 8) * ((body2->size).x >> 8);
				}
			} else {
				col_direction->x = (body2->matrix).cx;
				col_direction->y = (body2->matrix).cy;
				col_direction->z = (body2->matrix).cz;
				col_position->x = (body2->position).x + ((body2->matrix).cx >> 8) * ((body2->size).z >> 8);
				col_position->y = (body2->position).y + ((body2->matrix).cy >> 8) * ((body2->size).z >> 8);
				col_position->z = (body2->position).z + ((body2->size).z >> 8) * ((body2->matrix).cz >> 8);
			}
		} else {
			col_direction->x = (body2->matrix).bx;
			col_direction->y = (body2->matrix).by;
			col_direction->z = (body2->matrix).bz;
			col_position->x = ((body2->matrix).bx >> 8) * ((body2->size).y >> 8) + (body2->position).x;
			col_position->y = ((body2->matrix).by >> 8) * ((body2->size).y >> 8) + (body2->position).y;
			col_position->z = (body2->position).z + ((body2->matrix).bz >> 8) * ((body2->size).y >> 8);
		}
	} else {
		col_direction->x = (body2->matrix).ax;
		col_direction->y = (body2->matrix).ay;
		col_direction->z = (body2->matrix).az;
		col_position->x = ((body2->size).x >> 8) * ((body2->matrix).ax >> 8) + (body2->position).x;
		col_position->y = ((body2->matrix).ay >> 8) * ((body2->size).x >> 8) + (body2->position).y;
		col_position->z = (body2->position).z + ((body2->matrix).az >> 8) * ((body2->size).x >> 8);
	}
	return 1;
}



int DAT_800eae1c = 0;
int DAT_800eae20 = 0;
int DAT_800eae24 = 0;
int DAT_800eae30 = 0;
int DAT_800eae40 = 0;
int DAT_800eae48 = 0;
int DAT_800eae4c = 0;
int DAT_800eae50 = 0;
int DAT_800eae54 = 0;

int tnfs_collision_carcar_box_detect(tnfs_collision_data *car1, tnfs_collision_data *car2, tnfs_vec3 *col_position, tnfs_vec3 *col_direction) {
	char bVar1;
	uint uVar2;
	int piVar3;
	int iVar4;
	int piVar5;
	int iVar6;
	int aux;
	int iVar7;
	int iVar8;
	int iVar9;
	int iVar10;
	int iVar11;
	int iVar12;
	int iVar13;
	tnfs_vec9 local_100;
	tnfs_vec9 local_d8;
	tnfs_vec9 local_b0;
	tnfs_vec3 local_88;
	tnfs_vec3 local_68;
	tnfs_vec3 local_58;
	tnfs_vec3 local_48;
	tnfs_vec3 local_38;

	iVar13 = (fixmul(car1->matrix.ax, car2->matrix.ax) + fixmul(car1->matrix.ay ,car2->matrix.ay) + fixmul(car1->matrix.az, car2->matrix.az)) >> 8;
	local_100.ax = iVar13 * (car2->size.x >> 8);
	iVar12 = (fixmul(car1->matrix.ax, car2->matrix.bx) + fixmul(car1->matrix.ay, car2->matrix.by) + fixmul(car1->matrix.az, car2->matrix.bz)) >> 8;
	local_100.ay = iVar12 * (car2->size.y >> 8);
	iVar11 = (fixmul(car1->matrix.ax, car2->matrix.cx) + fixmul(car1->matrix.ay, car2->matrix.cy) + fixmul(car1->matrix.az, car2->matrix.cz)) >> 8;
	local_100.az = iVar11 * (car2->size.z >> 8);
	iVar10 = (fixmul(car1->matrix.bx, car2->matrix.ax) + fixmul(car1->matrix.by, car2->matrix.ay) + fixmul(car1->matrix.bz, car2->matrix.az)) >> 8;
	local_100.bx = iVar10 * (car2->size.x >> 8);
	iVar9 = (fixmul(car1->matrix.bx, car2->matrix.bx) + fixmul(car1->matrix.by, car2->matrix.by) + fixmul(car1->matrix.bz, car2->matrix.bz)) >> 8;
	local_100.by = iVar9 * (car2->size.y >> 8);
	iVar8 = (fixmul(car1->matrix.bx, car2->matrix.cx) + fixmul(car1->matrix.by, car2->matrix.cy) + fixmul(car1->matrix.bz, car2->matrix.cz)) >> 8;
	local_100.bz = iVar8 * (car2->size.z >> 8);
	iVar7 = (fixmul(car1->matrix.cx, car2->matrix.ax) + fixmul(car1->matrix.cy, car2->matrix.ay) + fixmul(car1->matrix.cz, car2->matrix.az)) >> 8;
	local_100.cx = iVar7 * (car2->size.x >> 8);
	iVar6 = (fixmul(car1->matrix.cx, car2->matrix.bx) + fixmul(car1->matrix.cy, car2->matrix.by) + fixmul(car1->matrix.cz, car2->matrix.bz)) >> 8;
	local_100.cy = iVar6 * (car2->size.y >> 8);
	iVar4 = (fixmul(car1->matrix.cx, car2->matrix.cx) + fixmul(car1->matrix.cy, car2->matrix.cy) + fixmul(car1->matrix.cz, car2->matrix.cz)) >> 8;
	local_100.cz = iVar4 * (car2->size.z >> 8);
	local_d8.ax = iVar13 * (car1->size.x >> 8);
	local_d8.ay = iVar12 * (car1->size.x >> 8);
	local_d8.az = iVar11 * (car1->size.x >> 8);
	local_d8.bx = iVar10 * (car1->size.y >> 8);
	local_d8.by = iVar9 * (car1->size.y >> 8);
	local_d8.bz = iVar8 * (car1->size.y >> 8);
	local_d8.cx = iVar7 * (car1->size.z >> 8);
	local_d8.cy = iVar6 * (car1->size.z >> 8);
	local_d8.cz = iVar4 * (car1->size.z >> 8);
	iVar4 = 0;
	iVar6 = 0;
	do {
		iVar8 = (car2->position).x - (car1->position).x;
		iVar9 = (car2->position).y - (car1->position).y;
		iVar11 = (car2->position).z - (car1->position).z;
		iVar7 = iVar8 >> 8;
		iVar10 = iVar9 >> 8;
		iVar12 = iVar11 >> 8;
		local_b0.ax = ((car1->matrix).ax >> 8) * iVar7 + ((car1->matrix).ay >> 8) * iVar10 + ((car1->matrix).az >> 8) * iVar12;
		local_b0.ay = ((car1->matrix).bx >> 8) * iVar7 + ((car1->matrix).by >> 8) * iVar10 + ((car1->matrix).bz >> 8) * iVar12;
		local_b0.az = ((car1->matrix).cx >> 8) * iVar7 + ((car1->matrix).cy >> 8) * iVar10 + ((car1->matrix).cz >> 8) * iVar12;
		local_b0.bx = ((car2->matrix).ax >> 8) * iVar7 + ((car2->matrix).ay >> 8) * iVar10 + ((car2->matrix).az >> 8) * iVar12;
		local_b0.by = ((car2->matrix).bx >> 8) * iVar7 + ((car2->matrix).by >> 8) * iVar10 + ((car2->matrix).bz >> 8) * iVar12;
		local_b0.bz = ((car2->matrix).cx >> 8) * iVar7 + ((car2->matrix).cy >> 8) * iVar10 + ((car2->matrix).cz >> 8) * iVar12;
		aux = col_position->x;
		iVar7 = tnfs_collision_carcar_huge_func(car1, car2, col_position, col_direction, &local_100, &local_d8, &local_b0);
		if (iVar7 == 0) {
			return iVar4;
		}
		bVar1 = 0;
		if (iVar4 == 0) {
			iVar4 = 0;
			local_68.x = (car1->speed).x - (car2->speed).x;
			piVar5 = abs(local_68.x);
			local_68.y = (car1->speed).y - (car2->speed).y;
			piVar3 = abs(local_68.y);
			local_68.z = (car1->speed).z - (car2->speed).z;
			if (piVar5 < piVar3) {
				piVar5 = piVar3;
			}
			piVar3 = abs(local_68.z);
			if (piVar5 < piVar3) {
				piVar5 = piVar3;
			}
			if (DAT_800eae40 < piVar5) {
				iVar7 = abs(iVar8);
				iVar10 = abs(iVar9);
				iVar12 = iVar7;
				if (iVar7 < iVar10) {
					iVar12 = iVar10;
				}
				iVar10 = abs(iVar11);
				if (iVar12 < iVar10) {
					iVar12 = iVar10;
				}
				if (iVar10 < iVar7) {
					iVar7 = iVar10;
				}
				if ((iVar12 < DAT_800eae48) && (DAT_800eae1c == 0)) {
					uVar2 = math_vec3_length_XYZ(local_68.x, piVar5, aux);
					iVar4 = math_inverse_value(uVar2);
					bVar1 = 1;
				} else {
					local_48.x = local_68.x;
					local_48.y = local_68.y;
					local_48.z = local_68.z;
					math_vec3_normalize_2(&local_48);
					local_38.x = iVar8;
					local_38.y = iVar9;
					local_38.z = iVar11;
					math_vec3_normalize_fast(&local_38);
					piVar5 = fixmul(local_48.x, local_38.x) + fixmul(local_48.y, local_38.y) + fixmul(local_48.z, local_38.z);
					if (((DAT_800eae50 < piVar5) && (DAT_800eae20 == 0)) || (((DAT_800eae30 != 0 && (DAT_800eae54 < piVar5)) && (iVar7 < DAT_800eae4c)))) {
						bVar1 = 1;
						uVar2 = math_vec3_length_XYZ(local_68.x, piVar5, aux);
						iVar4 = math_inverse_value(uVar2);
					}
				}
			}
			if ((bVar1) || (DAT_800eae24 != 0)) {
				iVar9 = -iVar4 >> 8;
				iVar4 = iVar9 * ((car1->speed).x >> 8);
				iVar7 = iVar9 * ((car1->speed).y >> 8);
				iVar8 = iVar9 * ((car1->speed).z >> 8);
				local_88.x = iVar9 * ((car2->speed).x >> 8) >> 5;
				local_88.y = iVar9 * ((car2->speed).y >> 8) >> 5;
				local_88.z = iVar9 * ((car2->speed).z >> 8) >> 5;
			} else {
				local_58.x = iVar8;
				local_58.y = iVar9;
				local_58.z = iVar11;
				math_vec3_normalize(&local_58);
				iVar4 = car1->field6_0x60 + car2->field6_0x60;
				iVar9 = math_div(car1->field6_0x60, iVar4);
				iVar8 = math_div(car2->field6_0x60, iVar4);
				iVar8 = iVar8 >> 2;
				iVar9 = iVar9 >> 2;
				iVar4 = -(iVar8 * local_58.x >> 0xe);
				iVar7 = -(iVar8 * local_58.y >> 0xe);
				iVar8 = -(iVar8 * local_58.z >> 0xe);
				local_88.x = iVar9 * local_58.x >> 0x13;
				local_88.y = iVar9 * local_58.y >> 0x13;
				local_88.z = iVar9 * local_58.z >> 0x13;
			}
			local_b0.cz = iVar8 >> 5;
			local_b0.cy = iVar7 >> 5;
			local_b0.cx = iVar4 >> 5;
			iVar4 = 1;
		}
		car1->position.x = car1->position.x + local_b0.cx;
		car1->position.y = car1->position.y + local_b0.cy;
		car1->position.z = car1->position.z + local_b0.cz;
		car2->position.x = car2->position.x + local_88.x;
		car2->position.y = car2->position.y + local_88.y;
		iVar6 = iVar6 + 1;
		car2->position.z = car2->position.z + local_88.z;
	} while (iVar6 < 0x20);
	return iVar4;
}


int DAT_800eae58 = 0;
int DAT_800eae70 = 0x8ccc;

/*
 * Apply speed vectors change for a 2-body collision
 */
char tnfs_collision_carcar_rebound(tnfs_collision_data *body1, tnfs_collision_data *body2, tnfs_vec3 * col_position, tnfs_vec3 * col_direction) {
	int iVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	int iVar6;
	tnfs_vec3 local_90;
	tnfs_vec3 tStack_80;
	tnfs_vec3 tStack_70;
	tnfs_vec3 local_60;
	tnfs_vec3 local_50;
	tnfs_vec3 local_40;
	tnfs_vec3 *local_30;

	local_60.x = col_position->x - body1->position.x;
	local_60.y = col_position->y - body1->position.y;
	local_60.z = col_position->z - body1->position.z;
	local_50.x = col_position->x - body2->position.x;
	local_50.y = col_position->y - body2->position.y;
	local_50.z = col_position->z - body2->position.z;
	math_vec3_cross_product(&tStack_80, &local_60, col_direction);
	local_30 = &local_50;
	math_vec3_cross_product(&tStack_70, local_30, col_direction);
	iVar1 = math_vec3_dot(&body1->speed, col_direction);
	iVar2 = math_vec3_dot(&body2->speed, col_direction);
	iVar3 = math_vec3_dot(&body1->angular_speed, &tStack_80);
	iVar4 = math_vec3_dot(&body2->angular_speed, &tStack_70);
	iVar5 = math_vec3_dot(&tStack_80, &tStack_80);
	iVar5 = math_mul(iVar5, body1->angular_acc_factor);
	iVar6 = math_vec3_dot(&tStack_70, &tStack_70);
	iVar6 = math_mul(iVar6, body2->angular_acc_factor);
	iVar1 = math_div(((iVar2 - iVar1) - iVar3) + iVar4, (body1->linear_acc_factor >> 1) + (body2->linear_acc_factor >> 1) + (iVar5 >> 1) + (iVar6 >> 1));
	if (-1 < iVar1) {
		iVar2 = math_mul(DAT_800eae70, iVar1);
		DAT_800eae58 = iVar2;
		local_90.x = math_mul(iVar2, col_direction->x);
		local_90.y = math_mul(iVar2, col_direction->y);
		local_90.z = math_mul(iVar2, col_direction->z);
		local_40.x = math_mul(body1->linear_acc_factor, local_90.x);
		local_40.y = math_mul(body1->linear_acc_factor, local_90.y);
		local_40.z = math_mul(body1->linear_acc_factor, local_90.z);
		body1->speed.x = body1->speed.x + local_40.x;
		body1->speed.y = body1->speed.y + local_40.y;
		body1->speed.z = body1->speed.z + local_40.z;
		local_40.x = math_mul(body2->linear_acc_factor, local_90.x);
		local_40.y = math_mul(body2->linear_acc_factor, local_90.y);
		local_40.z = math_mul(body2->linear_acc_factor, local_90.z);
		body2->speed.x = body2->speed.x - local_40.x;
		body2->speed.y = body2->speed.y - local_40.y;
		body2->speed.z = body2->speed.z - local_40.z;
		math_vec3_cross_product(&local_40, &local_60, &local_90);
		local_40.x = math_mul(body1->angular_acc_factor, local_40.x);
		local_40.y = math_mul(body1->angular_acc_factor, local_40.y);
		local_40.z = math_mul(body1->angular_acc_factor, local_40.z);
		body1->angular_speed.x = body1->angular_speed.x + local_40.x;
		body1->angular_speed.y = body1->angular_speed.y + local_40.y;
		body1->angular_speed.z = body1->angular_speed.z + local_40.z;
		math_vec3_cross_product(&local_40, local_30, &local_90);
		local_40.x = math_mul(body2->angular_acc_factor, local_40.x);
		local_40.y = math_mul(body2->angular_acc_factor, local_40.y);
		iVar2 = math_mul(body2->angular_acc_factor, local_40.z);
		body2->angular_speed.x = body2->angular_speed.x - local_40.x;
		body2->angular_speed.y = body2->angular_speed.y - local_40.y;
		body2->angular_speed.z = body2->angular_speed.z - iVar2;
	}
	return -1 < iVar1;
}

/*
 * enhances the rollover vectors for more cinematic crashes
 */
void tnfs_collision_carcar_exageration(tnfs_car_data * car) {
	car->collision_data.angular_speed.x = (car->collision_data.angular_speed.x >> 8) * 0x59;
	car->collision_data.angular_speed.y = (car->collision_data.angular_speed.y >> 8) * 0x59;
	car->collision_data.angular_speed.z = (car->collision_data.angular_speed.z >> 8) * 0x59;
	car->collision_data.angular_speed.x += (car->road_fence_normal.x >> 8) * -0x3da;
	car->collision_data.angular_speed.y += (car->road_fence_normal.y >> 8) * -0x3da;
	car->collision_data.angular_speed.z += (car->road_fence_normal.z >> 8) * -0x3da;
	car->collision_data.angular_speed.x += (car->road_heading.x >> 8) * -0x1da;
	car->collision_data.angular_speed.y += (car->road_heading.y >> 8) * -0x1da;
	car->collision_data.angular_speed.z += (car->road_heading.z >> 8) * -0x1da;
	car->collision_data.speed.x += (car->collision_data.matrix.bx >> 8) * 0x5b3;
	car->collision_data.speed.y += (car->collision_data.matrix.by >> 8) * 0x5b3;
	car->collision_data.speed.z += (car->collision_data.matrix.bz >> 8) * 0x5b3;
}


tnfs_vec3 g_col_position;
tnfs_vec3 g_col_direction;

/*
 * manage car-to-car collisions - adapted/simplified from TNFS original
 */
void tnfs_collision_carcar() {
	// both cars are near
	if (abs(car_data.position.x - xman_car_data.position.x) < 0x30000
			&& abs(car_data.position.z - xman_car_data.position.z) < 0x30000) {

		g_col_position.x = 0;
		g_col_position.y = 0;
		g_col_position.z = 0;
		g_col_direction.x = 0;
		g_col_direction.y = 0;
		g_col_direction.z = 0;

		// update collision structs
		tnfs_collision_data_set(&car_data);
		tnfs_collision_data_set(&xman_car_data);

		// if collided
		if (tnfs_collision_carcar_box_detect(&car_data.collision_data, &xman_car_data.collision_data, &g_col_position, &g_col_direction)) {

			// bounce off cars
			tnfs_collision_carcar_rebound(&car_data.collision_data, &xman_car_data.collision_data, &g_col_position, &g_col_direction);

			// FIXME xman always wrecked?
			xman_car_data.is_wrecked = 1;
			xman_car_data.field444_0x520 = 4;
			xman_car_data.collision_data.crashed_time = 300;

			if (car_data.speed > 0x100000 && car_data.is_wrecked == 0) {
				// big car wreck
				car_data.is_wrecked = 1;
				car_data.field444_0x520 = 4;
				car_data.collision_data.crashed_time = 300;

				// cinematic crash
				tnfs_track_update_vectors(&car_data);
				tnfs_track_update_vectors(&xman_car_data);
				tnfs_collision_carcar_exageration(&car_data);
				tnfs_collision_carcar_exageration(&xman_car_data);
			}

			if (car_data.is_wrecked == 0) {
				tnfs_collision_data_get(&car_data);
				car_data.speed_x = -car_data.speed_x;
				car_data.speed_z = -car_data.speed_z;
			}
			if (xman_car_data.is_wrecked == 0) {
				tnfs_collision_data_get(&xman_car_data);
				xman_car_data.speed_x = -xman_car_data.speed_x;
				xman_car_data.speed_z = -xman_car_data.speed_z;
			}
		}
	}
}

