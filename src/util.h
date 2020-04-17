#ifndef __UTIL_H
#define __UTIL_H

#include <stdio.h>
#include <stdlib.h>

int is_digit(char c);
unsigned int char_to_color(char c);
unsigned int utf8_bytes(const char *utf8);
#endif
