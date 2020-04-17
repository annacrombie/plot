#ifndef _PLOT_H_
#define _PLOT_H_

#define PLOT_VERSION "0.2.1"

#include <stdio.h>
#include <stdlib.h>

#define INBUF 512
#define CHARBUF 31
#define MAX_DATA 32
#define MAX_WIDTH 256
#define MAX_HEIGHT 256

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

struct canvas_elem {
	enum plot_peice peice;
	unsigned int color;
};


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
	unsigned int color;
	int side;
	char label[CHARBUF + 1];
};

struct input {
	char buf[INBUF];
	FILE *src;
	size_t rem;
	size_t size;
};

struct plot_data {
	double data[MAX_WIDTH];
	size_t len;
	unsigned int color;
	struct input src;
};

struct plot {
	struct x_label x_label;
	struct y_label y_label;
	struct canvas_elem canvas[MAX_WIDTH][MAX_HEIGHT];
	long normalized[MAX_DATA][MAX_WIDTH];
	double labels[MAX_HEIGHT];
	struct plot_data data[MAX_DATA];
	enum plot_charset charset;
	size_t datasets;
	unsigned int height;
	unsigned int width;
	long follow_rate;
	int follow;
	int color;
	int merge_plot_peices;
};

void plot_init(struct plot *plot);
void plot_add(struct plot *plot, FILE *f, int color);
int plot_plot(struct plot *plot);
void plot_destroy(struct plot *plot);
#endif
