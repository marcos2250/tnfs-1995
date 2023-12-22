/*
 * tnfs_math.c
 * Fixed point math functions
 */
#include <math.h>
#include "tnfs_math.h"

/*
 * fixed multiplication
 */
int math_mul(int x, int y) {
	return (((long long) x) * y + 0x8000) >> 16;
}

int math_mul_10(int x, int y) {
	return (((long long) x) * y + 0x8000) >> 10;
}

/*
 * fixed division, eg. 0x4b2 / 0x2800 => 0x884e6
 */
int math_div(int x, int y) {
	if (y == 0)
		return 0;
	return (((long long) x) << 14) / y;
}

int math_sin(int input) {
	double a = (double) input / 0x3FFFF;
	return sin(a) * 0x3FFFF;
}

int math_cos(int input) {
	double a = (double) input / 0x3FFFF;
	return cos(a) * 0x3FFFF;
}

int math_sin_2(int input) {
	double a = (double) input / 0xFFFF;
	return sin(a) * 0xFFFF;
}

int math_cos_2(int input) {
	double a = (double) input / 0xFFFF;
	return cos(a) * 0xFFFF;
}

int math_sin_3(int input) {
	double a = (double) input / 2670178;
	return sin(a) * 0xFFFF;
}

int math_cos_3(int input) {
	double a = (double) input / 2670178;
	return cos(a) * 0xFFFF;
}

int math_tan_3(int input) {
	double a = (double) input / 2670178;
	return tan(a) * 0xFFFF;
}

int math_atan2(int x, int y) {
	return atan2(y, x) * 2670217;
}

/*
 * rotate 2d vector
 */
void math_rotate_2d(int x1, int y1, int angle, int *x2, int *y2) {
	double a = (double) angle / 2670178; //angle= 0 to 16777215 (360)
	double s = sin(a);
	double c = cos(a);
	*x2 = (int) (x1 * c - y1 * s);
	*y2 = (int) (x1 * s + y1 * c);
}

int math_angle_wrap(int a) {
	int x = a;
	if (x < 0) {
		x = x + 0x1000000;
	}
	return x & 0xffffff;
}

void math_matrix_set_rot_Z(tnfs_vec9 *param_1, int angle) {
	double a = (double) angle / 2670178;
	int s = sin(a) * 0x10000;
	int c = cos(a) * 0x10000;

	param_1->ax = c;
	param_1->bx = -s;
	param_1->cx = 0;
	param_1->ay = s;
	param_1->by = c;
	param_1->cy = 0;
	param_1->az = 0;
	param_1->bz = 0;
	param_1->cz = 0x10000;
}

void math_matrix_set_rot_Y(tnfs_vec9 *param_1, int angle) {
	double a = (double) angle / 2670178;
	int s = sin(a) * 0x10000;
	int c = cos(a) * 0x10000;

	param_1->ax = c;
	param_1->bx = 0;
	param_1->cx = s;
	param_1->ay = 0;
	param_1->by = 0x10000;
	param_1->cy = 0;
	param_1->az = -s;
	param_1->bz = 0;
	param_1->cz = c;
}

void math_matrix_set_rot_X(tnfs_vec9 *param_1, int angle) {
	double a = (double) angle / 2670178;
	int s = sin(a) * 0x10000;
	int c = cos(a) * 0x10000;

	param_1->ax = 0x10000;
	param_1->bx = 0;
	param_1->cx = 0;
	param_1->ay = 0;
	param_1->by = c;
	param_1->cy = -s;
	param_1->az = 0;
	param_1->bz = s;
	param_1->cz = c;
}

void math_matrix_identity(tnfs_vec9 *param_1) {
	param_1->ax = 0x10000;
	param_1->ay = 0;
	param_1->az = 0;
	param_1->bx = 0;
	param_1->by = 0x10000;
	param_1->bz = 0;
	param_1->cx = 0;
	param_1->cy = 0;
	param_1->cz = 0x10000;
}

void math_matrix_transpose(tnfs_vec9 *param_1, tnfs_vec9 *param_2) {
	param_1->ax = param_2->ax;
	param_1->ay = param_2->bx;
	param_1->az = param_2->cx;
	param_1->bx = param_2->ay;
	param_1->by = param_2->by;
	param_1->bz = param_2->cy;
	param_1->cx = param_2->az;
	param_1->cy = param_2->bz;
	param_1->cz = param_2->cz;
}

