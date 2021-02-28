#include "posix.h"

#include <plot/plot.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "cli/animate.h"

static struct sigaction sigact = { 0 };
static int loop = 1;

static void
handle_sigint(int _)
{
	loop = 0;
}

static void
install_signal_handler(void)
{
	sigact.sa_flags = 0;
	sigact.sa_handler = handle_sigint;
	sigaction(SIGINT, &sigact, NULL);
}

enum esc_seq {
	esc_curs_hide,
	esc_curs_show,
	esc_curs_up,
	esc_curs_down,
};

static const char *esc_seq[] = {
	[esc_curs_hide] = "\033[?25l",
	[esc_curs_show] = "\033[?12l\033[?25h",
	[esc_curs_up]   = "\033[%dA",
	[esc_curs_down] = "\033[%dB",
};

void
do_esc(enum esc_seq es, ...)
{
	va_list ap;
	va_start(ap, es);
	vprintf(esc_seq[es], ap);
	va_end(ap);
}

void
animate_plot(struct plot *p, char *buf, uint32_t bufsize, long ms,
	fetch_new_data fetch)
{
	int height = p->height;

	if (p->x_label.every && p->x_label.side) {
		height += p->x_label.side == plot_label_side_both ? 2 : 1;
	}

	struct timespec sleep = {
		.tv_sec = 0,
		.tv_nsec = ms * 100000,
	};

	install_signal_handler();

	do_esc(esc_curs_hide);

	while (loop) {
		if (!fetch(p)) {
			loop = 0;
		}

		plot_string(p, buf, bufsize);
		fputs(buf, stdout);
		do_esc(esc_curs_up, height);
		fflush(stdout);

		nanosleep(&sleep, NULL);
	}

	do_esc(esc_curs_down, height);
	do_esc(esc_curs_show);
}
