#include "plot.h"
#define Y_LABEL_PAD "             "
#define Y_LABEL_FMT "%11.2f %s"

//static char plot_peice[][4] = { "┤", "┼", "─", "│", "╰", "╭", "╮", "╯", " " };

static char plot_peices[][4] = {
	" ", "─", "│", "┼",
	"╮", "╯", "╰", "╭",
	"┬", "┤", "┴", "├",
};

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
	PPBlank,        //0
	PPHoriz,        //1
	PPVert,         //2
	PPCross,        //3

	PPRightDown,    //4
	PPRightUp,      //5
	PPDownRight,    //6
	PPUpRight,      //7

	PPTDown,        //8
	PPTLeft,        //9
	PPTUp,          //10
	PPTRight        //11
};

static enum plot_peice join_matrix[12][12] = {
/*       ' '  ─   │   ┼   ╮   ╯   ╰   ╭   ┬   ┤   ┴   ├ */
	{ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11 },     /* ' ' */
	{ 1,  1,  3,  3,  8,  10, 10, 8,  8,  3,  10, 3  },     /*  ─  */
	{ 2,  3,  2,  3,  9,  9,  11, 11, 3,  9,  3,  11 },     /*  │  */
	{ 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3  },     /*  ┼  */
	{ 4,  8,  9,  3,  4,  9,  3,  8,  8,  9,  3,  3  },     /*  ╮  */
	{ 5,  10, 9,  3,  9,  5,  10, 3,  3,  9,  10, 3  },     /*  ╯  */
	{ 6,  10, 11, 3,  3,  10, 6,  11, 3,  3,  10, 11 },     /*  ╰  */
	{ 7,  8,  11, 3,  8,  3,  11, 7,  8,  3,  3,  11 },     /*  ╭  */
	{ 8,  8,  3,  3,  8,  3,  3,  8,  8,  3,  3,  3  },     /*  ┬  */
	{ 9,  3,  9,  3,  9,  9,  3,  3,  3,  9,  3,  3  },     /*  ┤  */
	{ 10, 10, 3,  3,  3,  10, 10, 3,  3,  3,  10, 3  },     /*  ┴  */
	{ 11, 3,  11, 3,  3,  3,  11, 11, 3,  3,  3,  11 }      /*  ├  */

};

struct canvas_elem {
	enum plot_peice peice;
	unsigned int color;
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
	plot->merge_plot_peices = 0;

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

static enum plot_peice next_peice(long y, long cur, long next)
{
	enum plot_peice i;

	if (y == cur)
		i = next > cur ? PPRightUp : next < cur ? PPRightDown : PPHoriz;
	else if (y == next)
		i = next > cur ? PPUpRight : next < cur ? PPDownRight : PPHoriz;
	else if ((y > cur && y < next) || (y < cur && y > next))
		i = PPVert;
	else
		i = PPBlank;

	return i;
}

static enum plot_peice combine_plot_peices(enum plot_peice a, enum plot_peice b)
{
	return join_matrix[a][b];
}

static void plot_write_norm(struct plot *plot, long *norm, struct canvas_elem **c)
{
	size_t x, y;
	enum plot_peice next;

	for (x = 2; x < norm[0] && x < plot->width; x++) {
		for (y = 0; y < plot->height; y++) {
			next = next_peice(y, norm[x], norm[x + 1]);

			if (next == PPBlank)
				continue;

			if (plot->merge_plot_peices)
				next = combine_plot_peices(c[x][y].peice, next);

			c[x][y].color = norm[1];
			c[x][y].peice = next;
		}
	}
}

static struct canvas_elem **plot_fill_canvas(struct plot *plot, long **norm)
{
	size_t x, y, i;
	struct canvas_elem **canvas =
		safe_calloc(plot->width, sizeof(struct canvas_elem *));

	for (x = 0; x < plot->width; x++) {
		canvas[x] = safe_calloc(plot->height, sizeof(struct canvas_elem));

		for (y = 0; y < plot->height; y++) {
			canvas[x][y].color = 0;
			canvas[x][y].peice = PPBlank;
		}
	}

	for (i = 0; i < plot->datasets; i++)
		plot_write_norm(plot, norm[i], canvas);

	return canvas;
}

static void plot_print_canvas(struct plot *plot, double *labels, struct canvas_elem **canvas)
{
	long x, y;

	for (y = plot->height - 1; y >= 0; y--) {
		if (plot->color)
			printf("%c[0m", 27);

		printf(Y_LABEL_FMT, labels[y], plot_peices[PPTLeft]);

		for (x = 0; x < plot->width; x++) {
			if (canvas[x][y].color > 0)
				printf("\e[%dm", canvas[x][y].color);

			printf("%s", plot_peices[canvas[x][y].peice]);
		}

		printf("\n");
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
	struct canvas_elem **canvas = plot_fill_canvas(plot, normalized);

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
