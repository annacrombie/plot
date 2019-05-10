#include "plot.h"

static char *default_format = "%11.2f %s";

struct plot_format *init_plot_format() {
  struct plot_format *format;
  format = malloc(sizeof(struct plot_format));

  if (format == NULL) {
    fprintf(stderr, "out of memory\n");
    exit(1);
  }

  format->height = 16;
  format->color = 0;
  format->label_format = default_format;

  return format;
}

void plot(int arrlen, double *arr) {
  struct plot_format *pf = init_plot_format();
  plotf(arrlen, arr, pf);
  free(pf);
}

void plotf(int arrlen, double *arr, struct plot_format *pf) {
  /* Determine the max and min of the array*/
  int i;
  double max = -1e308;
  double min = 1e308;
  for (i=0;i<arrlen;i++) {
    if (arr[i] > max) max = arr[i];
    if (arr[i] < min) min = arr[i];
  }

  if (max == min) {
    max += 7.5;
    min -= 7.5;
  }

  /* Create the labels for the graph */
  double *labels = calloc(pf->height, sizeof(double));
  double inc = (max - min) / (double)(pf->height - 1);
  double s =  min;
  for (i=0;i<pf->height;i++) {
    labels[i] = s;
    s += inc;
  }

  /* normalize the values from 0 to height and place the results in a new array */
  long *larr = calloc(arrlen + 1, sizeof(long));
  for (i=0;i<arrlen;i++) {
    larr[i] = lround((arr[i] - min) * (double)(pf->height - 1) / (max - min));
  }
  larr[arrlen] = larr[arrlen-1];

  /* create the graph */
  int j;
  char *chars = "┤\0┼\0─\0│\0╰\0╭\0╮\0╯\0 \0\0\0";
  //             0  4  8  12 16 20 24 28 32
  //             byte index
  char **graph = calloc(arrlen, sizeof(char *));
  size_t index;
  for (i=0;i<arrlen;i++) {
    graph[i] = calloc(pf->height, 4*sizeof(char));
    for (j=0;j<pf->height;j++) {
      if (j == larr[i]) {
        index =
          larr[i + 1] > larr[i] ? 28
                                : larr[i + 1] < larr[i] ? 24
                                                        : 8;
      } else if (j == larr[i + 1]) {
        index =
          larr[i + 1] > larr[i] ? 20
                                : larr[i + 1] < larr[i] ? 16
                                                        : 8;
      } else if ((j > larr[i] && j < larr[i + 1]) || (j < larr[i] && j > larr[i + 1])) {
        index = 12;
      } else {
        index = 32;
      }

      memcpy(graph[i] + (j * 4), &chars[index], 4);
    }
  }

  /* print the graph with labels */
  for (i=pf->height-1;i>=0;i--) {
    if (pf->color != 0) {
      if (i == larr[0]) printf("\e[%dm", pf->color);
      else printf("\e[0m");
    }

    printf(pf->label_format, labels[i], i == larr[0] ? &chars[4] : &chars[0]);
    for (j=0;j<arrlen;j++) {
      if (pf->color != 0) printf("\e[%dm", pf->color);
      fputs(graph[j] + (i * 4), stdout);
    }
    if (pf->color) fputs("\e[K", stdout);
    fputs("\n", stdout);
  }

  if (pf->color) fputs("\e[0m", stdout);

  /* free everything */
  free(larr);
  free(labels);
  for (i=0; i<arrlen; i++) { free(graph[i]); }
  free(graph);
}

