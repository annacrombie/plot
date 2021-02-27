#ifndef PLOT_FILE_INPUT_H
#define PLOT_FILE_INPUT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

enum plot_file_input_flags {
	plot_file_input_flag_rewind   = 1 << 0,
	plot_file_input_flag_infinite = 1 << 1,
};

struct plot_file_input {
	char *buf;
	FILE *src;
	uint32_t buf_max;
	uint32_t rem; // the number of remaining bytes left over in buf
	enum plot_file_input_flags flags;
};

uint32_t plot_file_input_read(struct plot_file_input *in, double *out,
	uint32_t out_max);
bool plot_file_input_init(struct plot_file_input *in, char *buf, uint32_t buf_max,
	FILE *f, enum plot_file_input_flags flags);
#endif
