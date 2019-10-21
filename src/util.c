#include "util.h"

void *safe_malloc(size_t size)
{
	void *p;

	if ((p = malloc(size)) == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(EXIT_FAILURE);
	}

	return p;
}

void *safe_calloc(size_t nmemb, size_t size)
{
	void *p;

	if ((p = calloc(nmemb, size)) == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(EXIT_FAILURE);
	}

	return p;
}

void *safe_realloc(void *p, size_t size)
{
	void *np;

	if ((np = realloc(p, size)) == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(EXIT_FAILURE);
	}

	return np;
}

int is_digit(char c)
{
	switch (c) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	case '-': case '.':
		return 1;
	default:
		return 0;
	}
}

unsigned int char_to_color(char c)
{
	unsigned int n;

	switch (c) {
	case 'b': n = 30; break;
	case 'r': n = 31; break;
	case 'g': n = 32; break;
	case 'y': n = 33; break;
	case 'l': n = 34; break;
	case 'm': n = 35; break;
	case 'c': n = 36; break;
	case 'w': n = 37; break;
	case 'B': n = 90; break;
	case 'R': n = 91; break;
	case 'G': n = 92; break;
	case 'Y': n = 93; break;
	case 'L': n = 94; break;
	case 'M': n = 95; break;
	case 'C': n = 96; break;
	case 'W': n = 97; break;
	default:  n = 0;  break;
	}

	return n;
}

unsigned int utf8_bytes(const char *utf8)
{
	unsigned char c;

	c = utf8[0];

	if ((c & 0x7f) == c)
		return 1;
	else if ((c & 0xdf) == c)
		return 2;
	else if ((c & 0xef) == c)
		return 3;
	else if ((c & 0xf7) == c)
		return 4;
	else
		return 0;
}
