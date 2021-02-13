#include "posix.h"

#include "animate.h"
#include "data_pipe.h"
#include "input.h"
#include "log.h"
#include "opts.h"
#include "plot.h"

int
main(int argc, char **argv)
{
	struct plot p = { 0 };
	plot_init(&p);

	logfile = stderr;

	int lc = parse_opts(&p, argc, argv);

	if (p.datasets == 0) {
		if (!pipeline_create("-")) {
			return 1;
		}
		plot_add(&p, lc);
	}

	if (p.flags & (plot_flag_animate | plot_flag_follow)) {
		/* set_input_buffer_size(8); */
		// TODO
		follow_plot(&p, p.follow_rate);
	} else {
		/* pdread_all_available(&p); */
		pipeline_exec_all(&p);
		plot_plot(&p);
	}

	fflush(stdout);

	return 0;
}
