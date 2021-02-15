#ifndef DATA_PROC_H
#define DATA_PROC_H

#include <stdbool.h>
#include <stdint.h>

#define DATA_LEN 128

struct dbuf { double dat[DATA_LEN]; uint32_t i, len; };

typedef void ((*dproc))(struct dbuf *out, struct dbuf *in, void *ctx);
typedef bool ((*dproc_ctx_validate))(void *ctx, uint32_t size);

enum data_proc_type {
	data_proc_avg,
	data_proc_sma,
	data_proc_cma,
	data_proc_roc,
	data_proc_type_count,
};

struct dproc_registry_elem {
	dproc dproc;
	dproc_ctx_validate ctx_validate;
};

extern const struct dproc_registry_elem dproc_registry[data_proc_type_count];
#endif
