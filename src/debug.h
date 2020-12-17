#ifndef _H_DEBUG
#define _H_DEBUG

#define DEBUG 1
#include <stdlib.h>

#define debug(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

#define die(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); \
				exit(EXIT_FAILURE);} while (0)
#endif
