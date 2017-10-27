#ifndef SAT_MACROS_H
#define SAT_MACROS_H
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#define BOUND_CHECK(len, idx) assert(idx < len)
#define UNUSED(x)  x __attribute__((unused))
static inline void LOG_FATAL(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "%s:%d: Aborting execution", __FILE__, __LINE__);
  vfprintf(stderr, fmt, args);
  va_end(args);
  abort();
}

static const bool enable_debug = true;

static inline void LOG_DEBUG(const char* fmt, ...) {
  if (enable_debug) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
  }
}

#endif //SAT_MACROS_H
