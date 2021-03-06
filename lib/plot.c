#include "posix.h"

#include <float.h>
#include <stdlib.h>
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
	};

	plot_set_charset(plot, plot_charset_unicode);
}

struct plot *
plot_alloc(uint32_t height, uint32_t width, uint32_t depth)
{
	uint32_t offs[] = {
		sizeof(struct plot),
		height * width,
		sizeof(double) * width * depth,
		sizeof(struct plot_data) * depth,
	};

	uint8_t *mem = calloc(1, offs[0] + offs[1] + offs[2] + offs[3]);

	plot_init((struct plot *)mem,
		mem + offs[0],
		(double *)(mem + offs[0] + offs[1]),
		(struct plot_data *)(mem + offs[0] + offs[1] + offs[2]),
		height, width, depth);

	return (struct plot *)mem;
}

void
plot_free(struct plot *p)
{
	free(p);
}

void
plot_set_custom_charset(struct plot *plot, char *str, uint32_t len)
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
	if (plot->datasets >= plot->depth) {
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


void
plot_fix_bounds(struct plot *p, double min, double max)
{
	p->flags |= plot_flag_fixed_bounds;
	p->bounds.min = min;
	p->bounds.max = max;
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

static bool
plot_prepare(struct plot *plot)
{
	uint32_t i;
	bool no_data = true;

	for (i = 0; i < plot->datasets; i++) {
		if (plot->data[i].len > 0) {
			no_data = false;
			break;
		}
	}

	if (plot->datasets < 1 || no_data) {
		return false;
	}

	if (!(plot->flags & plot_flag_fixed_bounds)) {
		set_auto_bounds(plot);
	}

	return true;
}

void
plot_print(struct plot *plot, FILE *f)
{
	if (!plot_prepare(plot)) {
		return;
	}

	plot_render_file(plot, f);
}

void
plot_string(struct plot *plot, char *buf, uint32_t buflen)
{
	if (!plot_prepare(plot)) {
		return;
	}

	plot_render_string(plot, buf, buflen);
}
