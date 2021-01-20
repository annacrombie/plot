#include "posix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "input.h"
#include "plot.h"
#include "util.h"

#define DEF_BUFFER_SIZE 256
#define TMP_ARR_SIZE 64
#define MAX_AHEAD 8

static size_t buf_size = DEF_BUFFER_SIZE;

void
set_input_buffer_size(size_t new_size)
{
	buf_size = new_size;
}

static int
read_numbers(struct input *in, double *dest, size_t max)
{
	char *endptr = NULL;
	size_t i, read;
	size_t len = 0, lr = 0;

	if (in->rem >= buf_size) {
		buf_size += in->rem;
	}

	read = fread(&in->buf[in->rem], sizeof(char), buf_size - in->rem, in->src);
	read += in->rem;

	for (i = 0; i < read; i++) {
		if (!is_digit(in->buf[i]) || (endptr != NULL && &in->buf[i] < endptr)) {
			continue;
		}

		lr = i;
		dest[len] = strtod(&in->buf[i], &endptr);
		len++;

		if (len == max) {
			break;
		}
	}

	if (!feof(in->src) && endptr >= &in->buf[i - 1]) {
		in->rem = read - lr;
		memmove(in->buf, &in->buf[lr], sizeof(char) * in->rem);
		len--;
	} else {
		in->rem = 0;
	}

	return len;
}

static void
shift_arr(double *arr, size_t off, size_t amnt)
{
	memmove(arr, arr + off, sizeof(double) * (amnt));
}

void
pd_avg(struct plot_data *pd, double *read_arr, double *arr, size_t *read_len, uint32_t avg_by)
{
	size_t i,len = 0;
	if (avg_by == 1) {
		memcpy(arr, read_arr, sizeof(double) * (*read_len));
	} else {
		for (i = 0; i < (*read_len); ++i) {
			pd->avg.sum += read_arr[i];

			if (++pd->avg.count >= avg_by) {
				arr[len++] = pd->avg.sum / pd->avg.count;

				pd->avg.sum = pd->avg.count = 0;
			}
		}
	}
}

static int
pdtry_buffer_noshift(struct plot_data *pd, size_t max_w, uint32_t avg_by)
{
	size_t len = 0;
	double read_arr[TMP_ARR_SIZE];
	double arr[TMP_ARR_SIZE];

	if (pd->len >= max_w) {
		return 0;
	} else if ((len = read_numbers(&pd->src, read_arr, TMP_ARR_SIZE)) == 0) {
		return 0;
	}

	pd_avg(pd, read_arr, arr, &len, avg_by);

	if (len >= max_w) {
		memcpy(pd->data, arr, max_w * sizeof(double));
		pd->len = max_w;
		return 1;
	} else if(len + pd->len > max_w) {
		len = max_w - pd->len;
	}

	memcpy(&pd->data[pd->len], arr, sizeof(double) * len);
	pd->len += len;
	return 1;
}

static int
pdtry_buffer_shift(struct plot_data *pd, size_t max_w, uint32_t *shifted, uint32_t avg_by)
{
	size_t len = 0;
	double read_arr[TMP_ARR_SIZE];
	double arr[TMP_ARR_SIZE];

	if ((len = read_numbers(&pd->src, read_arr, TMP_ARR_SIZE)) == 0) {
		return 0;
	}

	pd_avg(pd, read_arr, arr, &len, avg_by);

	if (len >= max_w) {
		memcpy(pd->data, &arr[len - max_w], max_w * sizeof(double));
		pd->len = max_w;
		return 1;
	} else if (len + pd->len > max_w) {
		*shifted = max_w - pd->len + len;
		if (pd->len < *shifted) {
			pd->len = 0;
		} else {
			pd->len -= *shifted;
			shift_arr(pd->data, *shifted, pd->len);
		}
	}

	memcpy(&pd->data[pd->len], arr, sizeof(double) * len);
	pd->len += len;
	return 1;
}

int
pdtry_all_buffers(struct plot *p)
{
	size_t i;
	uint32_t read = 0,
		 shifts[MAX_DATA] = { 0 },
		 maxshift = 0,
		 min_len = p->width;

	for (i = 0; i < p->datasets; i++) {
		if (p->data[i].len < min_len) {
			min_len = p->data[i].len;
		}
	}

	for (i = 0; i < p->datasets; i++) {
		if (!(p->flags & plot_flag_animate) && p->data[i].len - min_len > MAX_AHEAD) {
			continue;
		}

		read |= pdtry_buffer_shift(&p->data[i], p->width, &shifts[i], p->average);

		if (shifts[i] > maxshift) {
			maxshift = shifts[i];
		}
	}

	if (maxshift) {
		for (i = 0; i < p->datasets; i++) {
			if ((shifts[i] = maxshift - shifts[i])) {
				if ((p->data[i].len > shifts[i])) {
					p->data[i].len -= shifts[i];
					shift_arr(p->data[i].data, shifts[i], p->data[i].len);
				} else {
					p->data[i].len = 0;
				}
			}
		}

		p->x_label.start += maxshift;
	}

	return read;
}

 void
 pdread_all_available(struct plot *p)
 {
	uint32_t read;
	size_t i;
	do{
		for (i = 0, read = 0; i < p->datasets; i++) {
			read |= pdtry_buffer_noshift(&p->data[i], p->width, p->average);
		}
	} while(read);
}
