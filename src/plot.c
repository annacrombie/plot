#include "plot.h"

void plot(int height, const char *label_format, int arrlen, double *arr) {
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
  double *labels = calloc(height, sizeof(double));
  double inc = (max - min) / (double)(height - 1);
  double s =  min;
  for (i=0;i<height;i++) {
    labels[i] = s;
    s += inc;
  }

  /* normalize the values from 0 to height and place the results in a new array */
  long *larr = calloc(arrlen + 1, sizeof(long));
  for (i=0;i<arrlen;i++) {
    larr[i] = lround((arr[i] - min) * (double)(height - 1) / (max - min));
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
    graph[i] = calloc(height, 4*sizeof(char));
    for (j=0;j<height;j++) {
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
  for (i=height-1;i>=0;i--) {
    printf(label_format, labels[i], i == larr[0] ? &chars[4] : &chars[0]);
    for (j=0;j<arrlen;j++) {
      fputs(graph[j] + (i * 4), stdout);
    }
    fputs("\n", stdout);
  }

  /* free everything */
  free(larr);
  free(labels);
  for (i=0; i<arrlen; i++) { free(graph[i]); }
  free(graph);
}

