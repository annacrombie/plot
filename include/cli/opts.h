#ifndef OPTS_H
#define OPTS_H

#include "cli/main.h"

enum mode {
	mode_normal,
	mode_animate,
	mode_follow,
};

struct opts {
	enum mode mode;
	uint32_t follow_rate;
};

void parse_opts(struct opts *opts, struct plot *p,
	struct plot_static_memory *mem, int argc, char **argv);
#endif
