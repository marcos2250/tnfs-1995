/*
 * tnfs_math.h
 *
 */

#ifndef TNFS_MATH_H_
#define TNFS_MATH_H_

typedef struct {
	float x;
	float y;
	float z;
} vector3f;

typedef struct {
	int x;
	int y;
	int z;
} tnfs_vec3;

typedef struct {
	int ax;
	int ay;
	int az;
	int bx;
	int by;
	int bz;
	int cx;
	int cy;
	int cz;
} tnfs_vec9;


// fixed point math macros
#define fixmul(x,y) (int)(((long)(x) * (y) + 0x8000) >> 16)
#define fixdiv(x,y) (((x) << 6) / (y))

#define fix16(a) (((a)<0 ? (a) + 0xfffff : (a)) >> 16)
#define fix15(a) (((a)<0 ? (a) + 0x7ffff : (a)) >> 15)
#define fix12(a) (((a)<0 ? (a) + 0xfff : (a)) >> 12)
#define fix8(a)  (((a)<0 ? (a) + 0xff : (a)) >> 8)
#define fix7(a)  (((a)<0 ? (a) + 0x7f : (a)) >> 7)
#define fix6(a)  (((a)<0 ? (a) + 0x3f : (a)) >> 6)
#define fix4(a)  (((a)<0 ? (a) + 0xf : (a)) >> 4)
#define fix3(a)  (((a)<0 ? (a) + 0x7 : (a)) >> 3)
#define fix2(a)  (((a)<0 ? (a) + 3 : (a)) >> 2)

#define abs(a) ((a)<0 ? -(a) : (a))


int math_mul(int x, int y);
int math_div(int x, int y);
int math_inverse_value(int param_1);
int math_sin(int input);
int math_cos(int input);
int math_sin_2(int input);
int math_cos_2(int input);
int math_sin_3(int input);
int math_cos_3(int input);
int math_tan_3(int input);
int math_atan2(int x, int y);
void math_rotate_2d(int x1, int y1, int angle, int *x2, int *y2);
int math_angle_wrap(int a);
void math_matrix_set_rot_Z(tnfs_vec9 *param_1, int angle);
void math_matrix_set_rot_Y(tnfs_vec9 *param_1, int angle);
void math_matrix_set_rot_X(tnfs_vec9 *param_1, int angle);
void math_matrix_transpose(tnfs_vec9 *param_1, tnfs_vec9 *param_2);
void math_matrix_identity(tnfs_vec9 *param_1);
void math_matrix_multiply(tnfs_vec9 *result, tnfs_vec9 *m2, tnfs_vec9 *m1);
void matrix_create_from_pitch_yaw_roll(tnfs_vec9 *result, int pitch, int yaw, int roll);
int math_sqrt(int param_1);
int math_vec3_length_squared(tnfs_vec3 *car_size);
int math_vec3_length(tnfs_vec3 *car_size);
int math_vec3_length_XZ(tnfs_vec3 *param_1);
int math_vec2_dot_product(tnfs_vec3 *v1, tnfs_vec3 *v2);
void math_barycentric_coordinates(tnfs_vec3 *r3, tnfs_vec3 *r2, tnfs_vec3 *r1, tnfs_vec3 *tC, tnfs_vec3 *tB, tnfs_vec3 *tA);
void math_normalize(tnfs_vec3 *v);

#endif /* TNFS_MATH_H_ */
