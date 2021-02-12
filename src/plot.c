#include "posix.h"

#include <float.h>
#include <string.h>

#include "display.h"
#include "input.h"
#include "plot.h"
#include "util.h"

#define PLOT_DEFAULT_BOUND 8

void
plot_init(struct plot *plot)
{
	plot->x_label.side = 1;

	plot->y_label.width = 11;
	plot->y_label.prec = 2;
	plot->y_label.side = 1;

	plot->charset = PCUNICODE;
	plot->height = 24;
	plot->width = 80;

	plot->flags = 0;
	plot->follow_rate = 100;

	plot->datasets = 0;
}

void
plot_add(struct plot *plot, int color)
{
	if (color != 0) {
		plot->flags |= plot_flag_color;
	}

	plot->data[plot->datasets].len = 0;
	plot->data[plot->datasets].color = color;
	plot->datasets++;
}

static struct plot_bounds
plot_data_get_bounds(size_t len, struct plot_data *pda)
{
	size_t i, j;
	struct plot_bounds bounds;

	bounds.max = -1 * DBL_MAX;
	bounds.min = DBL_MAX;

	for (j = 0; j < len; j++) {
		for (i = 0; i < pda[j].len; i++) {
			if (pda[j].data[i] > bounds.max) {
				bounds.max = pda[j].data[i];
			}
			if (pda[j].data[i] < bounds.min) {
				bounds.min = pda[j].data[i];
			}
		}
	}

	if (bounds.max == bounds.min) {
		bounds.max += PLOT_DEFAULT_BOUND;
	}

	return bounds;
}

static void
plot_make_labels(struct plot *p)
{
	unsigned int i;
	double inc = (p->bounds.max - p->bounds.min) / (double)(p->height - 1);
	double s =  p->bounds.min;

	for (i = 0; i < p->height; i++) {
		p->labels[i] = s;
		s += inc;
	}
}

int
plot_plot(struct plot *plot)
{
	size_t i;
	int no_data = 1;

	for (i = 0; i < plot->datasets; i++) {
		if (plot->data[i].len > 0) {
			no_data = 0;
			break;
		}
	}

	if (plot->datasets < 1 || no_data) {
		//fprintf(stderr, "no data\n");
		return 0;
	}


	/* Determine the max and min of the array*/
	if (!(plot->flags & plot_flag_fixed_bounds)) {
		plot->bounds = plot_data_get_bounds(plot->datasets, plot->data);
	}

	plot_make_labels(plot);

	plot_display(plot);

	return 1;
}
