#include <float.h>
#include <math.h>
#include <string.h>

#include "display.h"
#include "input.h"
#include "plot.h"
#include "util.h"
#include "alloc.h"
#include "debug.h"

#define PLOT_DEFAULT_BOUND 8

struct y_label * init_y_label(void)
{
	struct y_label *y = NULL;
	y = ccalloc(1, sizeof(struct y_label));
	return y;
}

void free_y_label(struct y_label *y)
{
	free(y);
}

struct x_label * init_x_label(void)
{
	struct x_label *x = NULL;
	x = mmalloc(sizeof(struct x_label));
	return x;
}

void free_x_label(struct x_label *x)
{
	free(x);
}

struct input * init_input(void)
{
	struct input *i = mmalloc(sizeof(struct input));
	return i;
}

void free_input(struct input *i)
{
	free(i);
}

struct plot_bounds * init_plot_bounds(void)
{
	return mmalloc(sizeof(struct plot_bounds));
}

void free_plot_bounds(struct plot_bounds *p)
{
	free(p);
}

struct plot_data * init_plot_data(void)
{
	struct plot_data * p = NULL;
	p = ccalloc(1, sizeof(struct plot_data));
	p->data = ccalloc(MAX_WIDTH, sizeof(double));
	p->src = init_input();
	return p;
}

void free_plot_data(struct plot_data *p)
{
	free(p->data);
	free_input(p->src);
	free(p);
}

struct plot * init_plot_z(void)
{
	int i, j;
	struct plot *p;

	p = ccalloc(1, sizeof (struct plot));

	p->x_label = init_x_label();
	p->y_label = init_y_label();

	//TODO make this an init/free pair
	p->canvas = ccalloc(MAX_HEIGHT, sizeof (struct canvas_elem **));
	for (i = 0; i < MAX_HEIGHT; i++) {
		p->canvas[i] = ccalloc(MAX_WIDTH, sizeof (struct canvas_elem *));
		for (j = 0; j < MAX_WIDTH; j++) {
			p->canvas[i][j] = mmalloc(sizeof (struct canvas_elem));
		}
	}

	p->bounds = init_plot_bounds();

	//TODO make this an init/free pair
	p->data = ccalloc(MAX_DATA, sizeof (struct plot_data *));
	for (i = 0; i < MAX_DATA; i++) {
		p->data[i] = init_plot_data();
	}

	p->normalized = ccalloc(MAX_WIDTH, sizeof(long *));
	for (i = 0; i < MAX_WIDTH; i++) {
		p->normalized[i] = ccalloc(MAX_DATA, sizeof(long));
	}

	p->labels = ccalloc(MAX_HEIGHT, sizeof(double));

	return p;
}

void free_plot(struct plot *p)
{
	int i,j;
	free_y_label(p->y_label);
	free_x_label(p->x_label);

	for (i = 0; i < MAX_HEIGHT; i++) {
		for (j = 0; j < MAX_WIDTH; j++) {
			free(p->canvas[i][j]);
		}
		free(p->canvas[i]);
	}
	free(p->canvas);

	free_plot_bounds(p->bounds);

	for (i = 0; i < MAX_DATA; i++) {
		free_plot_data(p->data[i]);
	}
	free(p->data);

	for (i = 0; i < MAX_WIDTH; i++) {
		free(p->normalized[i]);
	}
	free(p->normalized);

	free(p->labels);
	free(p);
}

struct plot * plot_init(void)
{
	struct plot * plot;
	plot = init_plot_z();

	plot->x_label->side = 1;
	plot->y_label->width = 11;
	plot->y_label->prec = 2;
	plot->y_label->side = 1;

	plot->charset = PCUNICODE;
	plot->height = 24;
	plot->width = 80;
	plot->follow = 0;

	plot->animate = 0;
	plot->follow = 0;
	plot->follow_rate = 100;

	plot->average = 1;

	plot->datasets = 0;

	plot->fixed_bounds = 0;

	return plot;
}

static void
plot_data_init(struct plot_data *pd, FILE *src, int color)
{
	pd->src->src = src;
	pd->src->rem = 0;
	pd->src->size = 0;
	pd->len = 0;
	pd->color = color;
}

void
plot_add(struct plot *plot, FILE *f, int color)
{
	if (color != 0) {
		plot->color = 1;
	}

	plot_data_init(plot->data[plot->datasets], f, color);
	plot->datasets++;
}

static struct plot_bounds *
plot_data_get_bounds(struct plot_bounds *bounds, size_t len, struct plot_data **pda)
{
	size_t i, j;

	bounds->max = -1 * DBL_MAX;
	bounds->min = DBL_MAX;

	for (j = 0; j < len; j++) {
		for (i = 0; i < pda[j]->len; i++) {
			if (pda[j]->data[i] > bounds->max) {
				bounds->max = pda[j]->data[i];
			}
			if (pda[j]->data[i] < bounds->min) {
				bounds->min = pda[j]->data[i];
			}
		}
	}

	if (bounds->max == bounds->min) {
		bounds->max += PLOT_DEFAULT_BOUND;
	}

	return bounds;
}

static void
plot_make_labels(struct plot *p, struct plot_bounds *pb)
{
	unsigned int i;
	double inc = (pb->max - pb->min) / (double)(p->height - 1);
	double s =  pb->min;

	for (i = 0; i < p->height; i++) {
		p->labels[i] = s;
		s += inc;
	}
}

static void
plot_normalize_data(struct plot *p, struct plot_bounds *b)
{
	size_t i, j;

	double ratio = (double)(p->height - 1) / (b->max - b->min);

	for (j = 0; j < p->datasets; j++) {

		p->normalized[j][0] = p->data[j]->len + 2;
		p->normalized[j][1] = p->data[j]->color;
		for (i = 2; i < (size_t)p->normalized[j][0] && i < MAX_DATA; i++) {
			p->normalized[j][i] = lround(
				(p->data[j]->data[i - 2] - b->min) * ratio);
		}
	}
}

int
plot_plot(struct plot *plot)
{
	size_t i;
	int no_data = 1;

	for (i = 0; i < plot->datasets; i++) {
		if (plot->data[i]->len > 0) {
			no_data = 0;
			break;
		}
	}

	if (plot->datasets < 1 || no_data) {
		//fprintf(stderr, "no data\n");
		return 0;
	}

	/* Determine the max and min of the array*/
	if (!plot->fixed_bounds) {
		plot_data_get_bounds(plot->bounds, plot->datasets, plot->data);
	}

	plot_make_labels(plot, plot->bounds);

	plot_normalize_data(plot, plot->bounds);

	plot_display(plot);

	return 1;
}

void
plot_destroy(struct plot *plot)
{
	size_t i;

	for (i = 0; i < plot->datasets; i++) {
		if (plot->data[i]->src->src != NULL) {
			fclose(plot->data[i]->src->src);
		}
	}

	free_plot(plot);
}
