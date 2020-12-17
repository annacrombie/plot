#include "display.h"
#include "plot.h"
#include "util.h"

#define _X_ "x"

/*
   " ╔║╚╠╗═╦╝╣╩╬"
   " ╭│╰├╮─┬╯┤┴┼"
 */


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

static enum plot_peice
next_peice(long y, long cur, long next)
{
	enum plot_peice i;

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
plot_write_norm(struct plot *plot, long *norm)
{
	long x, y;
	enum plot_peice next;

	for (x = 2; x < norm[0] && x - 2 < (long)plot->width; x++) {
		for (y = 0; y < (long)plot->height; y++) {
			next = next_peice(
				y,
				norm[x],
				x + 1 >= norm[0] ? norm[x] :  norm[x + 1]
				);

			if (next == PPBlank) {
				continue;
			}

			if (plot->merge_plot_peices) {
				next = plot->canvas[x - 2][y]->peice | next;
			}

			plot->canvas[x - 2][y]->color = norm[1];
			plot->canvas[x - 2][y]->peice = next;
		}
	}
}

static void
plot_fill_canvas(struct plot *plot)
{
	size_t x, y, i;

	for (x = 0; x < plot->width; x++) {
		for (y = 0; y < plot->height; y++) {
			plot->canvas[x][y]->color = 0;
			plot->canvas[x][y]->peice = PPBlank;
		}
	}

	for (i = 0; i < plot->datasets; i++) {
		plot_write_norm(plot, plot->normalized[i]);
	}
}

static char *yl_l_fmt_fmt = "%%%d.%df ";
static char *yl_r_fmt_fmt = " %%-%d.%df";

static void
plot_y_label_init_fmts(struct y_label *yl, int side)
{
	if (side == 1) {
		if (!*yl->l_fmt) {
			snprintf(yl->l_fmt, CHARBUF, yl_l_fmt_fmt, yl->width, yl->prec);
		}
	} else if (side == 2) {
		if (!*yl->r_fmt) {
			snprintf(yl->r_fmt, CHARBUF, yl_r_fmt_fmt, yl->width, yl->prec);
		}
	}
}

static void
plot_print_y_label(struct plot *p, struct canvas_elem e, double l, int side)
{
	enum plot_peice pp;

	plot_y_label_init_fmts(p->y_label, side);

	pp = side == 1 ? PPTLeft | ((e.peice & 0x8) >> 2) : PPTRight | e.peice;

	if (side == 1) {
		printf("\033[0m");
		printf(p->y_label->l_fmt, l);
	}

	if (pp == PPCross && e.color > 0) {
		printf("\033[%dm", e.color);
	} else if (p->color) {
		printf("\033[0m");
	}

	printf("%s", plot_charsets[p->charset][pp]);

	if (side == 2) {
		printf("\033[0m");
		printf(p->y_label->r_fmt, l);
	}
}

static void
plot_print_canvas(struct plot *plot)
{
	long x, y;

	for (y = plot->height - 1; y >= 0; y--) {
		if ((plot->y_label->side & 1) == 1) {
			plot_print_y_label(plot, *plot->canvas[0][y], plot->labels[y], 1);
		}

		for (x = 0; x < (long)plot->width; x++) {
			if (plot->canvas[x][y]->color > 0) {
				printf("\033[%dm", plot->canvas[x][y]->color);
			}

			printf("%s", plot_charsets[plot->charset][plot->canvas[x][y]->peice]);

			if (plot->canvas[x][y]->color > 0) {
				printf("\033[%dm", 0);
			}
		}

		if ((plot->y_label->side & 2) == 2) {
			plot_print_y_label(plot, *plot->canvas[plot->width - 1][y], plot->labels[y], 2);
		}

		printf("\n");
	}

	if (plot->color) {
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

	if (p->x_label->every <= 0) {
		return;
	}

	every = p->x_label->every / p->average;
	if (!every) {
		every = 1;
	}

	start = p->x_label->start / p->average;

	snprintf(fmt[0], 31, "%%-%dld", every);

	if (p->x_label->color) {
		snprintf(fmt[1], 31, "\033[%%dm%%-%dld\033[0m", every);
	}

	if (start % every > 0) {
		tmp = every - (start % every);
	} else {
		tmp = 0;
	}

	end = (p->y_label->side & 1 ? p->y_label->width + 2 : 0) + tmp;
	for (i = 0; i < end && i < MAX_WIDTH; i++) {
		buf[printed++] = ' ';
	}

	end = start + p->width;
	for (i = start + tmp; i < end; i += every) {
		tmp = i < 0 ? -1 : 1;
		tmp = p->x_label->mod > 0 ? tmp * (i % p->x_label->mod) : i;
		tmp *= p->average;

		if (tmp == 0 && p->x_label->color) {
			printed += snprintf(&buf[printed], MAX_WIDTH - printed,
				fmt[1], p->x_label->color, tmp);
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
	if (plot->x_label->side && plot->x_label->every) {
		plot_print_x_label(plot, x_label);
	}

	/* create the graph */
	plot_fill_canvas(plot);

	if (plot->x_label->side & 2) {
		printf("%s", x_label);
	}

	/* print the graph with labels */
	plot_print_canvas(plot);

	if (plot->x_label->side & 1) {
		printf("%s", x_label);
	}
}
