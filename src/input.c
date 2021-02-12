#include "posix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "input.h"
#include "plot.h"
#include "util.h"

#define MAX_AHEAD 8

static bool
start_of_number(char c)
{
	switch (c) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	case '-': case '+':
		return true;
	default:
		return false;
	}
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
