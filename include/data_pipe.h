#ifndef DATA_PIPE_H
#define DATA_PIPE_H

#include "plot.h"

bool plot_pipeline_create(plot_input_func input_func, void *input_ctx);
#endif
