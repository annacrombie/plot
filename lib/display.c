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
bufputs(struct buf *buf, const char *s)
{
	if (buf->file) {
		fputs(s, buf->file);
	} else {
		for (; buf->bufi < buf->buflen && *s; ++s, ++buf->bufi) {
			buf->buf[buf->bufi] = *s;
		}
	}
}

static void
bufprintf(struct buf *buf, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	if (buf->file) {
		vfprintf(buf->file, fmt, ap);
	} else {
		if (buf->bufi < buf->buflen) {
			buf->bufi += vsnprintf(&buf->buf[buf->bufi], buf->buflen - buf->bufi, fmt, ap);
		}
	}

	va_end(ap);
}

static void
bufputc(struct buf *buf, char c)
{
	if (buf->file) {
		fputc(c, buf->file);
	} else {
		if (buf->bufi < buf->buflen) {
			buf->buf[buf->bufi] = c;
			++buf->bufi;
		}
	}
}

static const char *
color_to_ansi_escape(enum plot_color color)
{
	switch (color) {
	case plot_color_black:     return "\033[30m";
	case plot_color_red:       return "\033[31m";
	case plot_color_green:     return "\033[32m";
	case plot_color_yellow:    return "\033[33m";
	case plot_color_blue:      return "\033[34m";
	case plot_color_magenta:   return "\033[35m";
	case plot_color_cyan:      return "\033[36m";
	case plot_color_white:     return "\033[37m";
	case plot_color_brblack:   return "\033[90m";
	case plot_color_brred:     return "\033[91m";
	case plot_color_brgreen:   return "\033[92m";
	case plot_color_bryellow:  return "\033[93m";
	case plot_color_brblue:    return "\033[94m";
	case plot_color_brmagenta: return "\033[95m";
	case plot_color_brcyan:    return "\033[96m";
	case plot_color_brwhite:   return "\033[97m";
	default: assert(false); return "";
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

static uint8_t
color_get(struct plot *p, uint16_t x, uint16_t y)
{

	return *canvas_get(p, x, y) >> 4;
}

static void
canvas_set(struct plot *p, uint8_t piece, enum plot_color color, uint32_t x, uint32_t y)
{
	uint8_t *cv = canvas_get(p, x, y);

	if (p->flags & plot_flag_merge_plot_pieces) {
		piece |= *cv & 0xf;
	}

	*cv = (color << 4) | piece;
}

static enum plot_piece
next_piece(long y, long cur, long next)
{
	enum plot_piece i;

	if (y == cur) {
		i = next > cur ? PPRightUp : next < cur ? PPRightDown : PPHoriz;
	} else if (y == next) {
		i = next > cur ? PPUpRight : next < cur ? PPDownRight : PPHoriz;
	} else if ((y > cur && y < next) || (y < cur && y > next)) {
		i = PPVert;
	} else {
		i = PPBlank;
	}

	return i;
}

static void
plot_fill_canvas(struct plot *plot)
{
	size_t i, x, y;
	double ratio, *data;
	uint32_t cur, nxt, e;
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

			if (cur > nxt) {
				y = nxt;
				e = cur;
			} else {
				y = cur;
				e = nxt;
			}

			for (; y <= e; y++) {
				next_p = next_piece(y, cur, nxt);
				assert(next_p != PPBlank);

				canvas_set(plot, next_p, plot->data[i].color, x, y);
			}
		}
	}
}

#define FMT_BUF_LEN 64

static void
plot_print_canvas(struct plot *plot, struct buf *buf)
{
	long x, y;
	enum plot_color color = 0, last_color = 0;
	uint8_t piece = 0;
	const double inc = (plot->bounds.max - plot->bounds.min)
			   / (double)(plot->height - 1);
	double label_num = plot->bounds.max;
	bool have_last_color = false;

	char y_fmt_l[FMT_BUF_LEN + 1];
	if (plot->y_label.side & plot_label_side_left) {
		snprintf(y_fmt_l, FMT_BUF_LEN, "%%%d.%df ",
			plot->y_label.width, plot->y_label.prec);
	}

	char y_fmt_r[FMT_BUF_LEN + 1];
	if (plot->y_label.side & plot_label_side_right) {
		snprintf(y_fmt_r, FMT_BUF_LEN, " %%-%d.%df",
			plot->y_label.width, plot->y_label.prec);
	}

	for (y = plot->height - 1; y >= 0; y--) {
		/* y label left */
		if (plot->y_label.side & plot_label_side_left) {
			color = color_get(plot, 0, y);
			piece = piece_get(plot, 0, y);
			piece = PPTLeft | ((piece & 0x8) >> 2);

			if (have_last_color) {
				bufputs(buf, "\033[0m");
				have_last_color = false;
			}

			bufprintf(buf, y_fmt_l, label_num);
			if (piece == PPCross && color) {
				bufputs(buf, color_to_ansi_escape(color));
				have_last_color = true;
				last_color = color;
			}

			bufputs(buf, plot->charset[piece]);
		}

		/* canvas */
		for (x = 0; x < (long)plot->width; x++) {
			color = color_get(plot, x, y);
			piece = piece_get(plot, x, y);

			if (color) {
				if (!have_last_color || color != last_color) {
					bufputs(buf, color_to_ansi_escape(color));
					have_last_color = true;
					last_color = color;
				}
			} else if (have_last_color) {
				bufputs(buf, "\033[0m");
				have_last_color = false;
			}

			bufputs(buf, plot->charset[piece]);
		}

		/* y label right */
		if (plot->y_label.side & plot_label_side_right) {
			piece = PPTRight | piece;

			if (piece == PPCross && color) {
				bufputs(buf, color_to_ansi_escape(color));
			}

			bufputs(buf, plot->charset[piece]);

			if (color) {
				bufputs(buf, "\033[0m");
			}

			bufprintf(buf, y_fmt_r, label_num);
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
	char fmt[FMT_BUF_LEN + 1], clrfmt[FMT_BUF_LEN + 1];

	if (p->x_label.every <= 0) {
		return;
	}

	every = p->x_label.every;
	if (!every) {
		every = 1;
	}

	start = p->x_label.start;

	snprintf(fmt, FMT_BUF_LEN, "%%-%dld", every);

	if (p->x_label.color) {
		snprintf(clrfmt, FMT_BUF_LEN, "%%s%%-%dld\033[0m", every);
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

		if (tmp == 0 && p->x_label.color) {
			bufprintf(buf, clrfmt, color_to_ansi_escape(p->x_label.color), tmp);
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
