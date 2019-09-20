#define _POSIX_C_SOURCE 199309L

#include <signal.h>
#include "plot.h"
#include "follow.h"
#include "input.h"
#include "opts.h"

static void handle_sigint(int _)
{
	printf("%c[?12l%c[?25h", 27, 27);
	exit(EXIT_FAILURE);
}

static void install_signal_handler(void)
{
	struct sigaction *sigact;

	sigact = malloc(sizeof(struct sigaction));

	sigact->sa_flags = 0;
	sigact->sa_handler = handle_sigint;
	sigaction(SIGINT, sigact, NULL);
}


int main(int argc, char **argv)
{
	struct plot *p = plot_init();

	int lc = parse_opts(p, argc, argv);

	if (p->datasets == 0)
		plot_add(p, stdin, lc);

	plot_prepare(p);
	input_init();

	if (p->follow) {
		install_signal_handler();
		set_input_buffer_size(5);
		follow_plot(p);
	} else {
		pdread_all_available(p);
		plot_plot(p);
	}

	input_cleanup();
	plot_destroy(p, 1);

	return 0;
}
