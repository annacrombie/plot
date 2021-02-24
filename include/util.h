#ifndef __UTIL_H
#define __UTIL_H

#include "plot.h"

enum color char_to_color(char c);
unsigned int color_to_ansi_escape_color(enum color);
unsigned int utf8_bytes(const char *utf8);
#endif
