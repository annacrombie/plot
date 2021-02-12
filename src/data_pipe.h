#ifndef DATA_PIPE_H
#define DATA_PIPE_H

#include <stdbool.h>

#include "data_proc.h"
#include "plot.h"

bool pipeline_create(char *path);
bool pipeline_append(enum data_proc_type proc, void *ctx, uint32_t size);
void pipeline_exec_all(struct plot *p);
#endif
