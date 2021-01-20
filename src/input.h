#ifndef __READ_ARR_H
#define __READ_ARR_H

#include <stdio.h>

#include "plot.h"

void pdread_all_available(struct plot *p);
int pdtry_all_buffers(struct plot *p);
void set_input_buffer_size(size_t new_size);
#endif
