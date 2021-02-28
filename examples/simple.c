#include <math.h>
#include <plot/plot.h>
#include <stdio.h>

struct sine_wave_ctx {
	double amp, freq;
	double x;
};

static uint32_t
sine_wave(void *_ctx, double *out, uint32_t out_max)
{
	struct sine_wave_ctx *ctx = _ctx;
	double v = sin(ctx->x) * ctx->amp;
	ctx->x += ctx->freq;
	out[0] = v;
	return 1;
}

int
main(void)
{
	struct plot *p = plot_alloc(24, 80, 12);

	plot_add_dataset(p, plot_color_cyan, NULL, 0, sine_wave,
		&(struct sine_wave_ctx){ .amp = 5, .freq = 0.2 });

	plot_fetch_until_full(p);
	plot_print(p, stdout);
	plot_free(p);
}
