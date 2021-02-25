#include "posix.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "data_pipe.h"
#include "data_proc.h"
#include "input.h"
#include "log.h"
#include "plot.h"
#include "util.h"
#include "version.h"

struct plot_file_input file_input_ctxs[MAX_DATA] = { 0 };

static void
print_usage(FILE *f)
{
	fprintf(f,
		"plot %s-%s\n"
		"usage: plot [opts]\n"
		"opts\n"
		"  -i <filename>|- - specify a data source\n"
		"  -b [min]:[max] - set fixed plot bounds\n"
		"  -d [height]:[width] - set plot dimensions\n"
		"  -x [every]:[offset]:[mod]:[side]:[color] - set x label format\n"
		"  -y [width]:[prec]:[side] - set y label format\n"
		"  -c <color> - set color of next data source\n"
		"  -p <pipeline> - specify a data processing pipeline\n"
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
enum arg_type {
	arg_type_none,
	arg_type_int,
	arg_type_float,
};

static struct {
	const char *name;
	enum arg_type arg;
} dproc_info[data_proc_type_count] = {
	[data_proc_avg] = { "avg", arg_type_int },
	[data_proc_sma] = { "sma", arg_type_int },
	[data_proc_cma] = { "cma", },
	[data_proc_roc] = { "roc", arg_type_float },
};

static void
parse_pipeline(char *path, struct plot *p)
{
	uint32_t i, namelen, toklen, ctx_size;
	char *tok = strtok(path, PIPE_SEP), *endptr;
	const char *err;
	void *ctx;
	int32_t tmp_int;
	float tmp_float;

	do {
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
			}

			switch (dproc_info[i].arg) {
			case arg_type_none:
				err = "proc takes no argument";
				goto err;
				break;
			case arg_type_int:
				tmp_int = strtol(&tok[namelen + 1], &endptr, 10);
				if (*endptr) {
					err = "unable to parse integer argument";
					goto err;
				}

				ctx = &tmp_int;
				ctx_size = sizeof(int32_t);
				break;
			case arg_type_float:
				tmp_float = strtod(&tok[namelen + 1], &endptr);
				if (*endptr) {
					err = "unable to parse float argument";
					goto err;
				}

				ctx = &tmp_float;
				ctx_size = sizeof(float);
				break;
			}
		} else if (dproc_info[i].arg) {
			err = "missing argument";
			goto err;
		}

		if (!pipeline_append(i, ctx, ctx_size)) {
			exit(EXIT_FAILURE);
		}
	} while ((tok = strtok(NULL, PIPE_SEP)));

	return;
err:
	fprintf(stderr, "error parsing '%s': %s\n", tok, err);
	exit(EXIT_FAILURE);
}

static void
add_input(char *path, struct plot *p, enum color c)
{
	char *s;
	uint8_t flags = 0;

	if ((s = strchr(path, ':'))) {
		*s = 0;
		for (s = s + 1; s && *s; ++s) {
			switch (*s) {
			case 'n':
				flags |= plot_file_input_flag_nonblock;
				break;
			case 'r':
				flags |= plot_file_input_flag_rewind;
				break;
			default:
				fprintf(stderr, "invalid pipeline flag: '%c'\n", *s);
				exit(EXIT_FAILURE);
			}
		}
	}

	if (!plot_file_input_init(&file_input_ctxs[p->datasets], path, flags)) {
		exit(EXIT_FAILURE);
	} else if (!pipeline_create((pipeline_input_func)plot_file_input_read,
		&file_input_ctxs[p->datasets])) {
		exit(EXIT_FAILURE);
	}

	plot_add(p, c);
}

static void
set_charset(struct plot *p, char *charset)
{
	size_t len;

	if (charset[0] != '%') {
		if (strcmp(charset, "unicode") == 0) {
			plot_set_charset(p, PCUNICODE);
		} else if (strcmp(charset, "ascii") == 0) {
			plot_set_charset(p, PCASCII);
		} else {
			fprintf(stderr, "invalid charset '%s'\n", charset);
			exit(EXIT_FAILURE);
		}
	} else {
		len = strlen(charset) - 1;

		plot_set_custom_charset(p, &charset[1], len);
	}
}

void
parse_opts(struct plot *p, int argc, char **argv)
{
	char opt;
	int lc = 0;
	long tmp;
	uint32_t i;
	enum plot_file_input_flags global_flags = 0;

	while ((opt = getopt(argc, argv, "a:Ab:c:d:fhi:mp:s:S:x:y:")) != -1) {
		switch (opt) {
		case 'a':
			if ((tmp = strtol(optarg, NULL, 10)) < 0) {
				fprintf(stderr, "invalid average arg\n");
				exit(EXIT_FAILURE);
			}

			uint32_t tmpu = tmp;

			fprintf(stderr, "warning: -a %d is deprecated, "
				"please use `-p avg:%d` instead\n", tmpu, tmpu);

			if (!pipeline_append(data_proc_avg, &tmpu, sizeof(uint32_t))) {
				exit(EXIT_FAILURE);
			}
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
			global_flags |= plot_file_input_flag_infinite;
			p->flags |= plot_flag_follow;
			break;
		case 'i':
			add_input(optarg, p, lc);
			lc = 0;
			break;
		case 'm':
			p->flags |= plot_flag_merge_plot_pieces;
			break;
		case 'p':
			parse_pipeline(optarg, p);
			break;
		case 's':
			set_charset(p, optarg);
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

	if (p->datasets == 0) {
		add_input("-", p, lc);
	}

	for (i = 0; i < p->datasets; ++i) {
		file_input_ctxs[i].flags |= global_flags;
	}
}

