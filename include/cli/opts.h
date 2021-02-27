#ifndef OPTS_H
#define OPTS_H
#define MAX_DATASETS 32
#define MAX_WIDTH 512
#define MAX_HEIGHT 512

struct plot;

void parse_opts(struct plot *p, int argc, char **argv);
#endif
