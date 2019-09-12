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

static void parse_opts(struct plot *p, int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "hd:")) != -1) {
		switch (opt) {
		case 'd':
			if (!set_plot_dimensions(optarg, p)) {
				fprintf(stderr, "invalid dimension string '%s'", optarg);
				exit(EXIT_FAILURE);
			}
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
	size_t arrlen;
	double *arr;

	struct plot *p = plot_init();

	parse_opts(p, argc, argv);

	arrlen = read_arr(&arr);
	if (arrlen < 1)
		return 1;

	plot_add(p, arrlen, arr);
	plot_plot(p);
	plot_destroy(p);

	free(arr);

	return 0;
}
