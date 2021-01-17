#ifndef _PLOT_H_
#define _PLOT_H_

#define PLOT_VERSION "0.3.0"

#include <stdint.h>
#include <stdio.h>

#define INBUF 512
#define CHARBUF 31
#define MAX_DATA 32
#define MAX_WIDTH 512
#define MAX_HEIGHT 512

enum plot_charset {
	PCUNICODE = 0,
	PCASCII = 1,
	PCCUSTOM = 2
};

/* A peice can be defined by the four sides it touches of the cell it contains.
 * The convention used is that the South side is the starting point, and you go
 * counter-clockwise.  So the peice pointing South and North (│) is represented
 * by 0101 (binary) or 5.
 *
 * Here is a chart of all the peices
 * peice | binary | hex
 * ' '   | 0000   | 0
 * ╭     | 0011   | 3
 * │     | 0101   | 5
 * ╰     | 0110   | 6
 * ├     | 0111   | 7
 * ╮     | 1001   | 9
 * ─     | 1010   | a
 * ┬     | 1011   | b
 * ╯     | 1100   | c
 * ┤     | 1101   | d
 * ┴     | 1110   | e
 * ┼     | 1111   | f
 */

enum plot_peice {
	PPBlank     = 0x0,
	PPUpRight   = 0x3,
	PPVert      = 0x5,
	PPDownRight = 0x6,
	PPTRight    = 0x7,
	PPRightDown = 0x9,
	PPHoriz     = 0xa,
	PPTDown     = 0xb,
	PPRightUp   = 0xc,
	PPTLeft     = 0xd,
	PPTUp       = 0xe,
	PPCross     = 0xf,
};

enum color {
	clr_b,
	clr_r,
	clr_g,
	clr_y,
	clr_l,
	clr_m,
	clr_c,
	clr_w,
	clr_B,
	clr_R,
	clr_G,
	clr_Y,
	clr_L,
	clr_M,
	clr_C,
	clr_W,
};

enum side {
	side_neither = 0,
	side_bottom = 1 << 0,
	side_top    = 1 << 1,
	side_left   = 1 << 0,
	side_right  = 1 << 1,
	side_both   = 3,
};

/* low 4 bits are plot peice
 * high 4 bits are color
 */
typedef uint8_t canvas_elem;

struct y_label {
	char r_fmt[CHARBUF + 1];
	char l_fmt[CHARBUF + 1];
	uint32_t width, prec;
	enum side side;
};

struct x_label {
	char label[CHARBUF + 1];
	int64_t start;
	uint32_t mod, every;
	enum color color;
	enum side side;
};

struct input {
	char buf[INBUF];
	FILE *src;
	size_t rem, size;
};

struct plot_bounds {
	double max, min;
};

struct plot_data {
	double data[MAX_WIDTH];
	struct input src;
	struct { double sum; uint32_t count; } avg;
	size_t len;
	enum color color;
};

enum plot_flags {
	plot_flag_animate           = 1 << 0,
	plot_flag_follow            = 1 << 1,
	plot_flag_color             = 1 << 2,
	plot_flag_merge_plot_pieces = 1 << 3,
	plot_flag_fixed_bounds      = 1 << 4,
};

struct plot {
	canvas_elem canvas[MAX_WIDTH][MAX_HEIGHT];
	struct plot_data data[MAX_DATA];
	double labels[MAX_HEIGHT];
	struct plot_bounds bounds;
	struct x_label x_label;
	struct y_label y_label;
	enum plot_charset charset;
	uint32_t height, width;
	uint32_t datasets;
	uint32_t follow_rate;
	uint32_t average;
	uint32_t flags;
};

void plot_init(struct plot *plot);
void plot_add(struct plot *plot, FILE *f, int color);
int plot_plot(struct plot *plot);
void plot_destroy(struct plot *plot);
#endif
