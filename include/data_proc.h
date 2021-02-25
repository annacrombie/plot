#ifndef DATA_PROC_H
#define DATA_PROC_H

#include <stdbool.h>
#include <stdint.h>

#include "plot.h"

#define DATA_LEN 128

struct dbuf { double dat[DATA_LEN]; uint32_t i, len; };

typedef void ((*data_proc_proc))(struct dbuf *out, struct dbuf *in, void *ctx);
typedef bool ((*data_proc_init))(void *ctx, uint32_t size);

struct dproc_registry_elem {
	data_proc_proc proc;
	data_proc_init init;
};

extern const struct dproc_registry_elem dproc_registry[data_proc_type_count];
#endif
