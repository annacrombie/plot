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
