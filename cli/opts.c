#include "posix.h"

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cli/main.h"
#include "cli/opts.h"

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
		plot_version.version,
		plot_version.vcs_tag
		);
}

enum plot_color
char_to_color(char c)
{
	enum plot_color n;

	switch (c) {
	case 'b': n = plot_color_black;     break;
	case 'r': n = plot_color_red;       break;
	case 'g': n = plot_color_green;     break;
	case 'y': n = plot_color_yellow;    break;
	case 'l': n = plot_color_blue;      break;
	case 'm': n = plot_color_magenta;   break;
	case 'c': n = plot_color_cyan;      break;
	case 'w': n = plot_color_white;     break;
	case 'B': n = plot_color_brblack;   break;
	case 'R': n = plot_color_brred;     break;
	case 'G': n = plot_color_brgreen;   break;
	case 'Y': n = plot_color_bryellow;  break;
	case 'L': n = plot_color_brblue;    break;
	case 'M': n = plot_color_brmagenta; break;
	case 'C': n = plot_color_brcyan;    break;
	case 'W': n = plot_color_brwhite;   break;
	default:
		fprintf(stderr, "invalid color char: %c\n", c);
		n = 0;
		break;
	}

	return n;
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
set_x_label(char *s, struct plot *p)
{
	long l;

	if (parse_next_num(&s, &l)) {
		p->x_label.every = l;
	}

	if (parse_next_num(&s, &l)) {
		p->x_label.start = l;
	}

	if (parse_next_num(&s, &l)) {
		p->x_label.mod = l;
	}

	if (parse_next_num(&s, &l)) {
		p->x_label.side = l % 4;
	}

	if (*s) {
		p->x_label.color = char_to_color(*s);
	}

	return 1;
}

static void
set_y_label(char *s, struct plot *p)
{
	long l;

	if (parse_next_num(&s, &l)) {
		p->y_label.width = l;
	}

	if (parse_next_num(&s, &l)) {
		p->y_label.prec = l;
	}

	if (parse_next_num(&s, &l)) {
		p->y_label.side = l % 4;
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
	double min, max;

	if (parse_next_double(&s, &min)
	    && parse_next_double(&s, &max)) {
		plot_fix_bounds(p, min, max);
	} else {
		fprintf(stderr, "invalid bounds");
		exit(EXIT_FAILURE);
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
parse_pipeline(char *path, struct plot_data *pd)
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

		if (!plot_pipeline_append(pd, i, ctx, ctx_size)) {
			exit(EXIT_FAILURE);
		}
	} while ((tok = strtok(NULL, PIPE_SEP)));

	return;
err:
	fprintf(stderr, "error parsing '%s': %s\n", tok, err);
	exit(EXIT_FAILURE);
}

static struct plot_data *
add_input(char *path, struct plot *p, struct plot_static_memory *mem, enum plot_color c)
{
	char *s;
	uint8_t flags = 0;
	int file_flags = 0;
	struct plot_data *pd;
	FILE *f;

	if ((s = strchr(path, ':'))) {
		*s = 0;
		for (s = s + 1; s && *s; ++s) {
			switch (*s) {
			case 'n':
				file_flags |= O_NONBLOCK;
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


	if (strcmp(path, "-") == 0) {
		f = stdin;
	} else if (!(f = fopen(path, "r"))) {
		fprintf(stderr, "error opening file '%s': %s\n", path,
			strerror(errno));
		return false;
	}

	if (file_flags) {
		int fd, oldflags;

		if ((fd = fileno(f) == -1)) {
			fprintf(stderr, "couldn't get file descriptor for %s: %s\n", path, strerror(errno));
			return false;
		}

		oldflags = fcntl(fd, F_GETFL);
		fcntl(fd, F_SETFL, oldflags | O_NONBLOCK);
	}

	if (!plot_file_input_init(&mem->file_input_ctxs[p->datasets],
		mem->file_input_bufs[p->datasets], FILE_INPUT_BUF, f, flags)) {
		exit(EXIT_FAILURE);
	} else if (!plot_add_dataset(p, c,
		&mem->pipeline_elems[p->datasets * MAX_PIPELINE_ELEMENTS],
		MAX_PIPELINE_ELEMENTS,
		(plot_input_func)plot_file_input_read,
		&mem->file_input_ctxs[p->datasets])) {
		exit(EXIT_FAILURE);
	}

	pd = &p->data[p->datasets - 1];

	memcpy(pd->pipe, mem->default_pipeline,
		sizeof(struct plot_pipeline_elem) * mem->default_dataset.pipeline_len);
	pd->pipeline_len = mem->default_dataset.pipeline_len;

	return pd;
}

static void
set_charset(struct plot *p, char *charset)
{
	size_t len;

	if (charset[0] != '%') {
		if (strcmp(charset, "unicode") == 0) {
			plot_set_charset(p, plot_charset_unicode);
		} else if (strcmp(charset, "ascii") == 0) {
			plot_set_charset(p, plot_charset_ascii);
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
parse_opts(struct opts *opts, struct plot *p, struct plot_static_memory *mem,
	int argc, char **argv)
{
	char opt;
	enum plot_color lc = 0;
	long tmp;
	uint32_t i;
	enum plot_file_input_flags global_flags = 0;

	*opts = (struct opts){
		.mode = mode_normal,
		.follow_rate = 100,
	};

	struct plot_data *pd = &mem->default_dataset;
	plot_dataset_init(pd, 0, mem->default_pipeline, MAX_PIPELINE_ELEMENTS, NULL, NULL);

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

			if (!plot_pipeline_append(pd, data_proc_avg, &tmpu, sizeof(uint32_t))) {
				exit(EXIT_FAILURE);
			}
			break;
		case 'A':
			opts->mode = mode_animate;
			break;
		case 'b':
			set_fixed_plot_bounds(optarg, p);
			break;
		case 'd':
			set_plot_dimensions(optarg, p);
			break;
		case 'f':
			global_flags |= plot_file_input_flag_infinite;
			opts->mode = mode_follow;
			break;
		case 'i':
			pd = add_input(optarg, p, mem, lc);
			lc = 0;
			break;
		case 'm':
			p->flags |= plot_flag_merge_plot_pieces;
			break;
		case 'p':
			parse_pipeline(optarg, pd);
			break;
		case 's':
			set_charset(p, optarg);
			break;
		case 'S':
			opts->follow_rate = strtol(optarg, NULL, 10);
			break;
		case 'x':
			set_x_label(optarg, p);
			break;
		case 'y':
			set_y_label(optarg, p);
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

	if (optind < argc) {
		fprintf(stderr, "warning: ignoring trailing arguments\n");
	}

	if (p->datasets == 0) {
		add_input("-", p, mem, lc);
	}

	for (i = 0; i < p->datasets; ++i) {
		mem->file_input_ctxs[i].flags |= global_flags;
	}
}
