#include "plot.h"
#define Y_LABEL_PAD "             "
#define Y_LABEL_FMT "%11.2f %s"

static char plot_peice[][4] = { "┤", "┼", "─", "│", "╰", "╭", "╮", "╯", " " };

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

struct plot *plot_init()
{
	struct plot *plot;
	struct x_label *xl;

	xl = safe_malloc(sizeof(struct x_label));
	xl->mod = 0;
	xl->every = 0;
	xl->start = 0;
	xl->color = 0;

	plot = safe_malloc(sizeof(struct plot));

	plot->data = NULL;
	plot->height = 24;
	plot->width = 80;
	plot->datasets = 0;
	plot->x_label = xl;
	plot->color = 0;
	plot->follow = 0;

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
	size_t i, j;
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

static void plot_write_norm(struct plot *plot, long *norm, char **canvas, int cs)
{
	enum plot_peice peice;
	size_t x, y;
	char clr[6];
	char *p;

	if (norm[1] > 0)
		snprintf(clr, 6, "%c[%ldm", 27, norm[1]);

	for (x = 2; x < norm[0] && x < plot->width; x++) {
		for (y = 0; y < plot->height; y++) {
			peice = plot_plot_peice(y, norm[x], norm[x + 1]);
			if (peice == PPBlank)
				continue;

			p = canvas[x - 2] + (y * cs);

			if (norm[1] > 0) {
				memcpy(p, clr, 5);
				memcpy(p + 5, plot_peice[peice], 4);
			} else {
				memcpy(p, plot_peice[peice], 4);
			}
		}
	}
}

static char **plot_fill_canvas(struct plot *plot, long **norm)
{
	size_t x, y, i;
	char **canvas = safe_calloc(plot->width, sizeof(char *));
	int cs = plot->color ? 9 : 4;

	for (x = 0; x < plot->width; x++) {
		canvas[x] = safe_calloc(plot->height, cs * sizeof(char));

		for (y = 0; y < plot->height; y++)
			memcpy(canvas[x] + (y * cs), plot_peice[PPBlank], 4);
	}

	for (i = 0; i < plot->datasets; i++)
		plot_write_norm(plot, norm[i], canvas, cs);

	return canvas;
}

static void plot_print_canvas(struct plot *plot, double *labels, char **canvas)
{
	long x, y;
	int cs = plot->color ? 9 : 4;

	for (y = plot->height - 1; y >= 0; y--) {
		if (plot->color)
			printf("%c[0m", 27);

		printf(Y_LABEL_FMT, labels[y], plot_peice[PPBarrier]);

		for (x = 0; x < plot->width; x++)
			fputs(canvas[x] + (y * cs), stdout);

		fputs("\n", stdout);
	}

	if (plot->color)
		printf("%c[0m", 27);
}

static void plot_print_x_label(unsigned int w, struct x_label *xl)
{
	long cur;
	long end = xl->start + w;
	char buf[20];
	int sign;
	long disp;

	snprintf(buf, 20, "%%-%dld", xl->every);
	printf(Y_LABEL_PAD);

	for (cur = xl->start; cur < end; cur++)
		if (cur % xl->every != 0)
			printf(" ");
		else
			break;

	for (; cur < end; cur++)
		if (cur % xl->every == 0) {
			sign = cur < 0 ? -1 : 1;
			disp = sign * (xl->mod != 0 ? cur % xl->mod : cur);

			if (xl->color > 0 && disp == 0) {
				printf("%c[%dm", 27, xl->color);
				printf(buf, disp);
				printf("%c[0m", 27);
			} else {
				printf(buf, disp);
			}
		}
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

	/* create the graph */
	char **canvas = plot_fill_canvas(plot, normalized);

	/* print the graph with labels */
	plot_print_canvas(plot, y_labels, canvas);

	if (plot->x_label->every > 0)
		plot_print_x_label(plot->width, plot->x_label);

	/* free everything */
	free(bounds);
	free(y_labels);
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

	free(plot->x_label);
	free(plot);
}
