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
 * arctangent, an eighth of a turn in 16-bit angles (0x2000) by the ratio small / large, 256 steps + 1
 * (SE 0x4f4074, read by FUN_004890f0)
 */
static int g_atan_table[257] = { //
		0x0, 0x29, 0x51, 0x7a, 0xa3, 0xcc, 0xf4, 0x11d, 0x146, 0x16f, 0x197, 0x1c0, //
		0x1e9, 0x211, 0x23a, 0x262, 0x28b, 0x2b4, 0x2dc, 0x305, 0x32d, 0x356, 0x37e, 0x3a7, //
		0x3cf, 0x3f7, 0x420, 0x448, 0x470, 0x499, 0x4c1, 0x4e9, 0x511, 0x539, 0x561, 0x589, //
		0x5b1, 0x5d9, 0x601, 0x629, 0x651, 0x678, 0x6a0, 0x6c8, 0x6ef, 0x717, 0x73e, 0x766, //
		0x78d, 0x7b5, 0x7dc, 0x803, 0x82a, 0x851, 0x878, 0x89f, 0x8c6, 0x8ed, 0x914, 0x93b, //
		0x961, 0x988, 0x9ae, 0x9d5, 0x9fb, 0xa22, 0xa48, 0xa6e, 0xa94, 0xaba, 0xae0, 0xb06, //
		0xb2c, 0xb51, 0xb77, 0xb9d, 0xbc2, 0xbe7, 0xc0d, 0xc32, 0xc57, 0xc7c, 0xca1, 0xcc6, //
		0xceb, 0xd10, 0xd34, 0xd59, 0xd7d, 0xda2, 0xdc6, 0xdea, 0xe0f, 0xe33, 0xe56, 0xe7a, //
		0xe9e, 0xec2, 0xee5, 0xf09, 0xf2c, 0xf50, 0xf73, 0xf96, 0xfb9, 0xfdc, 0xfff, 0x1021, //
		0x1044, 0x1067, 0x1089, 0x10ab, 0x10ce, 0x10f0, 0x1112, 0x1134, 0x1156, 0x1177, 0x1199, 0x11bb, //
		0x11dc, 0x11fd, 0x121f, 0x1240, 0x1261, 0x1282, 0x12a3, 0x12c3, 0x12e4, 0x1305, 0x1325, 0x1345, //
		0x1366, 0x1386, 0x13a6, 0x13c6, 0x13e6, 0x1405, 0x1425, 0x1444, 0x1464, 0x1483, 0x14a2, 0x14c1, //
		0x14e0, 0x14ff, 0x151e, 0x153d, 0x155b, 0x157a, 0x1598, 0x15b7, 0x15d5, 0x15f3, 0x1611, 0x162f, //
		0x164c, 0x166a, 0x1688, 0x16a5, 0x16c2, 0x16e0, 0x16fd, 0x171a, 0x1737, 0x1754, 0x1770, 0x178d, //
		0x17aa, 0x17c6, 0x17e2, 0x17fe, 0x181b, 0x1837, 0x1853, 0x186e, 0x188a, 0x18a6, 0x18c1, 0x18dd, //
		0x18f8, 0x1913, 0x192e, 0x1949, 0x1964, 0x197f, 0x199a, 0x19b4, 0x19cf, 0x19e9, 0x1a04, 0x1a1e, //
		0x1a38, 0x1a52, 0x1a6c, 0x1a86, 0x1a9f, 0x1ab9, 0x1ad3, 0x1aec, 0x1b05, 0x1b1f, 0x1b38, 0x1b51, //
		0x1b6a, 0x1b83, 0x1b9c, 0x1bb4, 0x1bcd, 0x1be5, 0x1bfe, 0x1c16, 0x1c2e, 0x1c46, 0x1c5e, 0x1c76, //
		0x1c8e, 0x1ca6, 0x1cbe, 0x1cd5, 0x1ced, 0x1d04, 0x1d1b, 0x1d33, 0x1d4a, 0x1d61, 0x1d78, 0x1d8e, //
		0x1da5, 0x1dbc, 0x1dd3, 0x1de9, 0x1dff, 0x1e16, 0x1e2c, 0x1e42, 0x1e58, 0x1e6e, 0x1e84, 0x1e9a, //
		0x1eb0, 0x1ec5, 0x1edb, 0x1ef0, 0x1f06, 0x1f1b, 0x1f30, 0x1f45, 0x1f5a, 0x1f6f, 0x1f84, 0x1f99, //
		0x1fae, 0x1fc3, 0x1fd7, 0x1fec, 0x2000 };

