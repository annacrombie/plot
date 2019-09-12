#include "plot.h"
#include "read_arr.h"
#include <getopt.h>

static void print_usage(FILE *f)
{
	fprintf(f,
		"plot " PLOT_VERSION "\n"
		"usage: plot [opts]\n"
		"opts\n"
		"  -d [width]:[height] - set plot dimensions\n"
		"  -i [filename|-] - specify a data source\n"
		"  -h - duh...\n"
		);
}

/* parse a string like 34:54 with either side of the ':' optional.  If the
 * string is valid, return 1, otherwise return 0.
 */
static int set_plot_dimensions(char *dims, struct plot *p)
{
	char *end, *sep;
	size_t len = strlen(dims);

	if (strspn(dims, "0123456789:") < len)
		return 0;
	if (len == 1)
		return 1;

	sep = strchr(dims, ':');

	if (sep != strrchr(dims, ':'))
		return 0;

	if (sep == dims) {
		p->height = strtol(&dims[1], NULL, 10);
	} else {
		p->width = strtol(dims, &end, 10);

		if (end + 1 <= &dims[len - 1])
			p->height = strtol(&end[1], NULL, 10);
	}

	return 1;
}

static void add_data_from_file(FILE *f, struct plot *p)
{
	size_t len;
	double *arr;

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
				fprintf(stderr, "invalid dimension string '%s'", optarg);
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
