#ifndef DATA_PIPE_H
#define DATA_PIPE_H

#include <stdbool.h>

#include "data_proc.h"
#include "plot.h"

typedef bool ((*pipeline_input_func)(void *ctx, struct dbuf *out));

bool pipeline_create(pipeline_input_func input_func, void *input_ctx);
bool pipeline_append(enum data_proc_type proc, void *ctx, uint32_t size);
bool pipeline_exec_all(struct plot *p, uint32_t max_new);
void pipeline_fast_fwd(struct plot *p);
#endif
