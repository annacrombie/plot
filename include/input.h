#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <stdbool.h>

#include "data_proc.h"

enum plot_file_input_flags {
	plot_file_input_flag_rewind   = 1 << 0,
	plot_file_input_flag_nonblock = 1 << 1,
	plot_file_input_flag_infinite = 1 << 2,
};

#define MAX_INBUF 1024
struct plot_file_input {
	char buf[MAX_INBUF + 1]; // + 1 so we can always be sure there will be
	                         // a sentinel 0 at the end, as long as
	                         // everyone abides by MAX_INBUF
	FILE *src;
	size_t rem; // the number of remaining bytes left over in buf

	enum plot_file_input_flags flags;
};

bool plot_file_input_read(struct plot_file_input *in, struct dbuf *out);
bool plot_file_input_init(struct plot_file_input *in, const char *path,
	enum plot_file_input_flags flags);
#endif
