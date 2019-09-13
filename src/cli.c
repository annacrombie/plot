#include "plot.h"
#include "read_arr.h"
#include <getopt.h>

#define MAXWIDTH 1000
#define MAXHEIGHT 1000

static void print_usage(FILE *f)
{
	fprintf(f,
		"plot " PLOT_VERSION "\n"
		"usage: plot [opts]\n"
		"opts\n"
		"  -d [width]:[height] - set plot dimensions\n"
		"  -i [filename|-] - specify a data source\n"
		"  -x [every]:[offset]:[mod]"
		"  -h - duh...\n"
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

	return 1;
}

/* parse a string like 34:54 with either side of the ':' optional.  If the
 * string is valid, return 1, otherwise return 0.
 */
static int set_plot_dimensions(char *s, struct plot *p)
{
	long l;

	if (parse_next_num(&s, &l))
		p->height = l;

	if (parse_next_num(&s, &l))
		p->width = l;

	if (p->height > MAXHEIGHT || p->width > MAXWIDTH)
		return 0;

	return 1;
}

static void add_data_from_file(FILE *f, struct plot *p)
{
	size_t len;
	double *arr;

	if (f == NULL) {
		fprintf(stderr, "no such file\n");
		exit(EXIT_FAILURE);
	}

	len = read_arr(f, &arr, p->width);
	if (len >= 1)
		plot_add(p, len, arr);
}

static void parse_opts(struct plot *p, int argc, char **argv)
{
	char opt;
	FILE *f;

	while ((opt = getopt(argc, argv, "d:hi:")) != -1) {
		switch (opt) {
		case 'd':
			if (!set_plot_dimensions(optarg, p)) {
				fprintf(stderr, "invalid dimensions '%s'", optarg);
				exit(EXIT_FAILURE);
			}
			break;
		case 'i':
			f = (strcmp(optarg, "-") == 0) ? stdin : fopen(optarg, "r");
			add_data_from_file(f, p);
			break;
		case 'h':
			print_usage(stdout);
			exit(EXIT_SUCCESS);
		default:
			print_usage(stderr);
			exit(EXIT_FAILURE);
		}
	}
}

int main(int argc, char **argv)
{
	struct plot *p = plot_init();

	parse_opts(p, argc, argv);

	if (p->datasets == 0)
		add_data_from_file(stdin, p);

	plot_plot(p);
	plot_destroy(p, 1);

	return 0;
}
