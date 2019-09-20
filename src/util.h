#ifndef __UTIL_H
#define __UTIL_H
#include <stdlib.h>
#include <stdio.h>
void *safe_malloc(size_t size);
void *safe_calloc(size_t nmemb, size_t size);
void *safe_realloc(void *p, size_t size);
int is_digit(char c);
unsigned int char_to_color(char c);
#endif
