#ifndef STACKTRACE_H_
#define STACKTRACE_H_

#ifndef NDEBUG
#define trace_assert(condition) (void)((condition) || (__trace_assert(__FILE__, __LINE__, __func__, #condition), 0))
#else
#define trace_assert(condition) (void)(condition)
#endif
void __trace_assert(const char *file, int line, const char *function, const char *message);

void print_stacktrace(void);

#endif  // STACKTRACE_H_
