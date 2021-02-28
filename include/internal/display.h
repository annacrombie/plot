#ifndef _PLOT_DISPLAY_H
#define _PLOT_DISPLAY_H

#include <stdint.h>
#include <stdio.h>

struct plot;

void plot_render_string(struct plot *plot, char *bufmem, uint32_t buflen);
void plot_render_file(struct plot *plot, FILE *f);
#endif
