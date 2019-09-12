#include "read_arr.h"

#define MAX_NUM_LEN 20

size_t read_arr(FILE *f, double **arr, size_t maxlen)
{
	char numbuf[MAX_NUM_LEN + 1] = "";
	char buf[] = "\0";
	size_t numbuf_i, len;

	numbuf_i = len = 0;

	*arr = safe_calloc(maxlen, sizeof(double));

	while (fread(buf, 1, sizeof(char), f) != 0) {
		switch (*buf) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			if (numbuf_i == MAX_NUM_LEN - 1) {
				fprintf(stderr, "number too long\n");
				exit(EXIT_FAILURE);
			}

			numbuf[numbuf_i] = *buf;
			numbuf[numbuf_i + 1] = '\0';
			numbuf_i++;
			break;
		default:
			(*arr)[len] = strtod(numbuf, NULL);
			len++;
			numbuf_i = 0;
			if (len == maxlen)
				return maxlen;
			break;

		}
	}

	return len;
}

