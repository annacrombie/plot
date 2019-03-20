#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
//#include "log.h"

double *make_arr(int argc, char **argv) {
  double *arr = calloc(argc, sizeof(double));
  for (int i=0;i<argc;i++) {
    arr[i] = strtod(argv[i], NULL);
  }

  return arr;
}

void print_help() {
  printf("usage: plot [OPTS] [point[ point [ ...]]]\n");
}

int main(int argc, char **argv) {
  int i = 1;
  int height = 16;
  char *label_format = "%11.2f %s";
  /* Parse options */
  char *opts[] = { "-h", "-H", "-f" };
  for (;i<argc;i++) {
    if (strcmp(argv[i], opts[0]) == 0) {
      print_help();
      return 0;
    } else if (strcmp(argv[i], opts[1]) == 0) {
      if (argc >= i + 1) {
        height = atoi(argv[i+1]);
        if (height < 1) {
          printf("error: height must be >= 1\n");
          return 1;
        }
        i++;
      } else {
        printf("error: %s requires a value\n", opts[1]);
        return 1;
      }
    } else if (strcmp(argv[i], opts[2]) == 0) {
      if (argc >= i + 1) {
        label_format = argv[i+1];
        i++;
      } else {
        printf("error: %s requires a value\n", opts[1]);
        return 1;
      }

    } else {
      break;
    }
  }

  /* Get the array from the rest of the options */
  int arrlen = argc - i;
  double *arr;
  if (arrlen < 1) { print_help(); return 1; }
  arr = make_arr(arrlen, &argv[i]);

  /* Determine the max and min of the array*/
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
    printf(label_format, labels[i], &chars[4]);
    for (j=0;j<arrlen;j++) {
      fputs(graph[j] + (i * 4), stdout);
    }
    fputs("\n", stdout);
  }

  /* free everything */
  free(arr);
  free(larr);
  free(labels);
  for (i=0; i<arrlen; i++) { free(graph[i]); }
  free(graph);

  return 0;
}
