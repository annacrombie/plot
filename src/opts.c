#include "posix.h"

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "data_pipe.h"
#include "data_proc.h"
#include "display.h"
#include "log.h"
#include "plot.h"
#include "util.h"
#include "version.h"

static void
print_usage(FILE *f)
{
	fprintf(f,
		"plot %s-%s\n"
		"usage: plot [opts]\n"
		"opts\n"
		"  -i <filename>|- - specify a data source\n"
		"  -a <n> - average n inputs per data point\n"
		"  -b [min]:[max] - set fixed plot bounds\n"
		"  -d [height]:[width] - set plot dimensions\n"
		"  -x [every]:[offset]:[mod]:[side]:[color] - set x label format\n"
		"  -y [width]:[prec]:[side] - set y label format\n"
		"  -c <color> - set color of next data source\n"
		"  -f - \"follow\" input\n"
		"  -A - \"animate\" input by following and exit\n"
		"  -S <milliseconds> - follow rate\n"
		"  -m - visually merge overlapping lines, e.g. ╯ and ╰ form ┴\n"
		"  -s %%<charset>|ascii|unicode - set output charset\n"
		"  -h - duh...\n"
		"\n"
		"colors: "
		"\033[4mb\033[0mlack, "
		"\033[4mr\033[0med, "
		"\033[4mg\033[0mreen, "
		"\033[4my\033[0mellow, "
		"b\033[4ml\033[0mue, "
		"\033[4mm\033[0magenta, "
		"\033[4mc\033[0myan, "
		"\033[4mw\033[0mhite"
		"\n"
		"use capital character for bright variant\n"
		"\n"
		"ex.\n"
		"seq 1 99 | shuf | plot -c g\n",
		version.version,
		version.vcs_tag
		);
}

static int
parse_next_double(char **s, double *l)
{
	char *sep;

	if ((sep = strchr(*s, ':'))) {
		if (sep == *s) {
			(*s)++;
			return 0;
		}
		*sep = '\0';
		*l = strtod(*s, NULL);
		*s = sep + 1;
		return 1;
	} else if (**s) {
		*l = strtod(*s, NULL);
		**s = 0;
		return 1;
	} else {
		return 0;
	}
}

static int
parse_next_num(char **s, long *l)
{
	char *sep;

	if ((sep = strchr(*s, ':'))) {
		if (sep == *s) {
			(*s)++;
			return 0;
		}
		*sep = '\0';
		*l = strtol(*s, NULL, 10);
		*s = sep + 1;
		return 1;
	} else if (**s) {
		*l = strtol(*s, NULL, 10);
		**s = 0;
		return 1;
	} else {
		return 0;
	}
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

	if (*s) {
		xl->color = char_to_color(*s);
	}

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
set_fixed_plot_bounds(char *s, struct plot *p)
{
	double d;

	p->flags |= plot_flag_fixed_bounds;

	if (parse_next_double(&s, &d)) {
		p->bounds.min = d;
	}

	if (parse_next_double(&s, &d)) {
		p->bounds.max = d;
	}
}

#define PIPE_SEP "|"
#define ARG_SEP ':'

// Make sure no names are prefixes of other names
static struct {
	const char *name;
	bool takes_arg;
} dproc_info[data_proc_type_count] = {
	[data_proc_avg] = { "avg", true },
	[data_proc_sma] = { "sma", true },
};

static void
add_data_from_file(char *path, struct plot *p, int color)
{
	uint32_t i, namelen, toklen, ctx_size;
	char *tok = strtok(path, PIPE_SEP), *endptr;
	int32_t tmp;
	const char *err;
	void *ctx;

	if (!tok) {
		err = "missing filename";
		goto err;
	} else if (!pipeline_create(tok)) {
		exit(EXIT_FAILURE);
	}

	while ((tok = strtok(NULL, PIPE_SEP))) {
		toklen = strlen(tok);

		ctx = NULL;
		ctx_size = 0;

		for (i = 0; i < data_proc_type_count; ++i) {
			namelen = strlen(dproc_info[i].name);

			if (namelen <= toklen
			    && strncmp(tok, dproc_info[i].name, namelen) == 0) {
				break;
			}
		}

		if (i == data_proc_type_count) {
			err = "unknown proc";
			goto err;
		}

		if (namelen < toklen) {
			if (tok[namelen] != ARG_SEP) {
				err = "bad argument seperator";
				goto err;
			} else if (!tok[namelen + 1]) {
				err = "missing argument";
				goto err;
			} else if (!dproc_info[i].name) {
				err = "proc takes no argument";
				goto err;
			}

			tmp = strtol(&tok[namelen + 1], &endptr, 10);
			if (*endptr) {
				err = "invalid argument";
				goto err;
			}

			ctx = &tmp;
			ctx_size = sizeof(int32_t);
		} else if (dproc_info[i].name) {
			err = "missing argument";
			goto err;
		}

		if (!pipeline_append(i, ctx, ctx_size)) {
			exit(EXIT_FAILURE);
		}
	}

	plot_add(p, color);
	return;
err:
	fprintf(stderr, "error parsing '%s': %s\n", tok, err);
	exit(EXIT_FAILURE);
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
	long tmp;

	while ((opt = getopt(argc, argv, "a:Ab:c:d:fhi:ms:S:x:y:")) != -1) {
		switch (opt) {
		case 'a':
			if ((tmp = strtol(optarg, NULL, 10)) < 0) {
				fprintf(stderr, "invalid average arg\n");
				exit(EXIT_FAILURE);
			}
			p->average = tmp;
			break;
		case 'A':
			p->flags |= plot_flag_animate;
			break;
		case 'b':
			set_fixed_plot_bounds(optarg, p);
			break;
		case 'd':
			set_plot_dimensions(optarg, p);
			break;
		case 'f':
			p->flags |= plot_flag_follow;
			break;
		case 'i':
			add_data_from_file(optarg, p, lc);
			lc = 0;
			break;
		case 'm':
			p->flags |= plot_flag_merge_plot_pieces;
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

