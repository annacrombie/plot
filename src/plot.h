#ifndef PLOT_H__

#define PLOT_H__
#define PLOT_VERSION "0.1.0"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

struct plot_format {
  int height;
  int color;
  char *label_format;
};

struct plot_format *init_plot_format(char *);
void plot(int , double *, struct plot_format *);
#endif
