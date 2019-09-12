#ifndef PLOT_H__

#define PLOT_H__
#define PLOT_VERSION "0.2.0"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "float.h"

struct plot_data {
	double *data;
	size_t len;
	struct plot_data *next;
};

enum Orientation {
	Vertical,
	Horizontal
};

struct plot {
	struct plot_data *data;
	unsigned int height;
	unsigned int width;
	size_t datasets;
	enum Orientation orientation;
};

struct plot_bounds {
	double max;
	double min;
};

struct plot *plot_init();
void plot_add(struct plot *plot, size_t len, double *data);
void plot_plot(struct plot *plot);
void plot_destroy(struct plot *plot, int free_data);
#endif
