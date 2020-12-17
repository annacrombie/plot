#include <stdlib.h>
#include <stdio.h>

#include "alloc.h"
#include "debug.h"

void* mmalloc(size_t size){
	void* ptr = malloc(size);
	if (ptr == NULL) {
		die("%s", "Error allocating memory\n");
	}

	return ptr;
}

void* ccalloc(size_t num, size_t size){
	void* ptr = calloc(num, size);
	if (ptr == NULL) {
		die("%s", "Error allocating memory\n");
	}

	return ptr;
}
