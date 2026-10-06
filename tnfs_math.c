/*
 * tnfs_math.c
 * Fixed point math functions
 */
#include <math.h>
#include "tnfs_math.h"

/*
 * fixed multiplication as used in PSX version
 * PC version -> assembly code in math.h
 */
#ifndef __WATCOMC__
int math_mul(int x, int y) {
	return (((long long) x) * y + 0x8000) >> 16;
}
#endif

int math_mul_floor(int x, int y) {
	return math_mul(x >> 1, y >> 1) << 2;
}


/*
 * fixed division, eg. 0x2800 / 0x4b2 => 0x884e6
 */
int math_div(int x, int y) {
	if (y == 0)
		return 0;
	return (((long long) x) << 16) / y;
}

/*
 * trigonometric functions, 2 byte angle  (360 = 0xFFFF)
 */
int math_sin_2(int input) {
	double a = (double) input / 10430;
	return sin(a) * 0xFFFF;
}

int math_cos_2(int input) {
	double a = (double) input / 10430;
	return cos(a) * 0xFFFF;
}

int math_tan_2(int input) {
	double a = (double) input / 10430;
	return tan(a) * 0xFFFF;
}

/*
 * trigonometric functions, 3 byte angle (360 = 0xFFFFFF)
 */
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
	return atan2(y, x) * 2670178;
}

