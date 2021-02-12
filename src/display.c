#include "posix.h"

#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "display.h"
#include "plot.h"
#include "util.h"

#define _X_ "x"

/*
   " ╔║╚╠╗═╦╝╣╩╬"
   " ╭│╰├╮─┬╯┤┴┼"
 */

static enum plot_piece
piece_get(struct plot *p, uint16_t x, uint16_t y)
{
	return p->canvas[x][y] & 0xf;
}

static void
piece_set(struct plot *p, uint16_t x, uint16_t y, enum plot_piece pp)
{
	p->canvas[x][y] = (p->canvas[x][y] & 0xf0) | pp;
}

static uint8_t
color_get(struct plot *p, uint16_t x, uint16_t y)
{
	return p->canvas[x][y] >> 4;
}

static void
color_set(struct plot *p, uint16_t x, uint16_t y, enum color c)
{
	p->canvas[x][y] = c << 4 | (p->canvas[x][y] & 0xf);
}

static char plot_charsets[][16][4] = {
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
set_custom_plot_charset(char *str, size_t len)
{
	size_t i, j, k;
	unsigned int bytes;

	for (i = j = 0; i < 16; i++) {
		if (i == 1 || i == 2 || i == 4 || i == 8) {
			continue;
		}

		bytes = utf8_bytes(&str[j]);

		if (bytes + j > len) {
			return;
		}

		for (k = 0; k < bytes; k++) {
			plot_charsets[PCCUSTOM][i][k] = str[j + k];
		}

		j += k;
	}
}

static enum plot_piece
next_piece(long y, long cur, long next)
{
	enum plot_piece i;

	if (y == cur) {
		i = next > cur ? PPRightUp : next < cur ? PPRightDown : PPHoriz;
	}else if (y == next) {
		i = next > cur ? PPUpRight : next < cur ? PPDownRight : PPHoriz;
	}else if ((y > cur && y < next) || (y < cur && y > next)) {
		i = PPVert;
	}else {
		i = PPBlank;
	}

	return i;
}

static void
plot_fill_canvas(struct plot *plot)
{
	size_t i, x, y;
	double ratio;
	long cur, nxt;
	enum plot_piece next_p;

	memset(plot->canvas, 0, MAX_WIDTH * MAX_HEIGHT);

	ratio = (double)(plot->height - 1) / (plot->bounds.max - plot->bounds.min);

	for (i = 0; i < plot->datasets; i++) {
		if (!plot->data[i].len) {
			continue;
		}

		nxt = lround((plot->data[i].data[0] - plot->bounds.min) * ratio);

		for (x = 0; x < plot->data[i].len; x++) {
			cur = nxt;

			if (x < plot->data[i].len - 1) {
				nxt = lround((plot->data[i].data[x + 1] - plot->bounds.min) * ratio);
			}

			for (y = 0; y < plot->height; y++) {
				if ((next_p = next_piece(y, cur, nxt)) == PPBlank) {
					continue;
				}

				if (plot->flags & plot_flag_merge_plot_pieces) {
					next_p |= piece_get(plot, x, y);
				}

				color_set(plot, x, y, plot->data[i].color);
				piece_set(plot, x, y, next_p);
			}
		}
	}
}

static char *yl_l_fmt_fmt = "%%%d.%df ";
static char *yl_r_fmt_fmt = " %%-%d.%df";

static void
plot_y_label_init_fmts(struct y_label *yl, enum side side)
{
	if (side == side_left) {
		if (!*yl->l_fmt) {
			snprintf(yl->l_fmt, CHARBUF, yl_l_fmt_fmt, yl->width, yl->prec);
		}
	} else if (side == side_right) {
		if (!*yl->r_fmt) {
			snprintf(yl->r_fmt, CHARBUF, yl_r_fmt_fmt, yl->width, yl->prec);
		}
	}
}

static void
plot_print_y_label(struct plot *p, canvas_elem e, double l, enum side side)
{
	enum plot_piece pp, e_piece = e & 0xf;
	enum color e_color = e >> 4;

	plot_y_label_init_fmts(&p->y_label, side);

	if (side == side_left) {
		pp = PPTLeft | ((e_piece & 0x8) >> 2);
	} else {
		pp = PPTRight | e_piece;
	}

	if (side == side_left) {
		if (p->flags & plot_flag_color) {
			printf("\033[0m");
		}
		printf(p->y_label.l_fmt, l);
	}

	if (pp == PPCross && e_color > 0) {
		printf("\033[%dm", color_to_ansi_escape_color(e_color));
	} else if (p->flags & plot_flag_color) {
		printf("\033[0m");
	}

	printf("%s", plot_charsets[p->charset][pp]);

	if (side == side_right) {
		if (p->flags & plot_flag_color) {
			printf("\033[0m");
		}
		printf(p->y_label.r_fmt, l);
	}
}

static void
plot_print_canvas(struct plot *plot)
{
	long x, y;
	enum color color;

	for (y = plot->height - 1; y >= 0; y--) {
		if (plot->y_label.side & side_left) {
			plot_print_y_label(plot, plot->canvas[0][y], plot->labels[y], 1);
		}

		for (x = 0; x < (long)plot->width; x++) {
			color = color_get(plot, x, y);
			if (color) {
				printf("\033[%dm", color_to_ansi_escape_color(color));
			}

			printf("%s", plot_charsets[plot->charset][piece_get(plot, x, y)]);

			if (color) {
				printf("\033[0m");
			}
		}

		if (plot->y_label.side & side_right) {
			plot_print_y_label(plot, plot->canvas[plot->width - 1][y], plot->labels[y], 2);
		}

		printf("\n");
	}

	if (plot->flags & plot_flag_color) {
		printf("\033[0m");
	}
}

static void
plot_print_x_label(struct plot *p, char *buf)
{
	long end, tmp, i;
	int every, start;
	size_t printed = 0;
	char fmt[2][32] = { 0 };

	if (p->x_label.every <= 0) {
		return;
	}

	/* TODO is it possible to do this in the new system?  If not, remove
	 * these redundant checks
	 */
	/* every = p->x_label.every / p->average; */
	every = p->x_label.every;
	if (!every) {
		every = 1;
	}

	/* start = p->x_label.start / p->average; */
	start = p->x_label.start;

	snprintf(fmt[0], 31, "%%-%dld", every);

	if (p->x_label.color) {
		snprintf(fmt[1], 31, "\033[%%dm%%-%dld\033[0m", every);
	}

	if (start % every > 0) {
		tmp = every - (start % every);
	} else {
		tmp = 0;
	}

	if (p->y_label.side & side_left) {
		end = p->y_label.width + 2 + tmp;
	} else {
		end = tmp;
	}

	for (i = 0; i < end; i++) {
		buf[printed++] = ' ';
	}

	end = start + p->width;
	for (i = start + tmp; i < end; i += every) {
		tmp = i < 0 ? -1 : 1;
		tmp = p->x_label.mod > 0 ? tmp * (i % p->x_label.mod) : i;
		/* tmp *= p->average; */

		if (tmp == 0 && p->x_label.color) {
			printed += snprintf(&buf[printed], MAX_WIDTH - printed,
				fmt[1], color_to_ansi_escape_color(p->x_label.color), tmp);
		} else {
			printed += snprintf(&buf[printed], MAX_WIDTH - printed,
				fmt[0], tmp);
		}
	}

	if (printed < MAX_WIDTH) {
		buf[printed] = '\n';
	}
}

void
plot_display(struct plot *plot)
{
	char x_label[MAX_WIDTH] = { 0 };
	if (plot->x_label.side && plot->x_label.every) {
		plot_print_x_label(plot, x_label);
	}

	/* create the graph */
	plot_fill_canvas(plot);

	if (plot->x_label.side & side_top) {
		printf("%s", x_label);
	}

	/* print the graph with labels */
	plot_print_canvas(plot);

	if (plot->x_label.side & side_bottom) {
		printf("%s", x_label);
	}
}
