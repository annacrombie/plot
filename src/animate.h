#ifndef ANIMATE_H
#define ANIMATE_H
#include <stdbool.h>

struct plot;

typedef bool ((*fetch_new_data)(struct plot *p));

void animate_plot(struct plot *p, long ms, fetch_new_data fetch);
#endif