int math_angle14_32(short input) {
	int a = input;
	if (a > 8192)
		a -= 16384;
	return a * 0x400;
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

void math_rotate_vector_xz(tnfs_vec3 *v0, tnfs_vec3 *v1, int angle) {
	double a = (double) angle / 2670178;
	double s = sin(a);
	double c = cos(a);
	v1->x = (int) (v0->x * c + v0->z * s);
	v1->y = v0->y;
	v1->z = (int) (v0->z * c - v0->x * s);
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
	tnfs_vec9 mr;

	mr.ax = math_mul(m1->cx, m2->az) + math_mul(m1->bx, m2->ay) + math_mul(m1->ax, m2->ax);
	mr.bx = math_mul(m1->cx, m2->bz) + math_mul(m1->bx, m2->by) + math_mul(m1->ax, m2->bx);
	mr.cx = math_mul(m1->cx, m2->cz) + math_mul(m1->bx, m2->cy) + math_mul(m1->ax, m2->cx);
	mr.ay = math_mul(m1->cy, m2->az) + math_mul(m1->by, m2->ay) + math_mul(m1->ay, m2->ax);
	mr.by = math_mul(m1->cy, m2->bz) + math_mul(m1->by, m2->by) + math_mul(m1->ay, m2->bx);
	mr.cy = math_mul(m1->cy, m2->cz) + math_mul(m1->by, m2->cy) + math_mul(m1->ay, m2->cx);
	mr.az = math_mul(m1->cz, m2->az) + math_mul(m1->bz, m2->ay) + math_mul(m1->az, m2->ax);
	mr.bz = math_mul(m1->cz, m2->bz) + math_mul(m1->bz, m2->by) + math_mul(m1->az, m2->bx);
	mr.cz = math_mul(m1->cz, m2->cz) + math_mul(m1->bz, m2->cy) + math_mul(m1->az, m2->cx);

	memcpy(result, &mr, sizeof(tnfs_vec9));
}

/*
 * inverse value f(x) = 1/x
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

int math_vec3_length_XYZ(int px, int py, int pz) {
	int x, y, z;
	x = math_mul(px, px);
	y = math_mul(py, py);
	z = math_mul(pz, pz);
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

int math_vec3_distance_XZ(tnfs_vec3 *v1, tnfs_vec3 *v2) {
	int x, z;
	x = (v2->x - v1->x) >> 8;
	z = (v2->z - v1->z) >> 8;
	return sqrt(x * x + z * z);
}

int math_vec3_distance_squared_XZ(tnfs_vec3 *v1, tnfs_vec3 *v2) {
	int x, z;
	x = (v2->x - v1->x) >> 8;
	z = (v2->z - v1->z) >> 8;
	return x * x + z * z;
}

void math_vec3_cross_product(tnfs_vec3 *result, tnfs_vec3 *v1, tnfs_vec3 *v2) {
	result->x = math_mul(v1->y, v2->z) - math_mul(v1->z, v2->y);
	result->y = math_mul(v1->z, v2->x) - math_mul(v1->x, v2->z);
	result->z = math_mul(v1->x, v2->y) - math_mul(v1->y, v2->x);
}

void math_vec3_normalize(tnfs_vec3 *v) {
	int d;
	d = math_mul(v->x, v->x) + math_mul(v->y, v->y) + math_mul(v->z, v->z);
	d = math_sqrt(d);
	if (d != 0) {
		d = math_inverse_value(d);
		v->x = math_mul(v->x, d);
		v->y = math_mul(v->y, d);
		v->z = math_mul(v->z, d);
	}
}

void math_vec3_normalize_2(tnfs_vec3 *input) {
	math_vec3_normalize(input);
}

void math_vec3_normalize_fast(tnfs_vec3 *v) {
	int d;
	d = fixmul(v->x, v->x) + fixmul(v->y, v->y) + fixmul(v->z, v->z);
	d = math_sqrt(d);
	if (d != 0) {
		d = math_inverse_value(d);
		v->x = fixmul(v->x, d);
		v->y = fixmul(v->y, d);
		v->z = fixmul(v->z, d);
	}
}

int math_vec3_dot(tnfs_vec3 *v1, tnfs_vec3 *v2) {
	int iVar1;
	int iVar2;
	int iVar3;
	iVar1 = math_mul(v1->x, v2->x);
	iVar2 = math_mul(v1->y, v2->y);
	iVar3 = math_mul(v1->z, v2->z);
	return iVar1 + iVar2 + iVar3;
}

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

/*
 * result = row vector (m1->ax..az) multiplied by matrix m2
 * DOS 0x50163, PSX 0x80026b54
 */
void math_matrix_to_vec3(tnfs_vec3 *result, tnfs_vec9 *m1, tnfs_vec9 *m2) {
	result->x = math_mul(m1->ax, m2->ax) + math_mul(m1->ay, m2->bx) + math_mul(m1->az, m2->cx);
	result->y = math_mul(m1->ax, m2->ay) + math_mul(m1->ay, m2->by) + math_mul(m1->az, m2->cy);
	result->z = math_mul(m1->ax, m2->az) + math_mul(m1->ay, m2->bz) + math_mul(m1->az, m2->cz);
}

/*
 * extract the angles (X, Y, Z) of a rotation matrix. Used to reposition the car after a crash
 * DOS 0x4fcf9, PSX 0x800268bc
 */
void math_matrix_create_from_XYZ(tnfs_vec9 *matrix, int *angle_x, int *angle_y, int *angle_z) {
	tnfs_vec9 rotY;
	tnfs_vec9 rotZ;
	tnfs_vec9 vec;
	tnfs_vec3 result;
	int ay;
	int az;

	ay = math_atan2(matrix->ax, -matrix->az);
	az = math_atan2(math_sqrt(math_mul(matrix->ax, matrix->ax) + math_mul(matrix->az, matrix->az)), matrix->ay);

	math_matrix_set_rot_Y(&rotY, -ay);
	math_matrix_set_rot_Z(&rotZ, -az);
	math_matrix_multiply(&rotY, &rotY, &rotZ);

	vec.ax = matrix->cx;
	vec.ay = matrix->cy;
	vec.az = matrix->cz;
	math_matrix_to_vec3(&result, &vec, &rotY);

	*angle_z = az;
	*angle_x = math_atan2(result.z, -result.y);
	*angle_y = ay;
}

/*
 * re-normalize the orthogonal axes of a rotation matrix (fixes accumulated drift)
 * DOS 0x4f49d, PSX 0x80026118
 */
void math_matrix_orthonormalize(tnfs_vec9 *m) {
	tnfs_vec9 mat = *m;
	int n;

	n = math_inverse_value(((mat.ax >> 2) * mat.ax >> 0xe) + ((mat.ay >> 2) * mat.ay >> 0xe) + ((mat.az >> 2) * mat.az >> 0xe) + 0x10000 >> 1);
	mat.ax = (mat.ax >> 2) * n >> 0xe;
	mat.ay = (mat.ay >> 2) * n >> 0xe;
	mat.az = (mat.az >> 2) * n >> 0xe;

	mat.cx = ((mat.bz >> 2) * mat.ay >> 0xe) - ((mat.by >> 2) * mat.az >> 0xe);
	mat.cy = ((mat.bx >> 2) * mat.az >> 0xe) - ((mat.bz >> 2) * mat.ax >> 0xe);
	mat.cz = ((mat.by >> 2) * mat.ax >> 0xe) - ((mat.bx >> 2) * mat.ay >> 0xe);

	n = math_inverse_value(((mat.cx >> 2) * mat.cx >> 0xe) + ((mat.cy >> 2) * mat.cy >> 0xe) + ((mat.cz >> 2) * mat.cz >> 0xe) + 0x10000 >> 1);
	mat.cx = (mat.cx >> 2) * n >> 0xe;
	mat.cy = (mat.cy >> 2) * n >> 0xe;
	mat.cz = (mat.cz >> 2) * n >> 0xe;

	mat.bx = ((mat.az >> 2) * mat.cy >> 0xe) - ((mat.ay >> 2) * mat.cz >> 0xe);
	mat.by = ((mat.ax >> 2) * mat.cz >> 0xe) - ((mat.az >> 2) * mat.cx >> 0xe);
	mat.bz = ((mat.ay >> 2) * mat.cx >> 0xe) - ((mat.ax >> 2) * mat.cy >> 0xe);

	*m = mat;
}

void math_matrix_from_pitch_yaw_roll(tnfs_vec9 *result, int pitch, int yaw, int roll) {
	tnfs_vec9 tStack_38;

	math_matrix_set_rot_X(result, pitch);
	math_matrix_set_rot_Z(&tStack_38, roll);
	math_matrix_multiply(result, result, &tStack_38);
	math_matrix_set_rot_Y(&tStack_38, yaw);
	math_matrix_multiply(result, result, &tStack_38);
}

/*
 * Find Y-heights for 3 (X,Z) points (p1, p2, p3) projected over a triangle (tA, tB, tC) surface.
 * TNFS uses this to locate the points for: car center (p1), car front bumper (p2), car side edge (p3);
 * tA, tB, tC are the vertices of the current terrain triangle the car is at.
 */
void math_height_coordinates(tnfs_vec3 *p3, tnfs_vec3 *p2, tnfs_vec3 *p1, tnfs_vec3 *tA, tnfs_vec3 *tB, tnfs_vec3 *tC) {
	tnfs_vec3 vecCB;
	tnfs_vec3 vecAB;
	tnfs_vec3 cross;
	int denominator;

	// define vectors AB and CB
	vecAB.x = tA->x - tB->x;
	vecAB.y = tA->y - tB->y;
	vecAB.z = tA->z - tB->z;
	vecCB.x = tC->x - tB->x;
	vecCB.y = tC->y - tB->y;
	vecCB.z = tC->z - tB->z;

	math_vec3_cross_product(&cross, &vecAB, &vecCB);
	denominator = math_inverse_value(cross.y);

	// calculate y for the 3 requested points
	//p3->y = fixmul(-fixmul(cross.z, p3->z) - fixmul(cross.x, p3->x), denominator);
	//p2->y = fixmul(-fixmul(cross.z, p2->z) - fixmul(cross.x, p2->x), denominator);
	//p1->y = fixmul(-fixmul(cross.z, p1->z - tA->z) - fixmul(cross.x, p1->x - tA->x), denominator) + tA->y;

	// compiler optimized
	cross.x = cross.x >> 8;
	cross.z = cross.z >> 8;
	denominator = denominator >> 8;
	p3->y = ((-((p3->z >> 8) * cross.z) - ((p3->x >> 8) * cross.x)) >> 8) * denominator;
	p2->y = ((-((p2->z >> 8) * cross.z) - ((p2->x >> 8) * cross.x)) >> 8) * denominator;
	p1->y = ((-(((p1->z - tA->z) >> 8) * cross.z) - (((p1->x - tA->x) >> 8) * cross.x)) >> 8) * denominator + tA->y;
}


