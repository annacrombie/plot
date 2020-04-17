#ifndef __READ_ARR_H
#define __READ_ARR_H

#include <stdio.h>

#include "plot.h"

void pdread_all_available(struct plot *p);
int pdtry_all_buffers(struct plot *p, int shift);
int pdtry_buffer(struct plot_data *pd, size_t max_w, int shift);
void input_init(void);
void set_input_buffer_size(size_t new_size);
#endif
