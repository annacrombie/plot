#include "read_arr.h"

#define BUFFER_SIZE 64
#define ARR_GROW 8

static char *bufp = NULL;
static long leftovers = 0;

int read_numbers(FILE *f, double **dest)
{
	char *endptr = NULL;
	size_t i, read;
	long len, cap, lr;
	double *arr;

	if (bufp == NULL)
		bufp = safe_calloc(BUFFER_SIZE, sizeof(char));

	arr = safe_calloc(ARR_GROW, sizeof(double));
	cap = ARR_GROW;
	len = 0;

	read = fread(&bufp[leftovers], sizeof(char), BUFFER_SIZE - leftovers, f);
	read += leftovers;

	for (i = 0; i < read; i++) {
		if (!is_digit(bufp[i]) || (endptr != NULL && &bufp[i] < endptr))
			continue;

		lr = i;
		arr[len] = strtod(&bufp[i], &endptr);
		(len)++;

		if (len >= cap) {
			cap += ARR_GROW;
			arr = safe_realloc(arr, sizeof(double) * cap);
		}
	}

	if (!feof(f) && endptr >= &bufp[read - 1]) {
		leftovers = read - lr;
		bufp = memmove(bufp, &bufp[lr], sizeof(char) * leftovers);
		len--;
	} else {
		leftovers = 0;
	}

	*dest = arr;
	return len;
}
