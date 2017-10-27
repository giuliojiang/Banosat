#ifndef SAT_MACROS_H
#define SAT_MACROS_H

#define BOUND_CHECK(len, idx) assert(idx < len)
#define UNUSED(x)  x __attribute__((unused))
#define LOG_FATAL(x, ...) LOG_DEBUG(x, ##__VA_ARGS__); \
                            abort();
#ifdef DEBUG
#define LOG_DEBUG(x, ...) fprintf(stderr, x, ##__VA_ARGS__)
#else
#define LOG_DEBUG(x, ...)
#endif

#endif //SAT_MACROS_H
