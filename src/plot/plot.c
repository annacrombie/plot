#include <float.h>
#include <math.h>

#include "../util.h"
#include "plot.h"
#include "display.h"

struct plot_data {
	double *data;
	size_t len;
	struct plot_data *next;
	unsigned int color;
};

struct plot_bounds {
	double max;
	double min;
};

struct plot *plot_init(void)
{
	struct plot *plot;
	struct x_label *xl;
	struct y_label *yl;

	xl = safe_malloc(sizeof(struct x_label));
	xl->mod = 0;
	xl->every = 0;
	xl->start = 0;
	xl->color = 0;
	xl->side = 1;

	yl = safe_malloc(sizeof(struct y_label));
	yl->width = 11;
	yl->prec = 2;
	yl->side = 1;
	yl->r_fmt = NULL;
	yl->l_fmt = NULL;

	plot = safe_malloc(sizeof(struct plot));

	plot->data = NULL;
	plot->height = 24;
	plot->width = 80;
	plot->datasets = 0;
	plot->color = 0;
	plot->follow = 0;
	plot->merge_plot_peices = 0;
	plot->x_label = xl;
	plot->y_label = yl;

	return plot;
}

static struct plot_data *plot_data_init(size_t len, double *data, int color)
{
	struct plot_data *pd;

	pd = safe_malloc(sizeof(struct plot_data));

	pd->len = len;
	pd->data = data;
	pd->next = NULL;
	pd->color = color;

	return pd;
}

void plot_add(struct plot *plot, size_t len, double *data, int color)
{
	struct plot_data *d = plot->data;

	if (color != 0)
		plot->color = 1;

	if (d == NULL) {
		plot->data = plot_data_init(len, data, color);
	} else {
		while (d->next != NULL)
			d = d->next;

		d->next = plot_data_init(len, data, color);
	}

	plot->datasets++;
}

static struct plot_bounds *plot_data_get_bounds(struct plot_data *data)
{
	size_t i;
	struct plot_bounds *bounds;

	bounds = safe_malloc(sizeof(struct plot_bounds));

	bounds->max = DBL_MIN;
	bounds->min = DBL_MAX;

	while (data != NULL) {
		for (i = 0; i < data->len; i++) {
			if (data->data[i] > bounds->max)
				bounds->max = data->data[i];
			if (data->data[i] < bounds->min)
				bounds->min = data->data[i];
		}

		data = data->next;
	}

	return bounds;
}

static double *plot_make_labels(unsigned int height, struct plot_bounds *pb)
{
	unsigned int i;
	double *labels = safe_calloc(height, sizeof(double));
	double inc = (pb->max - pb->min) / (double)(height - 1);
	double s =  pb->min;

	for (i = 0; i < height; i++) {
		labels[i] = s;
		s += inc;
	}

	return labels;
}

static long **plot_normalize_data(struct plot *p, struct plot_bounds *b)
{
	long int i, j;
	long **normalized;

	double ratio = (double)(p->height - 1) / (b->max - b->min);

	struct plot_data *d = p->data;

	normalized = safe_calloc(p->datasets, sizeof(long *));

	j = 0;
	while (d != NULL) {
		normalized[j] = safe_calloc(d->len + 2, sizeof(long));

		normalized[j][0] = d->len + 2;
		normalized[j][1] = d->color;
		for (i = 2; i < normalized[j][0]; i++)
			normalized[j][i] = lround((d->data[i - 2] - b->min) * ratio);

		d = d->next;
		j++;
	}

	return normalized;

}

void plot_plot(struct plot *plot)
{
	size_t i;

	if (plot->datasets < 1)
		return;

	/* Determine the max and min of the array*/
	struct plot_bounds *bounds = plot_data_get_bounds(plot->data);

	/* Create the labels for the graph */
	double *y_labels = plot_make_labels(plot->height, bounds);

	/* normalize the values from 0 to height and place the results in a new
	 * array */
	long **normalized = plot_normalize_data(plot, bounds);

	plot_display(plot, y_labels, normalized);

	/* free everything */
	free(bounds);
	free(y_labels);
	for (i = 0; i < plot->datasets; i++)
		free(normalized[i]);
	free(normalized);
}

void plot_destroy(struct plot *plot, int free_data)
{
	struct plot_data *d, *e;

	d = plot->data;

	while (d != NULL) {
		e = d->next;
		if (free_data)
			free(d->data);
		free(d);
		d = e;
	}

	free(plot->x_label);

	if (plot->y_label->l_fmt != NULL)
		free(plot->y_label->l_fmt);
	if (plot->y_label->r_fmt != NULL)
		free(plot->y_label->r_fmt);
	free(plot->y_label);

	free(plot);
}
