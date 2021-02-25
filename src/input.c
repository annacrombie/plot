#include "posix.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "data_proc.h"
#include "input.h"
#include "log.h"
#include "plot.h"
#include "util.h"

bool
plot_file_input_init(struct plot_file_input *in, const char *path,
	enum plot_file_input_flags flags)
{
	int fd, oldflags;

	/* struct input *in = &pipelines[pipelines_len].in; */

	if (strcmp(path, "-") == 0) {
		in->src = stdin;
	} else if (!(in->src = fopen(path, "r"))) {
		fprintf(stderr, "error opening file '%s': %s\n", path,
			strerror(errno));
		return false;
	}

	if (flags & plot_file_input_flag_nonblock) {
		if ((fd = fileno(in->src) == -1)) {
			fprintf(stderr, "couldn't get file descriptor for '%s': %s\n",
				path, strerror(errno));
			return false;
		}

		oldflags = fcntl(fd, F_GETFL);
		fcntl(fd, F_SETFL, oldflags | O_NONBLOCK);
	}

	return true;
}


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

bool
plot_file_input_read(struct plot_file_input *in, struct dbuf *out)
{
	char *endptr = NULL;
	size_t i = 0, buflen, oldi = 0;
	double tmp;

	if (feof(in->src)) {
		if (in->flags & plot_file_input_flag_infinite) {
			clearerr(in->src);
			if (in->flags & plot_file_input_flag_rewind) {
				rewind(in->src);
			}
		}

		return false;
	}

	if (in->rem >= MAX_INBUF) {
		// grow the buffer if the number of digits remaining is bigger
		// than the buffer This should only happen with really tiny
		// values of buf_size, so maybe when the animation related
		// functions are updated, this can be removed
		/* buf_size += in->rem; */
		assert(false);
	}

	if (!(buflen = fread(&in->buf[in->rem], 1, MAX_INBUF - in->rem, in->src))) {
		if (errno == EAGAIN || !errno) {
			return false;
		} else {
			fprintf(stderr, "error reading from file %d: %s\n", errno, strerror(errno));
			return false;
		}
	}

	buflen += in->rem;

	while (i < buflen) {
		if (!start_of_number(in->buf[i])) {
			++i;
			continue;
		}

		errno = 0;
		tmp = strtod(&in->buf[i], &endptr);
		if (endptr == &in->buf[i] || errno) {
			++i;
			continue;
		}

		oldi = i;

		/* L("i :%d, pos: %ld, %f", out->len, i, tmp); */

		out->dat[out->len] = tmp;
		if (++out->len >= DATA_LEN) {
			break;
		}

		i += endptr - &in->buf[i];
	}

	if (i == buflen && !feof(in->src)) {
		// The last number we read ended right at the end of the
		// buffer, but there is still more to read.  unread this number
		// for now in case only a portion of it is in the buffer
		--out->len;
		i = oldi;
	}

	if ((in->rem = buflen - i)) {
		memmove(in->buf, &in->buf[i], in->rem);
	}

	memset(&in->buf[in->rem], 0, MAX_INBUF - in->rem);

	return true;
}
