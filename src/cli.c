#define _POSIX_C_SOURCE 199309L

#include <time.h>
#include <getopt.h>
#include <signal.h>
#include <poll.h>
#include <string.h>
#include "plot.h"
#include "util.h"

#define MAXWIDTH 1000
#define MAXHEIGHT 1000

static void print_usage(FILE *f)
{
	fprintf(f,
		"plot " PLOT_VERSION "\n"
		"usage: plot [opts]\n"
		"opts\n"
		"  -i [filename|-] - specify a data source\n"
		"  -d [width]:[height] - set plot dimensions\n"
		"  -x [every]:[offset]:[mod]:[side]:[color] - set x label format\n"
		"  -y [width]:[prec]:[side] - set y label format\n"
		"  -c <color> - set color of next data source\n"
		"  -f - \"follow\" input, only works with stdin\n"
		"  -m - visually merge overlapping lines, e.g. ╯ and ╰ form ┴\n"
		"  -h - duh...\n"
		"\n"
		"colors: "
		"\e[4mb\e[0mlack, "
		"\e[4mr\e[0med, "
		"\e[4mg\e[0mreen, "
		"\e[4my\e[0mellow, "
		"b\e[4ml\e[0mue, "
		"\e[4mm\e[0magenta, "
		"\e[4mc\e[0myan, "
		"\e[4mw\e[0mhite"
		"\n"
		"use capital character for bright variant\n"
		"\n"
		"ex.\n"
		"seq 1 99 | shuf | plot -c g\n"
		);
}

static int parse_next_num(char **buf, long *l)
{
	char *p;
	int n = 1;

	*l = 0;

	if (is_digit(**buf)) {
		*l = strtol(*buf, &p, 10);
		*buf = p;
		if (**buf == ':')
			(*buf)++;
		else if (**buf != '\0')
			n = 0;
	} else if (**buf == ':') {
		(*buf)++;
		n = 0;
	} else if (**buf != '\0') {
		n = 0;
	}

	return n;
}

static int set_x_label(char *s, struct x_label *xl)
{
	long l;

	if (parse_next_num(&s, &l))
		xl->every = l;

	if (parse_next_num(&s, &l))
		xl->start = l;

	if (parse_next_num(&s, &l))
		xl->mod = l;

	if (parse_next_num(&s, &l))
		xl->side = l % 4;

	xl->color = char_to_color(*s);

	return 1;
}

static void set_y_label(char *s, struct y_label *yl)
{
	long l;

	if (parse_next_num(&s, &l))
		yl->width = l;

	if (parse_next_num(&s, &l))
		yl->prec = l;

	if (parse_next_num(&s, &l))
		yl->side = l % 4;
}

/* parse a string like 34:54 with either side of the ':' optional.  If the
 * string is valid, return 1, otherwise return 0.
 */
static void set_plot_dimensions(char *s, struct plot *p)
{
	long l;

	if (parse_next_num(&s, &l) && (l < MAXHEIGHT && l > 0))
		p->height = l;

	if (parse_next_num(&s, &l) && (l < MAXWIDTH && l > 0))
		p->width = l;
}

static void add_data_from_file(char *filename, struct plot *p, int color)
{
	FILE *f = (strcmp(filename, "-") == 0) ? stdin : fopen(optarg, "r");

	if (f == NULL) {
		fprintf(stderr, "no such file\n");
		exit(EXIT_FAILURE);
	}

	plot_add(p, f, color);
	/*
	   len = read_arr(f, &arr, p->width);
	   if (len >= 1)
	        plot_add(p, len, arr, color);
	 */
}

static int parse_opts(struct plot *p, int argc, char **argv)
{
	char opt;
	int lc = 0;

	while ((opt = getopt(argc, argv, "c:d:fhi:mx:y:")) != -1) {
		switch (opt) {
		case 'd':
			set_plot_dimensions(optarg, p);
			break;
		case 'f':
			p->follow = 1;
			break;
		case 'i':
			add_data_from_file(optarg, p, lc);
			lc = 0;
			break;
		case 'm':
			p->merge_plot_peices = 1;
			break;
		case 'x':
			set_x_label(optarg, p->x_label);
			break;
		case 'y':
			set_y_label(optarg, p->y_label);
			break;
		case 'c':
			lc = char_to_color(*optarg);
			break;
		case 'h':
			print_usage(stdout);
			exit(EXIT_SUCCESS);
		default:
			print_usage(stderr);
			exit(EXIT_FAILURE);
		}
	}

	return lc;
}

static void follow_plot(struct plot *p)
{
	size_t i;
	long height = p->height + (p->x_label->every > 0 ? 1 : 0);
	struct pollfd *pfds = safe_calloc(p->datasets, sizeof(struct pollfd));
	nfds_t nfds = p->datasets;

	struct timespec sleep = {
		.tv_sec = 0,
		.tv_nsec = 500,
	};

	for (i = 0; i < p->datasets; i++) {
		pfds[i].fd = fileno(p->data[i]->src);
		pfds[i].events = POLLIN;
	}

	printf("%c[?25l", 27);

	while (1) {
		if (!plot_read_num(p, 1)) {
			if (poll(pfds, nfds, -1) < 1)
				return;

			for (i = 0; i < p->datasets; i++)
				if (pfds[i].revents & POLLIN && feof(p->data[i]->src))
					clearerr(p->data[i]->src);

			if (!plot_read_num(p, 1)) {
				nanosleep(&sleep, NULL);
				continue;
			}
		}

		plot_plot(p);
		printf("%c[%ldA", 27, height);
	}

	printf("%c[%ldB%c[?12l%c[?25h", 27, height, 27, 27);
}

static void handle_sigint(int _)
{
	printf("%c[?12l%c[?25h", 27, 27);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	struct sigaction sigact;
	struct plot *p = plot_init();

	int lc = parse_opts(p, argc, argv);

	if (p->datasets == 0)
		add_data_from_file("-", p, lc);

	plot_prepare(p);

	if (p->follow) {
		sigact.sa_flags = 0;
		sigact.sa_handler = handle_sigint;
		sigaction(SIGINT, &sigact, NULL);
		follow_plot(p);
	} else {
		while (plot_read_num(p, 0));

		plot_plot(p);
	}

	plot_destroy(p, 1);

	return 0;
}
