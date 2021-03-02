#ifndef OPTS_H
#define OPTS_H

#include <stdint.h>

#define MAX_DATASETS 32
#define MAX_WIDTH 512
#define MAX_HEIGHT 512

struct plot;

enum mode {
	mode_normal,
	mode_animate,
	mode_follow,
};

struct opts {
	enum mode mode;
	uint32_t follow_rate;
};

void parse_opts(struct opts *opts, struct plot *p, int argc, char **argv);
#endif