/*
 * arctangent in 1024 steps per turn (0x80 an eighth) by the ratio small / large, 256 steps + 1
 * (SE 0x4f3e9c, read by math_sin 0x484294, which is an arctangent)
 */
static unsigned char g_atan_1024_table[257] = { //
		0x00, 0x01, 0x01, 0x02, 0x03, 0x03, 0x04, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x08, 0x09, 0x0a, //
		0x0a, 0x0b, 0x0b, 0x0c, 0x0d, 0x0d, 0x0e, 0x0f, 0x0f, 0x10, 0x10, 0x11, 0x12, 0x12, 0x13, 0x14, //
		0x14, 0x15, 0x16, 0x16, 0x17, 0x17, 0x18, 0x19, 0x19, 0x1a, 0x1b, 0x1b, 0x1c, 0x1c, 0x1d, 0x1e, //
		0x1e, 0x1f, 0x1f, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x24, 0x24, 0x25, 0x26, 0x26, 0x27, 0x27, //
		0x28, 0x29, 0x29, 0x2a, 0x2a, 0x2b, 0x2c, 0x2c, 0x2d, 0x2d, 0x2e, 0x2e, 0x2f, 0x30, 0x30, 0x31, //
		0x31, 0x32, 0x33, 0x33, 0x34, 0x34, 0x35, 0x35, 0x36, 0x37, 0x37, 0x38, 0x38, 0x39, 0x39, 0x3a, //
		0x3a, 0x3b, 0x3c, 0x3c, 0x3d, 0x3d, 0x3e, 0x3e, 0x3f, 0x3f, 0x40, 0x41, 0x41, 0x42, 0x42, 0x43, //
		0x43, 0x44, 0x44, 0x45, 0x45, 0x46, 0x46, 0x47, 0x47, 0x48, 0x48, 0x49, 0x4a, 0x4a, 0x4b, 0x4b, //
		0x4c, 0x4c, 0x4d, 0x4d, 0x4e, 0x4e, 0x4f, 0x4f, 0x50, 0x50, 0x51, 0x51, 0x52, 0x52, 0x53, 0x53, //
		0x54, 0x54, 0x54, 0x55, 0x55, 0x56, 0x56, 0x57, 0x57, 0x58, 0x58, 0x59, 0x59, 0x5a, 0x5a, 0x5b, //
		0x5b, 0x5b, 0x5c, 0x5c, 0x5d, 0x5d, 0x5e, 0x5e, 0x5f, 0x5f, 0x60, 0x60, 0x60, 0x61, 0x61, 0x62, //
		0x62, 0x63, 0x63, 0x63, 0x64, 0x64, 0x65, 0x65, 0x66, 0x66, 0x66, 0x67, 0x67, 0x68, 0x68, 0x68, //
		0x69, 0x69, 0x6a, 0x6a, 0x6a, 0x6b, 0x6b, 0x6c, 0x6c, 0x6c, 0x6d, 0x6d, 0x6e, 0x6e, 0x6e, 0x6f, //
		0x6f, 0x70, 0x70, 0x70, 0x71, 0x71, 0x71, 0x72, 0x72, 0x73, 0x73, 0x73, 0x74, 0x74, 0x74, 0x75, //
		0x75, 0x76, 0x76, 0x76, 0x77, 0x77, 0x77, 0x78, 0x78, 0x78, 0x79, 0x79, 0x79, 0x7a, 0x7a, 0x7a, //
		0x7b, 0x7b, 0x7b, 0x7c, 0x7c, 0x7c, 0x7d, 0x7d, 0x7d, 0x7e, 0x7e, 0x7e, 0x7f, 0x7f, 0x7f, 0x80, //
		0x80 };

