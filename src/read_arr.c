#include "read_arr.h"

#define ARR_GROW_INC 5

size_t read_arr(double **arr)
{
	double *narr, *tarr;
	char buf[20] = "";
	char rd[1] = "";
	size_t t, bon, arr_len, arr_cap;

	t = bon = arr_len = arr_cap = 0;

	for (; bon < 20; bon++) buf[bon] = '\0';
	bon = 0;

	*arr = calloc(ARR_GROW_INC, sizeof(double));
	if (NULL == *arr) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}
	arr_cap += ARR_GROW_INC;

	while (1) {
		t = fread(rd, 1, sizeof(char), stdin);
		if (t == 0)
			return arr_len;

		if (!isspace(*rd)) {
			buf[bon] = *rd;
			bon++;
		} else {
			(*arr)[arr_len] = strtod(buf, NULL);
			arr_len += 1;

			while (bon > 0) {
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

