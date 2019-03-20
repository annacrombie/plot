#ifndef LOG_H
#define LOG_H

#ifdef DEBUG
#define LOG(...) printf("%s[%d] - ", __FILE__, __LINE__);printf(__VA_ARGS__);
#define FLOG(msg, ...) fprintf(stderr, "%s[%d] - "msg, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#else
#define LOG(msg, ...)
#define FLOG(msg, ...)
#endif
#endif
