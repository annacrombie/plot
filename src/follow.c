#include "posix.h"

#include <signal.h>
#include <stdio.h>
#include <time.h>

#include "follow.h"
#include "input.h"
#include "plot.h"
#include "util.h"

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


void
follow_plot(struct plot *p, long ms)
{
	size_t i;
	int height = p->height;

	if (p->x_label.every && p->x_label.side) {
		height += p->x_label.side == 3 ? 2 : 1;
	}

	struct timespec sleep = {
		.tv_sec = 0,
		.tv_nsec = ms * 100000,
	};

	install_signal_handler();

	printf("\033[?25l");

	while (loop) {
		if (!pdtry_all_buffers(p, 1)) {
			for (i = 0; i < p->datasets; i++) {
				if (feof(p->data[i].src.src)) {
					clearerr(p->data[i].src.src);
				}
			}
		}

		if (plot_plot(p)) {
			printf("\033[%dA", height);
			fflush(stdout);
		}

		nanosleep(&sleep, NULL);
	}

	printf("\033[%dB\033[?12l\033[?25h\n", height);
}
