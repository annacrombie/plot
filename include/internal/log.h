#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#define L(...) do { \
		if (plot_debug_logfile) { \
			fprintf(plot_debug_logfile, "%s:%d [\033[35m%s\033[0m] ", __FILE__, __LINE__, __func__); \
			fprintf(plot_debug_logfile, __VA_ARGS__); \
			fprintf(plot_debug_logfile, "\n"); \
		} \
} while(0) \

extern FILE *plot_debug_logfile;
#endif
