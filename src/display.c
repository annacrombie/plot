#include "posix.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "display.h"
#include "plot.h"

/* A piece can be defined by the four sides it touches of the cell it contains.
 * The convention used is that the South side is the starting point, and you go
 * counter-clockwise.  So the piece pointing South and North (│) is represented
 * by 0101 (binary) or 5.
 *
 * Here is a chart of all the pieces
 * piece | binary | hex
 * ' '   | 0000   | 0
 * ╭     | 0011   | 3
 * │     | 0101   | 5
 * ╰     | 0110   | 6
 * ├     | 0111   | 7
 * ╮     | 1001   | 9
 * ─     | 1010   | a
 * ┬     | 1011   | b
 * ╯     | 1100   | c
 * ┤     | 1101   | d
 * ┴     | 1110   | e
 * ┼     | 1111   | f
 */

enum plot_piece {
	PPBlank     = 0x0,
	PPUpRight   = 0x3,
	PPVert      = 0x5,
	PPDownRight = 0x6,
	PPTRight    = 0x7,
	PPRightDown = 0x9,
	PPHoriz     = 0xa,
	PPTDown     = 0xb,
	PPRightUp   = 0xc,
	PPTLeft     = 0xd,
	PPTUp       = 0xe,
	PPCross     = 0xf,
};

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
color_to_ansi_escape_color(enum plot_color color)
{
	switch (color) {
	case plot_color_black:     return 30;
	case plot_color_red:       return 31;
	case plot_color_green:     return 32;
	case plot_color_yellow:    return 33;
	case plot_color_blue:      return 34;
	case plot_color_magenta:   return 35;
	case plot_color_cyan:      return 36;
	case plot_color_white:     return 37;
	case plot_color_brblack:   return 90;
	case plot_color_brred:     return 91;
	case plot_color_brgreen:   return 92;
	case plot_color_bryellow:  return 93;
	case plot_color_brblue:    return 94;
	case plot_color_brmagenta: return 95;
	case plot_color_brcyan:    return 96;
	case plot_color_brwhite:   return 97;
	default: assert(false); return 0;
	}
}

static uint8_t
color_get(struct plot *p, uint16_t x, uint16_t y)
{
	return p->canvas[x][y] >> 4;
}

static void
color_set(struct plot *p, uint16_t x, uint16_t y, enum plot_color c)
{
	p->canvas[x][y] = c << 4 | (p->canvas[x][y] & 0xf);
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

#define FMT_BUF_LEN 64

static void
plot_print_y_label(struct plot *p, uint8_t e, double l, enum plot_label_side side)
{
	enum plot_piece pp, e_piece = e & 0xf;
	enum plot_color e_color = e >> 4;
	char fmt[FMT_BUF_LEN + 1];

	/* plot_y_label_init_fmts(&p->y_label, side); */

	if (side == plot_label_side_left) {
		pp = PPTLeft | ((e_piece & 0x8) >> 2);
	} else {
		pp = PPTRight | e_piece;
	}

	if (side == plot_label_side_left) {
		if (p->flags & plot_flag_color) {
			printf("\033[0m");
		}

		snprintf(fmt, FMT_BUF_LEN, "%%%d.%df ", p->y_label.width, p->y_label.prec);
		printf(fmt, l);
	}

	if (pp == PPCross && e_color > 0) {
		printf("\033[%dm", color_to_ansi_escape_color(e_color));
	} else if (p->flags & plot_flag_color) {
		printf("\033[0m");
	}

	printf("%s", p->charset[pp]);

	if (side == plot_label_side_right) {
		if (p->flags & plot_flag_color) {
			printf("\033[0m");
		}

		snprintf(fmt, FMT_BUF_LEN, " %%-%d.%df", p->y_label.prec, p->y_label.width);
		printf(fmt, l);
	}
}

static void
plot_print_canvas(struct plot *plot)
{
	long x, y;
	enum plot_color color;
	const double inc = (plot->bounds.max - plot->bounds.min)
			   / (double)(plot->height - 1);
	double label_num = plot->bounds.max;

	for (y = plot->height - 1; y >= 0; y--) {
		label_num -= inc;

		if (plot->y_label.side & plot_label_side_left) {
			plot_print_y_label(plot, plot->canvas[0][y], label_num, 1);
		}

		for (x = 0; x < (long)plot->width; x++) {
			color = color_get(plot, x, y);
			if (color) {
				printf("\033[%dm", color_to_ansi_escape_color(color));
			}

			printf("%s", plot->charset[piece_get(plot, x, y)]);

			if (color) {
				printf("\033[0m");
			}
		}

		if (plot->y_label.side & plot_label_side_right) {
			plot_print_y_label(plot, plot->canvas[plot->width - 1][y], label_num, 2);
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
	char fmt[FMT_BUF_LEN + 1];

	if (p->x_label.every <= 0) {
		return;
	}

	every = p->x_label.every;
	if (!every) {
		every = 1;
	}

	start = p->x_label.start;

	if (p->x_label.color) {
		snprintf(fmt, FMT_BUF_LEN, "\033[%%dm%%-%dld\033[0m", every);
	} else {
		snprintf(fmt, FMT_BUF_LEN, "%%-%dld", every);
	}

	if (start % every > 0) {
		tmp = every - (start % every);
	} else {
		tmp = 0;
	}

	if (p->y_label.side & plot_label_side_left) {
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
				fmt, color_to_ansi_escape_color(p->x_label.color), tmp);
		} else {
			printed += snprintf(&buf[printed], MAX_WIDTH - printed,
				fmt, tmp);
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

	if (plot->x_label.side & plot_label_side_top) {
		printf("%s", x_label);
	}

	/* print the graph with labels */
	plot_print_canvas(plot);

	if (plot->x_label.side & plot_label_side_bottom) {
		printf("%s", x_label);
	}
}
