#include <ctype.h>
#include "plot.h"

#define ARR_GROW_INC 5

size_t read_arr(double **arr)
{
	double *narr, *tarr;
	char buf[20] = "";
	char rd[1] = "";
	size_t t, bon, arr_len, arr_cap;

	t = bon = arr_len = arr_cap = 0;

	for (; bon < 20; bon++) buf[bon] = '\0';
	bon = 0;

	*arr = calloc(ARR_GROW_INC, sizeof(double));
	if (NULL == *arr) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}
	arr_cap += ARR_GROW_INC;

	while (1) {
		t = fread(rd, 1, sizeof(char), stdin);
		if (t == 0)
			return arr_len;

		if (!isspace(*rd)) {
			buf[bon] = *rd;
			bon++;
		} else {
			(*arr)[arr_len] = strtod(buf, NULL);
			arr_len += 1;

			while (bon > 0) {
				buf[bon] = '\0';
				bon--;
			}

			if (arr_len >= arr_cap) {
				narr = calloc(arr_cap + ARR_GROW_INC, sizeof(double));
				memcpy(narr, *arr, sizeof(double) * arr_cap);
				tarr = *arr;
				*arr = narr;
				free(tarr);
				arr_cap += ARR_GROW_INC;
			}
		}
	}
}

void print_help()
{
	printf(
		"usage: plot [OPTS] -|point[ point [ ...]]\n"
		"\n"
		"OPTS\n"
		"  -H HEIGHT - set total plot height to HEIGHT\n"
		"     (default:  16)\n"
		"  -f STRING - set y-axis label format string to STRING\n"
		"     (default:  \"%%11.2f %%s\")\n"
		"  -h - duh...\n"
		);
}

int main(int argc, char **argv)
{
	int i = 1;
	size_t arrlen;
	double *arr;

	struct plot *p = plot_init();
	/* Parse options */
	char *opts[] = { "-h", "-H", "-f", "-v", "-c" };

	for (; i < argc; i++) {
		if (strcmp(argv[i], opts[0]) == 0) {
			print_help();
			free(p);
			return 0;
		} else if (strcmp(argv[i], opts[1]) == 0) {
			if (argc > i + 1) {
				p->height = atoi(argv[i + 1]);
				if (p->height < 1) {
					fprintf(stderr, "error: height must be >= 1\n");
					free(p);
					return 1;
				}
				i++;
			} else {
				fprintf(stderr, "error: %s requires a value\n", opts[1]);
				free(p);
				return 1;
			}
		} else if (strcmp(argv[i], opts[3]) == 0) {
			printf("plot v%s\n", PLOT_VERSION);
			free(p);
			return 0;
		} else {
			break;
		}
	}

	/* Get the array from the rest of the options */
	arrlen = read_arr(&arr);
	if (arrlen < 1)
		return 0;

	plot_add(p, arrlen, arr);
	plot_plot(p);
	plot_destroy(p);

	free(arr);

	return 0;
}
