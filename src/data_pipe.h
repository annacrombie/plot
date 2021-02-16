#ifndef DATA_PIPE_H
#define DATA_PIPE_H

#include <stdbool.h>

#include "data_proc.h"
#include "plot.h"

enum pipeline_flags {
	pipeline_flag_rewind   = 1 << 0,
	pipeline_flag_nonblock = 1 << 1,
};

bool pipeline_create(const char *path, uint8_t pipeline_flags);
bool pipeline_append(enum data_proc_type proc, void *ctx, uint32_t size);
bool pipeline_exec_all(struct plot *p, uint32_t max_new);
void pipeline_reset_eofs(void);
void pipeline_fast_fwd(struct plot *p);
#endif
