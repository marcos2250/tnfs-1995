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


/* fixed point math macros */

// fixed truncated multiplication
#define fixmul(x,y) (int)(((x)>>8) * ((y)>>8))

#ifdef __GNUC__
// generic round shift functions, PSX version
#define fix15(a) (((a)<0 ? (a) + 0x7fff : (a)) >> 15)
#define fix8(a)  (((a)<0 ? (a) + 0xff : (a)) >> 8)
#define fix7(a)  (((a)<0 ? (a) + 0x7f : (a)) >> 7)
#define fix6(a)  (((a)<0 ? (a) + 0x3f : (a)) >> 6)
#define fix4(a)  (((a)<0 ? (a) + 0xf : (a)) >> 4)
#define fix3(a)  (((a)<0 ? (a) + 0x7 : (a)) >> 3)
#define fix2(a)  (((a)<0 ? (a) + 3 : (a)) >> 2)
#else

// fixed multiplication, PC version
int math_mul(int a, int b);
#pragma aux math_mul = \
"imul edx" \
"add  eax, 0x8000" \
"adc  edx, 0x0" \
"shrd eax, edx, 0x10" \
parm  [ eax ] [ edx ] \
value [ eax ];

/* fixed round shift functions from PC version */
int pfix15(int a, int b);
#pragma aux pfix15 = \
"sar edx, 0x1f" \
"shl edx, 0xf" \
"sbb eax, edx" \
"sar eax, 0xf" \
parm  [ edx ] [ eax ] \
value [ eax ];
#define fix15(a) (pfix15(a, a))

int pfix8(int a, int b);
#pragma aux pfix8 = \
"sar edx, 0x1f" \
"shl edx, 0x8" \
"sbb eax, edx" \
"sar eax, 0x8" \
parm  [ edx ] [ eax ] \
value [ eax ];
#define fix8(a) (pfix8(a, a))

int pfix7(int a, int b);
#pragma aux pfix7 = \
"sar edx, 0x1f" \
"shl edx, 0x7" \
"sbb eax, edx" \
"sar eax, 0x7" \
parm  [ edx ] [ eax ] \
value [ eax ];
#define fix7(a) (pfix7(a, a))

int pfix6(int a, int b);
#pragma aux pfix6 = \
"sar edx, 0x1f" \
"shl edx, 0x6" \
"sbb eax, edx" \
"sar eax, 0x6" \
parm  [ edx ] [ eax ] \
value [ eax ];
#define fix6(a) (pfix6(a, a))

int pfix4(int a, int b);
#pragma aux pfix4 = \
"sar edx, 0x1f" \
"shl edx, 0x4" \
"sbb eax, edx" \
"sar eax, 0x4" \
parm  [ edx ] [ eax ] \
value [ eax ];
#define fix4(a) (pfix4(a, a))

int pfix3(int a, int b);
#pragma aux pfix3 = \
"sar edx, 0x1f" \
"shl edx, 0x3" \
"sbb eax, edx" \
"sar eax, 0x3" \
parm  [ edx ] [ eax ] \
value [ eax ];
#define fix3(a) (pfix3(a, a))

int pfix2(int a, int b);
#pragma aux pfix2 = \
"sar edx, 0x1f" \
"shl edx, 0x2" \
"sbb eax, edx" \
"sar eax, 0x2" \
parm  [ edx ] [ eax ] \
value [ eax ];
#define fix2(a) (pfix2(a, a))

#endif

#define abs(a) ((a)>=0 ? (a) : -(a))


int math_mul(int x, int y);
int math_div(int x, int y);
int math_inverse_value(int param_1);
int math_sin(int input);
int math_cos(int input);
int math_sin_2(int input);
int math_cos_2(int input);
int math_tan_2(int input);
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
int math_vec3_distance_squared_XZ(tnfs_vec3 *v1, tnfs_vec3 *v2);
void math_height_coordinates(tnfs_vec3 *r3, tnfs_vec3 *r2, tnfs_vec3 *r1, tnfs_vec3 *tC, tnfs_vec3 *tB, tnfs_vec3 *tA);
void math_vec3_normalize(tnfs_vec3 *v);

#endif /* TNFS_MATH_H_ */
