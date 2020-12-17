#ifndef _PLOT_H_
#define _PLOT_H_

#define PLOT_VERSION "0.3.0"

#include <stdio.h>
#include <stdlib.h>

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
	unsigned int color;
	long start;
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
	unsigned int color;
	size_t len;
	double *data;
	struct input *src;
	struct {
		double sum;
		int count;
	} avg;
};

struct plot {
	int animate;
	int follow;
	int color;
	int merge_plot_peices;
	int fixed_bounds;
	enum plot_charset charset;
	unsigned int height;
	unsigned int width;
	long follow_rate;
	long average;
	double *labels;
	long **normalized;
	size_t datasets;
	struct x_label *x_label;
	struct y_label *y_label;
	struct canvas_elem ***canvas;
	struct plot_bounds *bounds;
	struct plot_data **data;
};

struct y_label * init_y_label(void);
void free_y_label(struct y_label *);
struct x_label * init_x_label(void);
void free_x_label(struct x_label *);
struct input * init_input(void);
void free_input(struct input *);
struct plot_bounds * init_plot_bounds(void);
void free_plot_bounds(struct plot_bounds *);
struct plot_data * init_plot_data(void);
void free_plot_data(struct plot_data *);
struct plot * init_plot_z(void);
void free_plot(struct plot *);

struct plot * plot_init(void);
void plot_add(struct plot *plot, FILE *f, int color);
int plot_plot(struct plot *plot);
void plot_destroy(struct plot *plot);
#endif
