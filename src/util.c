#include "posix.h"

#include <assert.h>

#include "util.h"

int
is_digit(char c)
{
	switch (c) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	case '-': case '.':
		return 1;
	default:
		return 0;
	}
}

unsigned int
color_to_ansi_escape_color(enum color color)
{
	switch (color) {
	case clr_b: return 30;
	case clr_r: return 31;
	case clr_g: return 32;
	case clr_y: return 33;
	case clr_l: return 34;
	case clr_m: return 35;
	case clr_c: return 36;
	case clr_w: return 37;
	case clr_B: return 90;
	case clr_R: return 91;
	case clr_G: return 92;
	case clr_Y: return 93;
	case clr_L: return 94;
	case clr_M: return 95;
	case clr_C: return 96;
	case clr_W: return 97;
	default: assert(0); return 0;
	}

}

enum color
char_to_color(char c)
{
	enum color n;

	switch (c) {
	case 'b': n = clr_b; break;
	case 'r': n = clr_r; break;
	case 'g': n = clr_g; break;
	case 'y': n = clr_y; break;
	case 'l': n = clr_l; break;
	case 'm': n = clr_m; break;
	case 'c': n = clr_c; break;
	case 'w': n = clr_w; break;
	case 'B': n = clr_B; break;
	case 'R': n = clr_R; break;
	case 'G': n = clr_G; break;
	case 'Y': n = clr_Y; break;
	case 'L': n = clr_L; break;
	case 'M': n = clr_M; break;
	case 'C': n = clr_C; break;
	case 'W': n = clr_W; break;
	default:
		fprintf(stderr, "invalid color char: %c\n", c);
		n = 0;
		break;
	}

	return n;
}

unsigned int
utf8_bytes(const char *utf8)
{
	unsigned char c;

	c = utf8[0];

	if ((c & 0x7f) == c) {
		return 1;
	}else if ((c & 0xdf) == c) {
		return 2;
	}else if ((c & 0xef) == c) {
		return 3;
	}else if ((c & 0xf7) == c) {
		return 4;
	}else {
		return 0;
	}
}
