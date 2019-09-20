#ifndef _PLOT_H_
#define _PLOT_H_
#define PLOT_VERSION "0.2.0"
#include <stdio.h>
#include <stdlib.h>

struct plot {
	struct plot_data **data;
	size_t datasets;
	unsigned int height;
	unsigned int width;
	int color;
	int follow;
	int merge_plot_peices;
	struct x_label *x_label;
	struct y_label *y_label;
};

struct y_label {
	unsigned int width;
	unsigned int prec;
	int side;
	char *r_fmt;
	char *l_fmt;
};

struct x_label {
	unsigned int mod;
	unsigned int every;
	long start;
	unsigned int color;
	int side;
	char *label;
};

struct plot_data {
	double *data;
	size_t len;
	unsigned int color;
	struct input *src;
};

struct plot *plot_init(void);
void plot_add(struct plot *plot, FILE *f, int color);
void plot_plot(struct plot *plot);
void plot_destroy(struct plot *plot, int free_data);
void plot_prepare(struct plot *p);
#endif
