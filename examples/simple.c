#include <math.h>
#include <plot/plot.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>

static uint32_t
sin_wave(void *_ctx, double *out, uint32_t out_max)
{
	double *x = _ctx, v = sin(*x);
	*x += 0.1;
	out[0] = v;
	return 1;
}

#define BUFLEN 99999

int
main(void)
{
	char buf[BUFLEN] = { 0 };
	double x = 0;

	struct plot *p = plot_alloc(24, 80, 1);

	plot_add_dataset(p, plot_color_green, NULL, 0, sin_wave, &x);
	plot_fetch_until_full(p);
	plot_plot(p, buf, BUFLEN);
	plot_free(p);

	printf("%s", buf);
}
