#include "posix.h"

#include "follow.h"
#include "input.h"
#include "opts.h"
#include "plot.h"

int
main(int argc, char **argv)
{
	struct plot p = { 0 };
	plot_init(&p);

	int lc = parse_opts(&p, argc, argv);

	if (p.datasets == 0) {
		plot_add(&p, stdin, lc);
	}

	if (p.flags & (plot_flag_animate|plot_flag_follow)) {
		set_input_buffer_size(8);
		follow_plot(&p, p.follow_rate);
	} else {
		pdread_all_available(&p);
		plot_plot(&p);
	}

	plot_destroy(&p);

	fflush(stdout);

	return 0;
}
