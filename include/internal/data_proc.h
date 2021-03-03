#ifndef DATA_PROC_H
#define DATA_PROC_H

#include <stdbool.h>
#include <stdint.h>

#include "plot/plot.h"

typedef bool ((*plot_data_proc_init))(void *proc_ctx, void *opts, uint32_t opts_size);

struct dproc_registry_elem {
	plot_data_proc_proc proc;
	plot_data_proc_init init;
};

extern const struct dproc_registry_elem dproc_registry[data_proc_type_count];
#endif
