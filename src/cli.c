#include "plot.h"

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

  plot(height, label_format, arrlen, arr);

  return 0;
}
