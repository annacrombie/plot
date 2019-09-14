#ifndef PLOT_H__

#define PLOT_H__
#define PLOT_VERSION "0.2.0"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "util.h"

struct plot {
	struct plot_data *data;
	unsigned int height;
	unsigned int width;
	size_t datasets;
	struct x_label *x_label;
	int color;
	int follow;
	int combine;
};

struct x_label {
	unsigned int mod;
	unsigned int every;
	long start;
	unsigned int color;
};

struct plot *plot_init();
void plot_add(struct plot *plot, size_t len, double *data, int color);
void plot_plot(struct plot *plot);
void plot_destroy(struct plot *plot, int free_data);
#endif