void math_matrix_multiply(tnfs_vec9 *result, tnfs_vec9 *m2, tnfs_vec9 *m1) {
	tnfs_vec9 v4;
	tnfs_vec9 *v5;

	v5 = result;
	v4.ax = math_mul(m1->cx, m2->az) + math_mul(m1->bx, m2->ay) + math_mul(m1->ax, m2->ax);
	v4.bx = math_mul(m1->cx, m2->bz) + math_mul(m1->bx, m2->by) + math_mul(m1->ax, m2->bx);
	v4.cx = math_mul(m1->cx, m2->cz) + math_mul(m1->bx, m2->cy) + math_mul(m1->ax, m2->cx);
	v4.ay = math_mul(m1->cy, m2->az) + math_mul(m1->by, m2->ay) + math_mul(m1->ay, m2->ax);
	v4.by = math_mul(m1->cy, m2->bz) + math_mul(m1->by, m2->by) + math_mul(m1->ay, m2->bx);
	v4.cy = math_mul(m1->cy, m2->cz) + math_mul(m1->by, m2->cy) + math_mul(m1->ay, m2->cx);
	v4.az = math_mul(m1->cz, m2->az) + math_mul(m1->bz, m2->ay) + math_mul(m1->az, m2->ax);
	v4.bz = math_mul(m1->cz, m2->bz) + math_mul(m1->bz, m2->by) + math_mul(m1->az, m2->bx);
	v4.cz = math_mul(m1->cz, m2->cz) + math_mul(m1->bz, m2->cy) + math_mul(m1->az, m2->cx);

	memcpy(v5, &v4, sizeof(tnfs_vec9));
}

void matrix_create_from_pitch_yaw_roll(tnfs_vec9 *result, int pitch, int yaw, int roll) {
	tnfs_vec9 tStack_38;

	math_matrix_set_rot_X(result, pitch);
	math_matrix_set_rot_Z(&tStack_38, roll);
	math_matrix_multiply(result, result, &tStack_38);
	math_matrix_set_rot_Y(&tStack_38, yaw);
	math_matrix_multiply(result, result, &tStack_38);
}

/*
 * inverse value f(x) = 1/v
 * 0x2260 => 0x77280
 * 0x15266 => 0xc1aa
 * 0x3a97a => 0x45e8
 */
int math_inverse_value(int v) {
	if (v == 0)
		return 0;
	return 0x100000000 / v;
}

/*
 * square root with 8 bit precision
 */
int math_sqrt(int param_1) {
	return sqrt(param_1) * 0xFF;
}

/*
 * vector3 length squared
 */
int math_vec3_length_squared(tnfs_vec3 *vector) {
	int x, y, z;
	x = math_mul(vector->x, vector->x);
	y = math_mul(vector->y, vector->y);
	z = math_mul(vector->z, vector->z);
	return x + y + z;
}

/*
 * vector3 length
 */
int math_vec3_length(tnfs_vec3 *vector) {
	int x, y, z;
	x = math_mul(vector->x, vector->x);
	y = math_mul(vector->y, vector->y);
	z = math_mul(vector->z, vector->z);
	return sqrt(x + y + z) * 0xFF;
}

/*
 * vector3 length of X & Z values
 */
int math_vec3_length_XZ(tnfs_vec3 *vector) {
	int x, z;
	x = math_mul(vector->x, vector->x);
	z = math_mul(vector->z, vector->z);
	return sqrt(x + z) * 0xFF;
}

void math_vec3_cross_product(tnfs_vec3 *result, tnfs_vec3 *v1, tnfs_vec3 *v2) {
	result->x = fixmul(v2->z, v1->y) - fixmul(v2->y, v1->z);
	result->y = fixmul(v2->x, v1->z) - fixmul(v2->z, v1->x);
	result->z = fixmul(v2->y, v1->x) - fixmul(v2->x, v1->y);
}

int math_vec2_dot_product(tnfs_vec3 *v1, tnfs_vec3 *v2) {
	  int x, z;
	  x = (v2->x - v1->x) >> 8;
	  z = (v2->z - v1->z) >> 8;
	  return x * x + z * z;
}

/*
 * Find Y point for (X,Z) coordinates (r1, r2, r3) projected over a triangle (tA, tB, tC) surface.
 */
void math_barycentric_coordinates(tnfs_vec3 *r3, tnfs_vec3 *r2, tnfs_vec3 *r1, tnfs_vec3 *tC, tnfs_vec3 *tB, tnfs_vec3 *tA) {
	tnfs_vec3 vecAB;
	tnfs_vec3 vecCB;
	tnfs_vec3 cross;
	int denominator;

	// vectors AB and CB
	vecCB.x = tC->x - tB->x;
	vecCB.y = tC->y - tB->y;
	vecCB.z = tC->z - tB->z;
	vecAB.x = tA->x - tB->x;
	vecAB.y = tA->y - tB->y;
	vecAB.z = tA->z - tB->z;

	math_vec3_cross_product(&cross, &vecCB, &vecAB);
	denominator = math_inverse_value(cross.y);

	r3->y = fixmul(-fixmul(cross.z, r3->z) - fixmul(cross.x, r3->x), denominator);
	r2->y = fixmul(-fixmul(cross.z, r2->z) - fixmul(cross.x, r2->x), denominator);
	r1->y = fixmul(-fixmul(cross.z, r1->z - tC->z) - fixmul(cross.x, r1->x - tC->x), denominator) + tC->y;
}

void math_normalize(tnfs_vec3 *v) {
	int d;
	d = fixmul(v->x, v->x) + fixmul(v->y, v->y) + fixmul(v->z, v->x);
	if (d != 0) {
		v->x /= d;
		v->y /= d;
		v->z /= d;
	}
}
