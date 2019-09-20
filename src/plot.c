#include <float.h>
#include <math.h>
#include <string.h>
#include "util.h"
#include "input.h"
#include "plot.h"
#include "display.h"

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

static struct plot_data *plot_data_init(FILE *src, int color)
{
	struct plot_data *pd;

	pd = safe_malloc(sizeof(struct plot_data));

	pd->src = src;
	pd->len = 0;
	pd->data = NULL;
	pd->color = color;

	return pd;
}

void plot_add(struct plot *plot, FILE *f, int color)
{
	size_t len;

	if (color != 0)
		plot->color = 1;

	if (plot->data == NULL) {
		plot->data = safe_calloc(1, sizeof(struct plot_data *));
		plot->data[0] = plot_data_init(f, color);
		plot->datasets = 1;
		return;
	}

	len = plot->datasets + 1;

	plot->data = safe_realloc(plot->data, len * sizeof(struct plot_data *));
	plot->data[len - 1] = plot_data_init(f, color);
	plot->datasets++;
}

void plot_prepare(struct plot *p)
{
	size_t i;

	for (i = 0; i < p->datasets; i++)
		p->data[i]->data = safe_calloc(p->width, sizeof(double));
}

static struct plot_bounds *
plot_data_get_bounds(size_t len, struct plot_data **pda)
{
	size_t i, j;
	struct plot_bounds *bounds;
	struct plot_data *data;

	bounds = safe_malloc(sizeof(struct plot_bounds));

	bounds->max = DBL_MIN;
	bounds->min = DBL_MAX;

	for (j = 0; j < len; j++) {
		data = pda[j];
		for (i = 0; i < data->len; i++) {
			if (data->data[i] > bounds->max)
				bounds->max = data->data[i];
			if (data->data[i] < bounds->min)
				bounds->min = data->data[i];
		}
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

	struct plot_data *d;

	normalized = safe_calloc(p->datasets, sizeof(long *));

	for (j = 0; j < p->datasets; j++) {
		d = p->data[j];

		normalized[j] = safe_calloc(d->len + 2, sizeof(long));

		normalized[j][0] = d->len + 2;
		normalized[j][1] = d->color;
		for (i = 2; i < normalized[j][0]; i++)
			normalized[j][i] = lround((d->data[i - 2] - b->min) * ratio);
	}

	return normalized;
}

void plot_plot(struct plot *plot)
{
	size_t i;

	if (plot->datasets < 1)
		return;

	/* Determine the max and min of the array*/
	struct plot_bounds *bounds = plot_data_get_bounds(plot->datasets, plot->data);

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
	size_t i;

	for (i = 0; i < plot->datasets; i++) {
		if (free_data)
			free(plot->data[i]->data);

		if (plot->data[i]->src != NULL)
			fclose(plot->data[i]->src);

		free(plot->data[i]);
	}

	if (plot->data != NULL)
		free(plot->data);

	free(plot->x_label);

	if (plot->y_label->l_fmt != NULL)
		free(plot->y_label->l_fmt);
	if (plot->y_label->r_fmt != NULL)
		free(plot->y_label->r_fmt);
	free(plot->y_label);

	free(plot);
}
