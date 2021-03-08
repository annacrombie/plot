#include "posix.h"

#include <stdio.h>

#include "cli/animate.h"
#include "cli/main.h"
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

int
main(int argc, char **argv)
{
	static struct plot_static_memory mem = { 0 };
	struct opts opts = { 0 };
	struct plot *p = &mem.plot;

	plot_init(p, mem.canvas, mem.data_buf, mem.pd, 24, 80, MAX_DATASETS);

	parse_opts(&opts, p, &mem, argc, argv);

	switch (opts.mode) {
	case mode_normal:
		plot_fetch_until_full(p);
		plot_string(p, mem.out_buf, OUT_BUF);
		fputs(mem.out_buf, stdout);
		fflush(stdout);
		break;
	case mode_animate:
		animate_plot(p, mem.out_buf, OUT_BUF, opts.follow_rate, animate_cb);
		break;
	case mode_follow:
		animate_plot(p, mem.out_buf, OUT_BUF, opts.follow_rate, follow_cb);
		break;
	}
}
