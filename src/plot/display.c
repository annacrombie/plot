#include "../util.h"
#include "plot.h"

/* A peice can be defined by the four sides it touches of the cell it contains.
 * The convention used is that the South side is the starting point, and you go
 * counter-clockwise.  So the peice pointing South and North (│) is represented
 * by 0101 (binary) or 5.
 *
 * Here is a chart of all the peices
 * peice | binary | hex
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

static char plot_peices[][4] = {
	" ", " ", " ", "╭", " ", "│", "╰", "├",
	" ", "╮", "─", "┬", "╯", "┤", "┴", "┼",
};

enum plot_peice {
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

struct canvas_elem {
	enum plot_peice peice;
	unsigned int color;
};

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

static void plot_write_norm(struct plot *plot, long *norm, struct canvas_elem **c)
{
	size_t x, y;
	enum plot_peice next;

	for (x = 2; x < norm[0] && x - 2 < plot->width; x++) {
		for (y = 0; y < plot->height; y++) {
			next = next_peice(
				y,
				norm[x],
				x + 1 >= norm[0] ? norm[x] :  norm[x + 1]
				);

			if (next == PPBlank)
				continue;

			if (plot->merge_plot_peices)
				next = c[x - 2][y].peice | next;

			c[x - 2][y].color = norm[1];
			c[x - 2][y].peice = next;
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

static char *yl_l_fmt_fmt = "%%%d.%df %%s";
static char *yl_r_fmt_fmt = "%%s %%-%d.%df";

static void plot_y_label_init_fmts(struct y_label *yl, int side)
{
	int chars;

	if (side == 1) {
		if (yl->l_fmt != NULL)
			return;
		chars = snprintf(NULL, 0, yl_l_fmt_fmt, yl->width, yl->prec) + 1;
		yl->l_fmt = safe_calloc(chars, sizeof(char));
		snprintf(yl->l_fmt, chars, yl_l_fmt_fmt, yl->width, yl->prec);
	} else if (side == 2) {
		if (yl->r_fmt != NULL)
			return;
		chars = snprintf(NULL, 0, yl_r_fmt_fmt, yl->width, yl->prec) + 1;
		yl->r_fmt = safe_calloc(chars, sizeof(char));
		snprintf(yl->r_fmt, chars, yl_r_fmt_fmt, yl->width, yl->prec);
	}
}

static void
plot_print_y_label(struct plot *p, struct canvas_elem e, double l, int side)
{
	struct y_label *yl = p->y_label;
	enum plot_peice pp;

	plot_y_label_init_fmts(yl, side);

	pp = side == 1 ? PPTLeft | ((e.peice & 0x8) >> 2) : PPTRight | e.peice;

	if (pp == PPCross && e.color > 0)
		printf("\e[%dm", e.color);
	else if (p->color)
		printf("%c[0m", 27);

	if (side == 1)
		printf(yl->l_fmt, l, plot_peices[pp]);
	else if (side == 2)
		printf(yl->r_fmt, plot_peices[pp], l);
}

static void
plot_print_canvas(struct plot *plot, double *labels, struct canvas_elem **canvas)
{
	long x, y;

	for (y = plot->height - 1; y >= 0; y--) {
		if ((plot->y_label->side & 1) == 1)
			plot_print_y_label(plot, canvas[0][y], labels[y], 1);

		for (x = 0; x < plot->width; x++) {
			if (canvas[x][y].color > 0)
				printf("\e[%dm", canvas[x][y].color);

			printf("%s", plot_peices[canvas[x][y].peice]);
		}

		if ((plot->y_label->side & 2) == 2)
			plot_print_y_label(plot, canvas[0][y], labels[y], 2);

		printf("\n");
	}

	if (plot->color)
		printf("%c[0m", 27);
}

static void plot_print_x_label(struct plot *p)
{
	struct x_label *xl = p->x_label;
	long end, tmp, i;
	char fmt[2][20];

	if (xl->every <= 0)
		return;

	snprintf(fmt[0], 20, "%%-%dld", xl->every);

	if (xl->color > 0)
		snprintf(fmt[1], 20, "%c[%%dm%%-%dld%c[0m", 27, xl->every, 27);

	tmp = xl->start % xl->every > 0 ? xl->every - (xl->start % xl->every) : 0;

	end = (p->y_label->side & 1 ? p->y_label->width + 2 : 0) + tmp;
	for (i = 0; i < end; i++)
		printf(" ");

	end = xl->start + p->width;
	for (i = xl->start + tmp; i < end; i += xl->every) {
		tmp = (i < 0 ? -1 : 1) * (xl->mod > 0 ? i % xl->mod : i);

		if (tmp == 0 && xl->color > 0)
			printf(fmt[1], xl->color, tmp);
		else
			printf(fmt[0], tmp);
	}

	printf("\n");
}

void plot_display(struct plot *plot, double *labels, long **norm)
{
	size_t i;

	/* create the graph */
	struct canvas_elem **canvas = plot_fill_canvas(plot, norm);

	if (plot->x_label->side & 2)
		plot_print_x_label(plot);

	/* print the graph with labels */
	plot_print_canvas(plot, labels, canvas);

	if (plot->x_label->side & 1)
		plot_print_x_label(plot);

	for (i = 0; i < plot->width; i++)
		free(canvas[i]);
	free(canvas);
}
