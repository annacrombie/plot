#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <time.h>
#include "util.h"
#include "plot.h"
#include "input.h"

void
follow_plot(struct plot *p)
{
	size_t i;
	long height = p->height + (p->x_label.every > 0 ? 1 : 0);

	struct timespec sleep = {
		.tv_sec = 0,
		.tv_nsec = 500,
	};

	printf("%c[?25l", 27);

	while (1) {
		if (!pdtry_all_buffers(p, 1)) {
			for (i = 0; i < p->datasets; i++) {
				if (feof(p->data[i].src.src)) {
					clearerr(p->data[i].src.src);
				}
			}

			nanosleep(&sleep, NULL);
			continue;
		}

		plot_plot(p);
		printf("%c[%ldA", 27, height);
	}

	printf("%c[%ldB%c[?12l%c[?25h", 27, height, 27, 27);
}
