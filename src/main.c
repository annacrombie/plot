#include "posix.h"

#include "animate.h"
#include "log.h"
#include "opts.h"
#include "plot.h"

void
fast_fwd(struct plot *p)
{
	while (plot_fetch(p, 0)) {
	}
}

bool
animate_cb(struct plot *p)
{
	return plot_fetch(p, 1);
}

bool
follow_cb(struct plot *p)
{
	fast_fwd(p);
	return true;
}

int
main(int argc, char **argv)
{
	struct plot p = { 0 };
	static uint8_t canvas[MAX_WIDTH * MAX_HEIGHT];
	static double data_buf[MAX_WIDTH * MAX_HEIGHT];

	plot_init(&p, canvas, data_buf, 24, 80);

	logfile = stderr;

	parse_opts(&p, argc, argv);

	if (p.flags & plot_flag_animate) {
		animate_plot(&p, p.follow_rate, animate_cb);
	} else if (p.flags & plot_flag_follow) {
		animate_plot(&p, p.follow_rate, follow_cb);
	} else {
		fast_fwd(&p);
		plot_plot(&p);
	}

	fflush(stdout);

	return 0;
}
