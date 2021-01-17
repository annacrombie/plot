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

/* low 4 bits are plot peice
 * high 4 bits are color
 */
typedef uint8_t canvas_elem;

struct y_label {
	unsigned int width;
	unsigned int prec;
	int side;
	char r_fmt[CHARBUF + 1];
	char l_fmt[CHARBUF + 1];
};

struct x_label {
	unsigned int mod;
	unsigned int every;
	long start;
	enum color color;
	int side;
	char label[CHARBUF + 1];
};

struct input {
	char buf[INBUF];
	FILE *src;
	size_t rem;
	size_t size;
};

struct plot_bounds {
	double max;
	double min;
};

struct plot_data {
	double data[MAX_WIDTH];
	size_t len;
	enum color color;
	struct input src;
	struct {
		double sum;
		int count;
	} avg;
};

struct plot {
	canvas_elem canvas[MAX_WIDTH][MAX_HEIGHT];
	struct x_label x_label;
	struct y_label y_label;
	double labels[MAX_HEIGHT];
	struct plot_data data[MAX_DATA];
	enum plot_charset charset;
	size_t datasets;
	unsigned int height;
	unsigned int width;
	long follow_rate;
	long average;
	int animate;
	int follow;
	int color;
	int merge_plot_peices;
	int fixed_bounds;
	struct plot_bounds bounds;
};

void plot_init(struct plot *plot);
void plot_add(struct plot *plot, FILE *f, int color);
int plot_plot(struct plot *plot);
void plot_destroy(struct plot *plot);
#endif