/*
 * quarter sine wave, 256 steps + 1 (1024 per turn), PC version (DOS 0x106408, SE 0x4c6b84)
 */
int g_sine_quarter_table[257] = { //
		0x0, 0x192, 0x324, 0x4b6, 0x648, 0x7da, 0x96c, 0xafe, 0xc8f, 0xe21, 0xfb2, 0x1144, //
		0x12d5, 0x1466, 0x15f6, 0x1787, 0x1917, 0x1aa7, 0x1c37, 0x1dc7, 0x1f56, 0x20e5, 0x2273, 0x2402, //
		0x2590, 0x271d, 0x28aa, 0x2a37, 0x2bc4, 0x2d50, 0x2edb, 0x3066, 0x31f1, 0x337b, 0x3505, 0x368e, //
		0x3817, 0x399f, 0x3b26, 0x3cad, 0x3e33, 0x3fb9, 0x413e, 0x42c3, 0x4447, 0x45ca, 0x474d, 0x48ce, //
		0x4a50, 0x4bd0, 0x4d50, 0x4ecf, 0x504d, 0x51ca, 0x5347, 0x54c3, 0x563e, 0x57b8, 0x5931, 0x5aaa, //
		0x5c22, 0x5d98, 0x5f0e, 0x6083, 0x61f7, 0x636a, 0x64dc, 0x664d, 0x67bd, 0x692d, 0x6a9b, 0x6c08, //
		0x6d74, 0x6edf, 0x7049, 0x71b1, 0x7319, 0x7480, 0x75e5, 0x774a, 0x78ad, 0x7a0f, 0x7b70, 0x7cd0, //
		0x7e2e, 0x7f8b, 0x80e7, 0x8242, 0x839c, 0x84f4, 0x864b, 0x87a1, 0x88f5, 0x8a48, 0x8b9a, 0x8cea, //
		0x8e39, 0x8f87, 0x90d3, 0x921e, 0x9368, 0x94b0, 0x95f6, 0x973c, 0x987f, 0x99c1, 0x9b02, 0x9c42, //
		0x9d7f, 0x9ebc, 0x9ff6, 0xa12f, 0xa267, 0xa39d, 0xa4d2, 0xa605, 0xa736, 0xa866, 0xa994, 0xaac0, //
		0xabeb, 0xad14, 0xae3b, 0xaf61, 0xb085, 0xb1a8, 0xb2c8, 0xb3e7, 0xb504, 0xb620, 0xb73a, 0xb852, //
		0xb968, 0xba7c, 0xbb8f, 0xbca0, 0xbdae, 0xbebc, 0xbfc7, 0xc0d0, 0xc1d8, 0xc2de, 0xc3e1, 0xc4e3, //
		0xc5e3, 0xc6e2, 0xc7de, 0xc8d8, 0xc9d1, 0xcac7, 0xcbbb, 0xccae, 0xcd9f, 0xce8d, 0xcf7a, 0xd064, //
		0xd14d, 0xd233, 0xd318, 0xd3fa, 0xd4db, 0xd5b9, 0xd695, 0xd770, 0xd848, 0xd91e, 0xd9f2, 0xdac4, //
		0xdb94, 0xdc61, 0xdd2d, 0xddf6, 0xdebe, 0xdf83, 0xe046, 0xe106, 0xe1c5, 0xe282, 0xe33c, 0xe3f4, //
		0xe4aa, 0xe55e, 0xe60f, 0xe6be, 0xe76b, 0xe816, 0xe8bf, 0xe965, 0xea09, 0xeaab, 0xeb4b, 0xebe8, //
		0xec83, 0xed1c, 0xedb2, 0xee46, 0xeed8, 0xef68, 0xeff5, 0xf080, 0xf109, 0xf18f, 0xf213, 0xf294, //
		0xf314, 0xf391, 0xf40b, 0xf484, 0xf4fa, 0xf56d, 0xf5de, 0xf64d, 0xf6ba, 0xf724, 0xf78b, 0xf7f1, //
		0xf853, 0xf8b4, 0xf912, 0xf96e, 0xf9c7, 0xfa1e, 0xfa72, 0xfac5, 0xfb14, 0xfb61, 0xfbac, 0xfbf5, //
		0xfc3b, 0xfc7e, 0xfcbf, 0xfcfe, 0xfd3a, 0xfd74, 0xfdab, 0xfde0, 0xfe13, 0xfe43, 0xfe70, 0xfe9b, //
		0xfec4, 0xfeea, 0xff0e, 0xff2f, 0xff4e, 0xff6a, 0xff84, 0xff9c, 0xffb1, 0xffc3, 0xffd3, 0xffe1, //
		0xffec, 0xfff4, 0xfffb, 0xfffe, 0x10000 };

