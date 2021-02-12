#include "posix.h"

#include <assert.h>

#include "data_proc.h"
#include "log.h"

static void
proc_avg(struct dbuf *out, struct dbuf *in, void *ctx)
{
	uint32_t i, n = *(uint32_t *)ctx;
	double sum;

	for (; in->i + n < in->len; in->i += n) {
		sum = 0.0;

		for (i = 0; i < n; ++i) {
			sum += in->dat[in->i + i];
		}

		out->dat[out->len] = sum / (double)n;

		if (++out->len > DATA_LEN) {
			break;
		}
	}
}

static void
proc_sma(struct dbuf *out, struct dbuf *in, void *ctx)
{
	uint32_t i, n = *(uint32_t *)ctx;
	double sum;

	for (; in->i + n < in->len; ++in->i) {
		sum = 0.0;

		for (i = 0; i < n; ++i) {
			sum += in->dat[in->i + i];
		}

		out->dat[out->len] = sum / (double)n;

		if (++out->len > DATA_LEN) {
			break;
		}
	}
}

static bool
avg_validator(void *ctx, uint32_t size)
{
	assert(size == sizeof(uint32_t));

	uint32_t n = *(uint32_t *)ctx;

	if (n == 0 || n >= DATA_LEN) {
		fprintf(stderr, "invalid argument: %d\n", n);
		return false;
	} else {
		return true;
	}
}

const struct dproc_registry_elem dproc_registry[data_proc_type_count] = {
	[data_proc_avg] = { proc_avg, avg_validator },
	[data_proc_sma] = { proc_sma, avg_validator },
};
