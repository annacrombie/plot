#define _POSIX_C_SOURCE 199309L

#include <signal.h>

#include "follow.h"
#include "input.h"
#include "opts.h"
#include "plot.h"

struct sigaction sigact = { 0 };

static void
handle_sigint(int _)
{
	printf("%c[?12l%c[?25h", 27, 27);
	exit(0);
}

static void
install_signal_handler(void)
{
	sigact.sa_flags = 0;
	sigact.sa_handler = handle_sigint;
	sigaction(SIGINT, &sigact, NULL);
}

int
main(int argc, char **argv)
{
	struct plot p = { 0 };
	plot_init(&p);

	int lc = parse_opts(&p, argc, argv);

	if (p.datasets == 0) {
		plot_add(&p, stdin, lc);
	}

	input_init();

	if (p.follow) {
		install_signal_handler();
		set_input_buffer_size(5);
		follow_plot(&p);
	} else {
		pdread_all_available(&p);
		plot_plot(&p);
	}

	plot_destroy(&p);

	return 0;
}
