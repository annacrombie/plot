#include "plot.h"

static char *plot_chars = "┤\0┼\0─\0│\0╰\0╭\0╮\0╯\0 \0\0\0";
//                         0  4  8  12 16 20 24 28 32
enum plot_peice {
	PPBarrier,
	PPCross,
	PPRight,
	PPVert,
	PPDownRight,
	PPUpRight,
	PPRightDown,
	PPRightUp,
	PPBlank
};

static char *plot_peice_c(enum plot_peice p)
{
	return &plot_chars[p * 4];
}

struct plot *plot_init()
{
	struct plot *plot;

	plot = safe_malloc(sizeof(struct plot));

	plot->data = NULL;
	plot->height = 24;
	plot->width = 80;
	plot->orientation = Horizontal;
	plot->datasets = 0;

	return plot;
}

static struct plot_data *plot_data_init(size_t len, double *data)
{
	struct plot_data *pd;

	pd = safe_malloc(sizeof(struct plot_data));

	pd->len = len;
	pd->data = data;
	pd->next = NULL;

	return pd;
}

void plot_add(struct plot *plot, size_t len, double *data)
{
	struct plot_data *d = plot->data;

	if (d == NULL) {
		plot->data = plot_data_init(len, data);
	} else {
		while (d->next != NULL)
			d = d->next;

		d->next = plot_data_init(len, data);
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
	size_t i, j;
	long **normalized;

	double ratio = (double)(p->height - 1) / (b->max - b->min);

	struct plot_data *d = p->data;

	normalized = safe_calloc(p->datasets, sizeof(long *));

	j = 0;
	while (d != NULL) {
		normalized[j] = safe_calloc(d->len + 1, sizeof(long));

		normalized[j][0] = d->len;
		for (i = 1; i <= d->len; i++)
			normalized[j][i] = lround((d->data[i - 1] - b->min) * ratio);

		d = d->next;
		j++;
	}

	return normalized;

}

static enum plot_peice plot_plot_peice(long y, long cur, long next)
{
	enum plot_peice i;

	if (y == cur)
		i = next > cur ? PPRightUp : next < cur ? PPRightDown : PPRight;
	else if (y == next)
		i = next > cur ? PPUpRight : next < cur ? PPDownRight : PPRight;
	else if ((y > cur && y < next) || (y < cur && y > next))
		i = PPVert;
	else
		i = PPBlank;

	return i;
}

static void plot_write_norm(struct plot *plot, long *norm, char **canvas)
{
	enum plot_peice peice;
	size_t x, y;

	for (x = 1; x < norm[0] && x < plot->width; x++) {
		for (y = 0; y < plot->height; y++) {
			peice = plot_plot_peice(y, norm[x], norm[x + 1]);
			if (peice != PPBlank)
				memcpy(canvas[x] + (y * 4), plot_peice_c(peice), 4);
		}
	}
}

static char **plot_fill_canvas(struct plot *plot, long **norm)
{
	size_t x, y, i;
	char **canvas = safe_calloc(plot->width, sizeof(char *));

	for (x = 0; x < plot->width; x++) {
		canvas[x] = safe_calloc(plot->height, 4 * sizeof(char));
		for (y = 0; y < plot->height; y++)
			memcpy(canvas[x] + (y * 4), plot_peice_c(PPBlank), 4);
	}

	for (i = 0; i < plot->datasets; i++)
		plot_write_norm(plot, norm[i], canvas);

	return canvas;
}

static void plot_print_canvas(struct plot *plot, double *labels, char **canvas)
{
	long x, y;

	for (y = plot->height - 1; y >= 0; y--) {
		printf("%11.2f %s", labels[y], plot_peice_c(PPBarrier));

		for (x = 0; x < plot->width; x++)
			fputs(canvas[x] + (y * 4), stdout);

		fputs("\n", stdout);
	}

}

void plot_plot(struct plot *plot)
{
	size_t i;

	/* Determine the max and min of the array*/
	struct plot_bounds *bounds = plot_data_get_bounds(plot->data);

	/* Create the labels for the graph */
	double *labels = plot_make_labels(plot->height, bounds);

	/* normalize the values from 0 to height and place the results in a new
	 * array */
	long **normalized = plot_normalize_data(plot, bounds);

	/* create the graph */
	char **canvas = plot_fill_canvas(plot, normalized);

	/* print the graph with labels */
	plot_print_canvas(plot, labels, canvas);

	/* free everything */
	free(bounds);
	free(labels);
	for (i = 0; i < plot->datasets; i++)
		free(normalized[i]);
	free(normalized);
	for (i = 0; i < plot->width; i++)
		free(canvas[i]);
	free(canvas);
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

	free(plot);
}
