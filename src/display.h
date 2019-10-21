#ifndef _PLOT_DISPLAY_H
#define _PLOT_DISPLAY_H
#include <stdlib.h>
void plot_display(struct plot *plot, double *labels, long **norm);
void set_custom_plot_charset(char *str, size_t len);
#endif
