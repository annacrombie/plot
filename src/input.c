#include <stdio.h>
#include <string.h>
#include "util.h"
#include "plot.h"

#define BUFFER_SIZE 64

static char *bufp = NULL;
static long leftovers = 0;
static double *tmp_arr = NULL;

void input_init(void)
{
	bufp = safe_calloc(BUFFER_SIZE, sizeof(char));
	tmp_arr = safe_calloc(BUFFER_SIZE, sizeof(double));
	leftovers = 0;
}

void input_cleanup(void)
{
	free(bufp);
	free(tmp_arr);
}

static int read_numbers(FILE *f, double **dest)
{
	char *endptr = NULL;
	size_t i, read;
	long len = 0, lr;

	read = fread(&bufp[leftovers], sizeof(char), BUFFER_SIZE - leftovers, f);
	read += leftovers;

	for (i = 0; i < read; i++) {
		if (!is_digit(bufp[i]) || (endptr != NULL && &bufp[i] < endptr))
			continue;

		lr = i;
		tmp_arr[len] = strtod(&bufp[i], &endptr);
		(len)++;
	}

	if (!feof(f) && endptr >= &bufp[read - 1]) {
		leftovers = read - lr;
		bufp = memmove(bufp, &bufp[lr], sizeof(char) * leftovers);
		len--;
	} else {
		leftovers = 0;
	}

	*dest = tmp_arr;
	return len;
}

static void *shift_arr(double *arr, size_t off, size_t amnt)
{
	return memmove(arr, arr + off, sizeof(double) * (amnt));
}

int pdtry_buffer(struct plot_data *pd, size_t max_w, int shift)
{
	size_t len;
	double *arr;

	if (!shift && pd->len >= max_w)
		return 0;

	if ((len = read_numbers(pd->src, &arr)) == 0)
		return 0;

	if (len >= max_w) {
		if (shift)
			memcpy(pd->data, &arr[len - max_w], max_w * sizeof(double));
		else
			memcpy(pd->data, arr, max_w * sizeof(double));

		pd->len = max_w;
		return 1;
	}

	if (len + pd->len > max_w) {
		if (shift) {
			shift = max_w - pd->len + len;
			pd->data = shift_arr(pd->data, shift, pd->len - shift);
			pd->len = pd->len - shift;

			shift = 1;
		} else {
			len = max_w - pd->len;
		}
	}

	memcpy(&pd->data[pd->len], arr, len * sizeof(double));
	pd->len += len;
	return 1;
}

int pdtry_all_buffers(struct plot *p, int shift)
{
	size_t i;
	int ret = 0;

	for (i = 0; i < p->datasets; i++)
		ret |= pdtry_buffer(p->data[i], p->width, shift);

	return ret;
}

void pdread_all_available(struct plot *p)
{
	while (pdtry_all_buffers(p, 0));
}
