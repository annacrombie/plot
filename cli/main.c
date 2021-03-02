#include "posix.h"

#include <plot/plot.h>
#include <stdio.h>

#include "cli/animate.h"
#include "cli/opts.h"

bool
animate_cb(struct plot *p)
{
	return plot_fetch(p, 1);
}

bool
follow_cb(struct plot *p)
{
	while (plot_fetch(p, 0)) {
	}
	return true;
}

#define BUFSIZE (MAX_WIDTH * MAX_HEIGHT)

int
main(int argc, char **argv)
{
	struct plot p = { 0 };
	static uint8_t canvas[MAX_WIDTH * MAX_HEIGHT];
	static double data_buf[MAX_WIDTH * MAX_HEIGHT];
	static struct plot_data pd[MAX_DATASETS];
	static char buf[MAX_WIDTH * MAX_HEIGHT];

	plot_init(&p, canvas, data_buf, pd, 24, 80, MAX_DATASETS);

	struct opts opts = { 0 };
	parse_opts(&opts, &p, argc, argv);

	switch (opts.mode) {
	case mode_normal:
		plot_fetch_until_full(&p);
		plot_string(&p, buf, BUFSIZE);
		fputs(buf, stdout);
		fflush(stdout);
		break;
	case mode_animate:
		animate_plot(&p, buf, BUFSIZE, opts.follow_rate, animate_cb);
		break;
	case mode_follow:
		animate_plot(&p, buf, BUFSIZE, opts.follow_rate, follow_cb);
		break;
	}
}
