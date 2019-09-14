#ifndef __READ_ARR_H
#define __READ_ARR_H
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "util.h"
size_t read_arr(FILE *f, double **arr, size_t maxlen);
int read_next_num(FILE *f, long *num);
#endif
