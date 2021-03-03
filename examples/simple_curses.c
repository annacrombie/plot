#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <curses.h>
#include <sys/ioctl.h>
#include <locale.h>
#include <math.h>
#include <plot/plot.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static bool resized = true;

struct sine_wave_ctx {
	double amp, freq;
	double x;
};

static uint32_t
sine_wave(void *_ctx, double *out, uint32_t out_max)
{
	struct sine_wave_ctx *ctx = _ctx;
	double v = sin(ctx->x) * ctx->amp;
	ctx->x += ctx->freq;
	out[0] = v;
	return 1;
}

static void
handle_sigwinch(int _)
{
	resized = true;
}

#define MAX_HEIGHT 256
#define MAX_WIDTH 256

int
main(void)
{

	struct plot *p = plot_alloc(MAX_HEIGHT, MAX_WIDTH, 12);
	struct sine_wave_ctx sine_wave_ctx = { .amp = 5, .freq = 0.2 };

	bool run = true, paused = false;
	int key;

	const struct timespec tick_st = { .tv_nsec = 1000000000 / 30 };
	const size_t buf_size = 99999;
	char buf[buf_size];

	setlocale(LC_ALL, "");

	{ /* setup curses */
		setenv("ESCDELAY", "10", 1);
		initscr();
		cbreak();
		noecho();
		nonl();
		start_color();
		use_default_colors();
		intrflush(stdscr, FALSE);
		keypad(stdscr, TRUE);
		nodelay(stdscr, TRUE);
		wbkgdset(stdscr, ' ');
		curs_set(0);

		struct sigaction sigact;

		memset(&sigact, 0, sizeof(struct sigaction));

		sigact.sa_flags = 0;
		sigact.sa_handler = handle_sigwinch;
		sigaction(SIGWINCH, &sigact, NULL);
	}

	plot_add_dataset(p, 0, NULL, 0, sine_wave, &sine_wave_ctx);

	while (run) {
		if (resized) {
			struct winsize w;

			ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

			resize_term(w.ws_row, w.ws_col);
			wresize(stdscr, w.ws_row, w.ws_col);

			wredrawln(stdscr, 0, w.ws_row);

			if ((p->width = w.ws_col) > MAX_WIDTH) {
				p->width = MAX_WIDTH;
			}

			p->width -= 20;

			if ((p->height = w.ws_row) > MAX_HEIGHT) {
				p->height = MAX_HEIGHT;
			}

			p->height -= 2;

			resized = false;
		}

		werase(stdscr);
		snprintf(buf, buf_size, "amp: %f, freq: %f", sine_wave_ctx.amp, sine_wave_ctx.freq);
		mvwaddstr(stdscr, 0, 0, buf);
		plot_string(p, buf, buf_size);
		mvwaddstr(stdscr, 1, 0, buf);

		while ((key = getch()) != ERR) {
			switch (key) {
			case 'q':
				run = false;
				break;
			case ' ':
				paused = !paused;
				break;
			case '.':
				plot_fetch(p, 1);
				break;
			case KEY_RIGHT:
				sine_wave_ctx.freq += 0.01;
				break;
			case KEY_LEFT:
				if ((sine_wave_ctx.freq -= 0.01) < 0) {
					sine_wave_ctx.freq = 0;
				}
				break;
			case KEY_UP:
				++sine_wave_ctx.amp;
				break;
			case KEY_DOWN:
				--sine_wave_ctx.amp;
				break;
			}
		}

		if (!paused) {
			plot_fetch(p, 1);
		}

		nanosleep(&tick_st, NULL);
	}

	endwin();
	plot_free(p);
}
