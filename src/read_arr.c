#include "read_arr.h"

#define MAX_NUM_LEN 20

size_t read_arr(FILE *f, double **arr, size_t maxlen)
{
	size_t len;
	long num;

	len = 0;
	*arr = safe_calloc(maxlen, sizeof(double));

	while (read_next_num(f, &num)) {
		(*arr)[len] = num;
		len++;

		if (len == maxlen)
			return maxlen;
	}

	return len;
}

int read_next_num(FILE *f, long *num)
{
	char numbuf[MAX_NUM_LEN + 1] = "";
	char buf[] = "\0";
	size_t numbuf_i;

	numbuf_i = 0;

	while (fread(buf, 1, sizeof(char), f) != 0) {
		if (is_digit(*buf)) {
			if (numbuf_i == MAX_NUM_LEN - 1) {
				fprintf(stderr, "number too long\n");
				exit(EXIT_FAILURE);
			}

			numbuf[numbuf_i] = *buf;
			numbuf[numbuf_i + 1] = '\0';
			numbuf_i++;
		} else {
			*num = strtod(numbuf, NULL);
			return 1;
		}
	}

	return 0;
}