/*
 * sine of a 1024 steps per turn angle, from the quarter table
 */
int math_sine_lookup(int i) {
	int x = i & 0xff;
	if (i & 0x100)
		x = 0x100 - x;
	if (i & 0x200)
		return -g_sine_quarter_table[x];
	return g_sine_quarter_table[x];
}

/*
 * trigonometric functions, 2 byte angle  (360 = 0xFFFF)
 * table value plus the derivative times the remaining 6 bits, PC version (DOS 0x9ae90/0x9aefc, SE 0x472fc0/0x473030)
 */
int math_sin_2(int input) {
	int i = input >> 6;
	return math_sine_lookup(i) + ((math_sine_lookup(i + 0x100) >> 2) * (((input & 0x3f) * 0x6487e) >> 9) >> 21);
}

int math_cos_2(int input) {
	int i = input >> 6;
	return math_sine_lookup(i + 0x100) + ((((input & 0x3f) * 0x6487e) >> 9) * (-math_sine_lookup(i) >> 2) >> 21);
}

int math_tan_2(int input) {
	double a = (double) input / 10430;
	return tan(a) * 0xFFFF;
}

/*
 * trigonometric functions, 3 byte angle (360 = 0xFFFFFF)
 */
int math_sin_3(int input) {
	return math_sin_2(input >> 8);
}

int math_cos_3(int input) {
	return math_cos_2(input >> 8);
}

int math_tan_3(int input) {
	double a = (double) input / 2670178;
	return tan(a) * 0xFFFF;
}

/*
 * arctangent of y / x in 3-byte angles (SE math_atan2 0x42ebbc: FUN_004890f0(y, x) << 8; ReNFS ea_atan2): the smaller
 * over the larger as a 32-bit fraction, the table of an eighth of a turn with a linear step between its entries, then
 * the octant
 */
int math_atan2(int x, int y) {
	unsigned int ax, ay, small, large, ratio, i;
	int octant, a;

	octant = 0;
	ax = x;
	ay = y;
	if (x < 0) {
		octant = 2;
		ax = -(unsigned int) x;
	}
	if (y < 0) {
		ay = -(unsigned int) y;
		octant |= 4;
	}
	if (ay == ax) {
		a = 0x2000;
	} else {
		small = ay;
		large = ax;
		if ((int) ax < (int) ay) {
			octant |= 1;
			small = ax;
			large = ay;
		}
		ratio = (unsigned int) (((unsigned long long) small << 32) / large);
		i = ratio >> 24;
		a = g_atan_table[i] + ((unsigned int) ((g_atan_table[i + 1] - g_atan_table[i]) * ((ratio >> 8) & 0xffff)) >> 16);
	}
	switch (octant) {
	case 1:
		a = 0x4000 - a;
		break;
	case 2:
		a = 0x8000 - a;
		break;
	case 3:
		a = a + 0x4000;
		break;
	case 4:
		a = -a;
		break;
	case 5:
		a = a - 0x4000;
		break;
	case 6:
		a = a - 0x8000;
		break;
	case 7:
		a = -a - 0x4000;
		break;
	}
	return (int) ((unsigned int) a << 8);
}

