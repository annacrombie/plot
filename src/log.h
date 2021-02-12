#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#define L(...) do { \
		if (logfile) { \
			fprintf(logfile, "%s:%d [\033[35m%s\033[0m] ", __FILE__, __LINE__, __func__); \
			fprintf(logfile, __VA_ARGS__); \
			fprintf(logfile, "\n"); \
		} \
} while(0) \

extern FILE *logfile;
#endif
