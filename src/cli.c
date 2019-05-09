#include <ctype.h>
#include "plot.h"

#define ARR_GROW_INC 5

size_t read_arr(double **arr) {
  double *narr, *tarr;
  char buf[20] = "";
  char rd[1] = "";
  size_t t, bon, arr_len, arr_cap;
  t = bon = arr_len = arr_cap = 0;

  for (;bon < 20; bon++) buf[bon] = '\0';
  bon = 0;

  *arr = calloc(ARR_GROW_INC, sizeof(double));
  if (NULL == *arr) {
    fprintf(stderr, "out of memory\n");
    exit(1);
  }
  arr_cap += ARR_GROW_INC;

  while (1) {
    t = fread(rd, 1, sizeof(char), stdin);
    if (t == 0) return arr_len;

    if (!isspace(*rd)) {
      buf[bon] = *rd;
      bon++;
    } else {
      (*arr)[arr_len] = strtod(buf, NULL);
      arr_len += 1;

      while (bon>0) {
        buf[bon] = '\0';
        bon--;
      }

      if (arr_len >= arr_cap) {
        narr = calloc(arr_cap + ARR_GROW_INC, sizeof(double));
        memcpy(narr, *arr, sizeof(double) * arr_cap);
        tarr = *arr;
        *arr = narr;
        free(tarr);
        arr_cap += ARR_GROW_INC;
      }
    }
  }
}

double *make_arr(int argc, char **argv) {
  double *arr = calloc(argc, sizeof(double));
  if (NULL == arr) {
    fprintf(stderr, "out of memory\n");
    exit(1);
  }
  for (int i=0;i<argc;i++) {
    arr[i] = strtod(argv[i], NULL);
  }

  return arr;
}

void print_help() {
  printf(
    "usage: plot [OPTS] -|point[ point [ ...]]\n"
    "\n"
    "OPTS\n"
    "  -H HEIGHT - set total plot height to HEIGHT\n"
    "     (default:  16)\n"
    "  -f STRING - set y-axis label format string to STRING\n"
    "     (default:  \"%%11.2f %%s\")\n"
    "  -h - duh...\n"
  );
}

int main(int argc, char **argv) {
  int i = 1;
  int height = 16;
  char *label_format = "%11.2f %s";
  /* Parse options */
  char *opts[] = { "-h", "-H", "-f", "-v" };
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
    } else if (strcmp(argv[i], opts[3]) == 0) {
      printf("plot v%s\n", PLOT_VERSION);
      return 0;
    } else {
      break;
    }
  }

  /* Get the array from the rest of the options */
  size_t arrlen = argc - i;
  double *arr;
  if (arrlen < 1) { print_help(); return 1; }

  if (strcmp(argv[i], "-") == 0) {
    arrlen = read_arr(&arr);
    if (arrlen < 1) { return 0; }
  } else {
    arr = make_arr(arrlen, &argv[i]);
  }

  plot(height, label_format, arrlen, arr);
  free(arr);

  return 0;
}