/*
 * arctangent of y / x in 1024 steps per turn, nearest table entry (SE 0x484294, named math_sin in Ghidra, called with
 * (y, x); ReNFS ea_atan_1024). The car tilt (tnfs_height_*) uses it, shifted to 3-byte angles by << 14.
 */
int math_atan_1024(int x, int y) {
	unsigned int ratio, w;
	int flags, t;

	flags = 0;
	if (x < 0) {
		flags = 8;
		x = (int) -(unsigned int) x;
	}
	if (y < 0) {
		flags |= 0x10;
		y = (int) -(unsigned int) y;
	}
	if (y >= x) {
		if (y == x) {
			t = 0x80;
			goto octant;
		}
		w = y;
		y = x;
		x = w;
		flags |= 4;
	}
	ratio = (unsigned int) (((unsigned long long) (unsigned int) y << 32) / (unsigned int) x);
	t = g_atan_1024_table[(ratio >> 24) + ((ratio >> 23) & 1)];
octant:
	switch (flags) {
	case 4:
		return 0x100 - t;
	case 8:
		return 0x200 - t;
	case 0xc:
		return t + 0x100;
	case 0x10:
		return -t;
	case 0x14:
		return t - 0x100;
	case 0x18:
		return t - 0x200;
	case 0x1c:
		return -t - 0x100;
	}
	return t;
}

int math_angle14_32(short input) {
	int a = input;
	if (a > 8192)
		a -= 16384;
	return a * 0x400;
}

/*
 * rotate 2d vector (DOS 0x5929b, SE 0x42ece8)
 */
