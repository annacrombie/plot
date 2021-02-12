#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <stdbool.h>

#include "data_proc.h"

#define MAX_INBUF 1024
struct input {
	char buf[MAX_INBUF + 1]; // + 1 so we can always be sure there will be
	                         // a sentinel 0 at the end, as long as
	                         // everyone abides by MAX_INBUF
	FILE *src;
	size_t rem; // the number of remaining bytes left over in buf
	struct dbuf out;
};


bool input_read(struct input *in);
#endif
