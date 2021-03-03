#include "posix.h"

#include <assert.h>

#include "internal/data_proc.h"
#include "internal/log.h"

static void
avg_proc(struct plot_dbuf *out, struct plot_dbuf *in, void *ctx)
{
	uint32_t i, *n = ctx;
	double sum;

	for (; in->i + *n < in->len; in->i += *n) {
		sum = 0.0;

		for (i = 0; i < *n; ++i) {
			sum += in->dat[in->i + i];
		}

		out->dat[out->len] = sum / (double)*n;

		if (++out->len > PLOT_DBUF_SIZE) {
			break;
		}
	}
}

static bool
avg_init(void *proc_ctx, void *opts, uint32_t opts_size)
{
	assert(opts_size == sizeof(uint32_t));

	uint32_t *n = opts, *proc_n = proc_ctx;

	if (*n == 0 || *n >= PLOT_DBUF_SIZE) {
		fprintf(stderr, "invalid argument: %d\n", *n);
		return false;
	}

	*proc_n = *n;
	return true;
}

/*
 * this struct (and all pipeline ctx structs must be <= PLOT_PIPLEINE_CTX_SIZE
 */
struct sma_proc_ctx {
	uint32_t n;
	uint32_t sumlen;
	double sum;
};

static void
sma_proc(struct plot_dbuf *out, struct plot_dbuf *in, void *_ctx)
{
	struct sma_proc_ctx *ctx = _ctx;

	/* output zeroes until we have enough data to make sure everything
	 * lines up*/
	while (ctx->leading < ctx->n / 2) {
		++ctx->leading;

		out->dat[out->len] = 0;

		if (++out->len > PLOT_DBUF_SIZE) {
			return;
		}
	}

	uint32_t i;
	double sum;

	for (; in->i + ctx->n < in->len; ++in->i) {

		sum = 0.0;

		for (i = 0; i < ctx->n; ++i) {
			sum += in->dat[in->i + i];
		}

		out->dat[out->len] = sum / (double)ctx->n;

		if (++out->len > PLOT_DBUF_SIZE) {
			break;
		}
	}
}

static bool
sma_init(void *proc_ctx, void *opts, uint32_t opts_size)
{
	assert(opts_size == sizeof(uint32_t));

	uint32_t *n = opts;
	struct sma_proc_ctx *ctx = proc_ctx;

	if (*n == 0 || *n >= PLOT_DBUF_SIZE) {
		fprintf(stderr, "invalid argument: %d\n", *n);
		return false;
	} else if (!(*n & 1)) {
		fprintf(stderr, "sma argument must be odd\n");
		return false;
	}

	ctx->n = *n;

	return true;
}

static void
cma_proc(struct plot_dbuf *out, struct plot_dbuf *in, void *_ctx)
{
	struct {
		double cma;
		double n;
	} *ctx = _ctx;

	for (; in->i < in->len; ++in->i) {
		ctx->cma =
			out->dat[out->len] =  (in->dat[in->i] + ctx->n * ctx->cma)
					     / (ctx->n + 1);

		ctx->n += 1.0;

		if (++out->len > PLOT_DBUF_SIZE) {
			break;
		}
	}
}

static void
roc_proc(struct plot_dbuf *out, struct plot_dbuf *in, void *_ctx)
{
	struct {
		float t;
		double old;
		bool init;
	} *ctx = _ctx;

	if (!ctx->init && in->i < in->len) {
		ctx->old = in->dat[in->i];
		ctx->init = true;
	}

	for (; in->i < in->len; ++in->i) {
		out->dat[out->len] = (in->dat[in->i] - ctx->old) / ctx->t;
		ctx->old = in->dat[in->i];

		if (++out->len > PLOT_DBUF_SIZE) {
			break;
		}
	}
}

static bool
roc_init(void *proc_ctx, void *opts, uint32_t opts_size)
{
	assert(opts_size == sizeof(float));
	if (*(float *)opts == 0.0f) {
		fprintf(stderr, "argument cannot be 0\n");
		return false;
	} else {
		return true;
	}
}

const struct dproc_registry_elem dproc_registry[data_proc_type_count] = {
	[data_proc_avg] = { avg_proc, avg_init },
	[data_proc_sma] = { sma_proc, sma_init },
	[data_proc_cma] = { cma_proc },
	[data_proc_roc] = { roc_proc, roc_init },
};