void math_rotate_2d(int x1, int y1, int angle, int *x2, int *y2) {
	int c = math_cos_2(angle >> 8);
	int s = math_sin_2(angle >> 8);
	*x2 = math_mul(c, x1) - math_mul(s, y1);
	*y2 = math_mul(s, x1) + math_mul(c, y1);
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
	// SE 0x41d83c: the table sine of angle >> 8
	int s = math_sin_2(angle >> 8);
	int c = math_cos_2(angle >> 8);

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
	// SE 0x41d8a4: the table sine of angle >> 8
	int s = math_sin_2(angle >> 8);
	int c = math_cos_2(angle >> 8);

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
	// SE 0x41d90c: the table sine of angle >> 8
	int s = math_sin_2(angle >> 8);
	int c = math_cos_2(angle >> 8);

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
 * square root of a 16.16 value (SE 0x42eb54, DOS 0x8676a; ReNFS fix_sqrt): Newton steps from the value itself until a
 * step changes the root by less than 1 >> 8, as every caller asks (8 bits, guess = the value)
 */
int math_sqrt(int param_1) {
	int x;
	unsigned int old;

	if (param_1 == 0)
		return 0;
	x = param_1;
	do {
		old = x;
		x = (unsigned int) (x + (((unsigned int) (param_1 / x) << 16) + (int) ((((long long) (param_1 % x)) * 0x10000) / x))) >> 1;
	} while ((old - x) >> 8);
	return x;
}

/*
 * vector3 length squared
 */
int math_vec3_length_squared(tnfs_vec3 *vector) {
	// SE 0x41d570: halved until each part is <= 256.0 and the squares fit, then scaled back
	int x, y, z, n;
	unsigned int x2, y2, z2;
	x = vector->x;
	y = vector->y;
	z = vector->z;
	n = 0;
	while (abs(x) > 0x1000000 || abs(y) > 0x1000000 || abs(z) > 0x1000000) {
		x >>= 1;
		y >>= 1;
		z >>= 1;
		n++;
	}
	x2 = math_mul(x, x);
	while (1) {
		y2 = math_mul(y, y);
		z2 = math_mul(z, z);
		if ((z2 >> 2) + (x2 >> 2) + (y2 >> 2) <= 0x40000000)
			break;
		x >>= 1;
		y >>= 1;
		z >>= 1;
		x2 = math_mul(x, x);
		n++;
	}
	x2 = x2 + y2 + z2;
	for (; n != 0; n--)
		x2 <<= 2;
	return x2;
}

/*
 * vector3 length
 */
int math_vec3_length(tnfs_vec3 *vector) {
	// SE 0x41d3b8: halved until each part is <= 256.0 and the squares fit, then scaled back
	int x, y, z, n;
	unsigned int x2, y2, z2, l;
	x = vector->x;
	y = vector->y;
	z = vector->z;
	n = 0;
	while (abs(x) > 0x1000000 || abs(y) > 0x1000000 || abs(z) > 0x1000000) {
		x >>= 1;
		y >>= 1;
		z >>= 1;
		n++;
	}
	x2 = math_mul(x, x);
	while (1) {
		y2 = math_mul(y, y);
		z2 = math_mul(z, z);
		if ((z2 >> 2) + (x2 >> 2) + (y2 >> 2) <= 0x40000000)
			break;
		x >>= 1;
		y >>= 1;
		z >>= 1;
		x2 = math_mul(x, x);
		n++;
	}
	l = math_sqrt(x2 + y2 + z2);
	for (; n != 0; n--)
		l *= 2;
	return l;
}

int math_vec3_length_XYZ(int px, int py, int pz) {
	tnfs_vec3 v;
	v.x = px;
	v.y = py;
	v.z = pz;
	return math_vec3_length(&v);
}

/*
 * vector3 length of X & Z values
 */
int math_vec3_length_XZ(tnfs_vec3 *vector) {
	// SE 0x41d4b8: halved until each part is <= 256.0 and the squares fit, then scaled back
	int x, z, n;
	unsigned int x2, z2, l;
	x = vector->x;
	z = vector->z;
	n = 0;
	while (abs(x) > 0x1000000 || abs(z) > 0x1000000) {
		x >>= 1;
		z >>= 1;
		n++;
	}
	x2 = math_mul(x, x);
	while (1) {
		z2 = math_mul(z, z);
		if ((z2 >> 2) + (x2 >> 2) <= 0x40000000)
			break;
		x >>= 1;
		z >>= 1;
		x2 = math_mul(x, x);
		n++;
	}
	l = math_sqrt(z2 + x2);
	for (; n != 0; n--)
		l *= 2;
	return l;
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
	// SE 0x41d07c (DOS SE 0x2bf9c): halved until each part is <= 256.0 and the squares fit, each part divided by the
	// length (math_vec3_normalize multiplies by its inverse instead)
	int x, y, z, len;
	unsigned int x2, y2, z2;
	x = input->x;
	y = input->y;
	z = input->z;
	while (abs(x) > 0x1000000 || abs(y) > 0x1000000 || abs(z) > 0x1000000) {
		x >>= 1;
		y >>= 1;
		z >>= 1;
	}
	x2 = math_mul(x, x);
	while (1) {
		y2 = math_mul(y, y);
		z2 = math_mul(z, z);
		if ((z2 >> 2) + (y2 >> 2) + (x2 >> 2) <= 0x40000000)
			break;
		x >>= 1;
		y >>= 1;
		x2 = math_mul(x, x);
		z >>= 1;
	}
	len = math_sqrt(x2 + y2 + z2);
	if (len != 0) {
		input->x = math_div(x, len);
		input->y = math_div(y, len);
		input->z = math_div(z, len);
	}
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


