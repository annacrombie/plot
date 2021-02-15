#include "posix.h"

#include "animate.h"
#include "data_pipe.h"
#include "input.h"
#include "log.h"
#include "opts.h"
#include "plot.h"

bool
animate_cb(struct plot *p)
{
	return pipeline_exec_all(p, 1);
}

bool
follow_cb(struct plot *p)
{
	pipeline_reset_eofs();
	pipeline_fast_fwd(p);
	return true;
}

int
main(int argc, char **argv)
{
	struct plot p = { 0 };
	plot_init(&p);

	logfile = stderr;

	parse_opts(&p, argc, argv);

	if (p.flags & plot_flag_animate) {
		animate_plot(&p, p.follow_rate, animate_cb);
	} else if (p.flags & plot_flag_follow) {
		animate_plot(&p, p.follow_rate, follow_cb);
	} else {
		pipeline_fast_fwd(&p);
		plot_plot(&p);
	}

	fflush(stdout);

	return 0;
}
