#include "posix.h"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "internal/display.h"
#include "internal/log.h"
#include "plot/plot.h"

struct buf {
	FILE *file;
	char *buf;
	uint32_t bufi;
	uint32_t buflen;
};

static void
bufprintf(struct buf *buf, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	if (buf->file) {
		vfprintf(buf->file, fmt, ap);
	} else {
		if (buf->bufi >= buf->buflen) {
			return;
		}

		buf->bufi += vsnprintf(&buf->buf[buf->bufi], buf->buflen - buf->bufi, fmt, ap);
	}
	va_end(ap);
}

static void
bufputc(struct buf *buf, char c)
{
	if (buf->file) {
		fputc(c, buf->file);
	} else {
		if (buf->bufi >= buf->buflen) {
			return;
		}

		buf->buf[buf->bufi] = c;
		++buf->bufi;
	}
}

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

uint8_t *
canvas_get(struct plot *p, uint16_t x, uint16_t y)
{
	return &p->canvas[(x * p->height) + y];
}

static enum plot_piece
piece_get(struct plot *p, uint16_t x, uint16_t y)
{
	return *canvas_get(p, x, y) & 0xf;
}

static void
piece_set(struct plot *p, uint16_t x, uint16_t y, enum plot_piece pp)
{
	uint8_t *cv = canvas_get(p, x, y);
	*cv = (*cv & 0xf0) | pp;
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

	return *canvas_get(p, x, y) >> 4;
}

static void
color_set(struct plot *p, uint16_t x, uint16_t y, enum plot_color c)
{
	uint8_t *cv = canvas_get(p, x, y);
	*cv = c << 4 | (*cv & 0xf);
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
	double ratio, *data;
	long cur, nxt;
	enum plot_piece next_p;

	memset(plot->canvas, 0, plot->height * plot->width);

	ratio = (double)(plot->height - 1) / (plot->bounds.max - plot->bounds.min);

	for (i = 0; i < plot->datasets; i++) {
		if (!plot->data[i].len) {
			continue;
		}

		data = &plot->data_buf[i * plot->width];

		nxt = lround((data[0] - plot->bounds.min) * ratio);

		for (x = 0; x < plot->data[i].len; x++) {
			cur = nxt;

			if (x < plot->data[i].len - 1) {
				nxt = lround((data[x + 1] - plot->bounds.min) * ratio);
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
plot_print_y_label(struct plot *p, struct buf *buf, uint8_t e, double l, enum plot_label_side side)
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
			bufprintf(buf, "\033[0m");
		}

		snprintf(fmt, FMT_BUF_LEN, "%%%d.%df ", p->y_label.width, p->y_label.prec);
		bufprintf(buf, fmt, l);
	}

	if (pp == PPCross && e_color > 0) {
		bufprintf(buf, "\033[%dm", color_to_ansi_escape_color(e_color));
	} else if (p->flags & plot_flag_color) {
		bufprintf(buf, "\033[0m");
	}

	bufprintf(buf, "%s", p->charset[pp]);

	if (side == plot_label_side_right) {
		if (p->flags & plot_flag_color) {
			bufprintf(buf, "\033[0m");
		}

		snprintf(fmt, FMT_BUF_LEN, " %%-%d.%df", p->y_label.prec, p->y_label.width);
		bufprintf(buf, fmt, l);
	}
}

static void
plot_print_canvas(struct plot *plot, struct buf *buf)
{
	long x, y;
	enum plot_color color;
	const double inc = (plot->bounds.max - plot->bounds.min)
			   / (double)(plot->height - 1);
	double label_num = plot->bounds.max;

	for (y = plot->height - 1; y >= 0; y--) {
		if (plot->y_label.side & plot_label_side_left) {
			plot_print_y_label(plot, buf, *canvas_get(plot, 0, y), label_num, 1);
		}

		for (x = 0; x < (long)plot->width; x++) {
			color = color_get(plot, x, y);
			if (color) {
				bufprintf(buf, "\033[%dm", color_to_ansi_escape_color(color));
			}

			bufprintf(buf, "%s", plot->charset[piece_get(plot, x, y)]);

			if (color) {
				bufprintf(buf, "\033[0m");
			}
		}

		if (plot->y_label.side & plot_label_side_right) {
			plot_print_y_label(plot, buf, *canvas_get(plot, plot->width - 1, y), label_num, 2);
		}

		bufputc(buf, '\n');

		label_num -= inc;
	}

	if (plot->flags & plot_flag_color) {
		bufprintf(buf, "\033[0m");
	}
}

static void
plot_print_x_label(struct plot *p, struct buf *buf)
{
	long end, tmp, i;
	int every, start;
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
		bufputc(buf, ' ');
	}

	end = start + p->width;
	for (i = start + tmp; i < end; i += every) {
		tmp = i < 0 ? -1 : 1;
		tmp = p->x_label.mod > 0 ? tmp * (i % p->x_label.mod) : i;
		/* tmp *= p->average; */

		if (tmp == 0 && p->x_label.color) {
			bufprintf(buf, fmt, color_to_ansi_escape_color(p->x_label.color), tmp);
		} else {
			bufprintf(buf, fmt, tmp);
		}
	}

	bufputc(buf, '\n');
}

static void
plot_render(struct plot *plot, struct buf *buf)
{
	if (plot->x_label.side & plot_label_side_top) {
		plot_print_x_label(plot, buf);
	}

	plot_fill_canvas(plot);
	plot_print_canvas(plot, buf);

	if (plot->x_label.side & plot_label_side_bottom) {
		plot_print_x_label(plot, buf);
	}

	bufputc(buf, 0);
}

void
plot_render_file(struct plot *plot, FILE *f)
{
	struct buf buf = { .file = f };

	plot_render(plot, &buf);
}

void
plot_render_string(struct plot *plot, char *bufmem, uint32_t buflen)
{
	struct buf buf = { .buf = bufmem, .buflen = buflen };

	assert(buflen);

	plot_render(plot, &buf);
}
