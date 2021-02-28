#ifndef _PLOT_DISPLAY_H
#define _PLOT_DISPLAY_H
#include <stdint.h>

struct plot;

void plot_render(struct plot *plot, char *bufmem, uint32_t buflen);
#endif
