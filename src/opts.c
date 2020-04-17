#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include "plot.h"
#include "display.h"
#include "util.h"

static void
print_usage(FILE *f)
{
	fprintf(f,
		"plot " PLOT_VERSION "\n"
		"usage: plot [opts]\n"
		"opts\n"
		"  -i <filename>|- - specify a data source\n"
		"  -d [width]:[height] - set plot dimensions\n"
		"  -x [every]:[offset]:[mod]:[side]:[color] - set x label format\n"
		"  -y [width]:[prec]:[side] - set y label format\n"
		"  -c <color> - set color of next data source\n"
		"  -f - \"follow\" input\n"
		"  -S [milliseconds] - follow rate\n"
		"  -m - visually merge overlapping lines, e.g. ╯ and ╰ form ┴\n"
		"  -s %%<charset>|ascii|unicode - set output charset\n"
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

static int
parse_next_num(char **buf, long *l)
{
	char *p;
	int n = 1;

	*l = 0;

	if (is_digit(**buf)) {
		*l = strtol(*buf, &p, 10);
		*buf = p;
		if (**buf == ':') {
			(*buf)++;
		}else if (**buf != '\0') {
			n = 0;
		}
	} else if (**buf == ':') {
		(*buf)++;
		n = 0;
	} else if (**buf != '\0') {
		n = 0;
	}

	return n;
}

static int
set_x_label(char *s, struct x_label *xl)
{
	long l;

	if (parse_next_num(&s, &l)) {
		xl->every = l;
	}

	if (parse_next_num(&s, &l)) {
		xl->start = l;
	}

	if (parse_next_num(&s, &l)) {
		xl->mod = l;
	}

	if (parse_next_num(&s, &l)) {
		xl->side = l % 4;
	}

	xl->color = char_to_color(*s);

	return 1;
}

static void
set_y_label(char *s, struct y_label *yl)
{
	long l;

	if (parse_next_num(&s, &l)) {
		yl->width = l;
	}

	if (parse_next_num(&s, &l)) {
		yl->prec = l;
	}

	if (parse_next_num(&s, &l)) {
		yl->side = l % 4;
	}
}

/* parse a string like 34:54 with either side of the ':' optional.  If the
 * string is valid, return 1, otherwise return 0.
 */
static void
set_plot_dimensions(char *s, struct plot *p)
{
	long l;

	if (parse_next_num(&s, &l) && (l < MAX_HEIGHT && l > 0)) {
		p->height = l;
	}

	if (parse_next_num(&s, &l) && (l < MAX_WIDTH && l > 0)) {
		p->width = l;
	}
}

static void
add_data_from_file(char *filename, struct plot *p, int color)
{
	FILE *f = (strcmp(filename, "-") == 0) ? stdin : fopen(optarg, "r");

	if (f == NULL) {
		fprintf(stderr, "no such file\n");
		exit(EXIT_FAILURE);
	}

	plot_add(p, f, color);
}

static enum plot_charset
set_charset(char *charset)
{
	size_t len;

	if (charset[0] != '%') {
		if (strcmp(charset, "unicode") == 0) {
			return PCUNICODE;
		} else if (strcmp(charset, "ascii") == 0) {
			return PCASCII;
		} else {
			fprintf(stderr, "invalid charset '%s'\n", charset);
			exit(EXIT_FAILURE);
		}
	} else {
		len = strlen(charset) - 1;

		set_custom_plot_charset(&charset[1], len);
		return PCCUSTOM;
	}
}

int
parse_opts(struct plot *p, int argc, char **argv)
{
	char opt;
	int lc = 0;

	while ((opt = getopt(argc, argv, "a:c:d:fhi:ms:S:x:y:")) != -1) {
		switch (opt) {
		case 'a':
			if ((p->average = strtol(optarg, NULL, 10)) < 0) {
				fprintf(stderr, "invalid average arg\n");
				exit(EXIT_FAILURE);
			}
			break;
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
		case 's':
			p->charset = set_charset(optarg);
			break;
		case 'S':
			p->follow_rate = strtol(optarg, NULL, 10);
			break;
		case 'x':
			set_x_label(optarg, &p->x_label);
			break;
		case 'y':
			set_y_label(optarg, &p->y_label);
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

