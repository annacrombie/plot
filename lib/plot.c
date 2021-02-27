#include "posix.h"

#include <float.h>
#include <string.h>

#include "internal/display.h"
#include "internal/log.h"
#include "internal/util.h"
#include "plot/file_input.h"
#include "plot/plot.h"

#define PLOT_DEFAULT_BOUND 8


#define _X_ ""
/* " ╔║╚╠╗═╦╝╣╩╬" */
static const char plot_charsets[][16][4] = {
	/* " ╭│╰├╮─┬╯┤┴┼" */
	{
		" ", _X_, _X_, "╭", _X_, "│", "╰", "├",
		_X_, "╮", "─", "┬", "╯", "┤", "┴", "┼",
	},
	{
		" ", _X_, _X_, ".", _X_, "|", "`", "|",
		_X_, ",", "-", "?", "'", "|", "?", "+",
	},
	{
		"?", _X_, _X_, "?", _X_, "?", "?", "?",
		_X_, "?", "?", "?", "?", "?", "?", "?",
	}
};

void
plot_set_charset(struct plot *plot, enum plot_charset charset)
{
	memcpy(plot->charset, plot_charsets[charset], 16 * 4);
}

void
plot_init(struct plot *plot, uint8_t *canvas, double *data_buf,
	struct plot_data *pd, uint32_t height, uint32_t width,
	uint32_t depth)
{
	*plot = (struct plot) {
		.canvas = canvas,
		.data_buf = data_buf,
		.data = pd,
		.x_label.side = 1,
		.y_label.width = 11,
		.y_label.prec = 2,
		.y_label.side = 1,
		.height = height,
		.width = width,
		.depth = depth,
		.follow_rate = 100,
	};

	plot_set_charset(plot, plot_charset_unicode);
}

void
plot_set_custom_charset(struct plot *plot, char *str, size_t len)
{
	size_t i, j, k;
	unsigned int bytes;

	plot_set_charset(plot, plot_charset_custom);

	for (i = j = 0; i < 16; i++) {
		if (i == 1 || i == 2 || i == 4 || i == 8) {
			continue;
		}

		bytes = utf8_bytes(&str[j]);

		if (bytes + j > len) {
			return;
		}

		for (k = 0; k < bytes; k++) {
			plot->charset[i][k] = str[j + k];
		}

		j += k;
	}
}

void
plot_dataset_init(struct plot_data *pd, enum plot_color color,
	struct plot_pipeline_elem *ple, uint32_t ple_max,
	plot_input_func input_func, void *input_ctx)
{
	*pd = (struct plot_data){
		.in = { .read = input_func, .ctx = input_ctx, },
		.pipe = ple,
		.pipeline_max = ple_max,
		.color = color,
	};
}

bool
plot_add_dataset(struct plot *plot, enum plot_color color,
	struct plot_pipeline_elem *ple, uint32_t ple_max,
	plot_input_func input_func, void *input_ctx)
{
	if (plot->datasets > plot->depth) {
		return false;
	}

	if (color != 0) {
		plot->flags |= plot_flag_color;
	}

	plot_dataset_init(&plot->data[plot->datasets], color, ple, ple_max,
		input_func, input_ctx);

	++plot->datasets;

	return true;
}

static void
set_auto_bounds(struct plot *p)
{
	uint32_t i, j;
	double max = -1 * DBL_MAX, min = DBL_MAX, *data;

	for (j = 0; j < p->datasets; j++) {
		data = &p->data_buf[j * p->width];

		for (i = 0; i < p->data[j].len; i++) {
			if (data[i] > max) {
				max = data[i];
			}
			if (data[i] < min) {
				min = data[i];
			}
		}
	}

	if (max == min) {
		max += PLOT_DEFAULT_BOUND;
	}

	p->bounds.max = max;
	p->bounds.min = min;
}

bool
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
		return false;
	}

	if (!(plot->flags & plot_flag_fixed_bounds)) {
		set_auto_bounds(plot);
	}

	plot_display(plot);

	return true;
}
