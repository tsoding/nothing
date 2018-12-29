#ifndef STACKTRACE_H_
#define STACKTRACE_H_

#define trace_assert(condition)                         \
    if (!(condition)) {                                 \
        fprintf(                                        \
            stderr,                                     \
            "%s:%d: %s: Assertion `%s' failed\n",       \
            __FILE__, __LINE__,                         \
            __func__,                                   \
            #condition);                                \
        print_stacktrace();                             \
        exit(1);                                        \
    }

void print_stacktrace(void);

#endif  // STACKTRACE_H_
