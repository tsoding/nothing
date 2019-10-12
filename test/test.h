#ifndef TEST_H_
#define TEST_H_

#include "math.h"

#define TEST_RUN(name)                          \
    if (name() < 0) {                           \
        return -1;                              \
    }

#define TEST_IGNORE(name)                       \
    (void)(name)                                \

#define TEST(name)                              \
    static int name##_body(void);               \
    static int name(void) {                     \
        printf("%s() ...", #name);              \
        if (name##_body() == 0) {               \
            printf(" OK\n");                    \
            return 0;                           \
        } else {                                \
            printf(" FAILED\n");                \
            return -1;                          \
        }                                       \
    }                                           \
    static int name##_body(void)

// TODO(#308): ASSERT_* macros evaluate expressions several times

#define ASSERT_STREQN(expected, actual, n)                              \
    if (strncmp(expected, actual, n) != 0) {                            \
        fprintf(stderr, "\n%s:%d: ASSERT_STREQN: \n",                   \
                __FILE__, __LINE__);                                    \
        fprintf(stderr, "  Expected: ");                                \
        fwrite(expected, sizeof(char), n, stderr);                      \
        fprintf(stderr, "\n");                                          \
        fprintf(stderr, "  Actual: ");                                  \
        fwrite(actual, sizeof(char), n, stderr);                        \
        fprintf(stderr, "\n");                                          \
        return -1;                                                      \
    }

#define ASSERT_STREQ(expected, actual)                                  \
    if (strcmp(expected, actual) != 0) {                                \
        fprintf(stderr, "\n%s:%d: ASSERT_STREQ: \n",                    \
                __FILE__, __LINE__);                                    \
        fprintf(stderr, "  Expected: %s\n", expected);                  \
        fprintf(stderr, "  Actual: %s\n", actual);                      \
        return -1;                                                      \
    }

#define ASSERT_INTEQ(expected, actual)                                  \
    if (expected != actual) {                                           \
        fprintf(stderr, "\n%s:%d: ASSERT_INTEQ: \n",                    \
                __FILE__, __LINE__);                                    \
        fprintf(stderr, "  Expected: %d\n", expected);                  \
        fprintf(stderr, "  Actual: %d\n", actual);                      \
        return -1;                                                      \
    }

#define ASSERT_LONGINTEQ(expected, actual)                              \
    if (expected != actual) {                                           \
        fprintf(stderr, "\n%s:%d: ASSERT_LONGINTEQ: \n",                \
                __FILE__, __LINE__);                                    \
        fprintf(stderr, "  Expected: %ld\n", expected);                 \
        fprintf(stderr, "  Actual: %ld\n", actual);                     \
        return -1;                                                      \
    }


#define ASSERT_FLOATEQ(expected, actual, margin)                        \
    if (fabsf(expected - actual) > margin) {                            \
        fprintf(stderr, "\n%s:%d: ASSERT_FLOATEQ: \n",                  \
                __FILE__, __LINE__);                                    \
        fprintf(stderr, "  Expected: %f\n", expected);                  \
        fprintf(stderr, "  Actual:   %f\n", actual);                    \
        fprintf(stderr, "  Margin:   %f\n", margin);                    \
        return -1;                                                      \
    }

#define ASSERT_EQ(type, expected, actual, handler)                      \
    {                                                                   \
        type _expected = (expected);                                    \
        type _actual = (actual);                                        \
        if (_expected != _actual) {                                     \
            fprintf(stderr, "\n%s:%d: ASSERT_EQ: \n",                   \
                    __FILE__, __LINE__);                                \
            handler                                                     \
            return -1;                                                  \
        }                                                               \
    }

#define ASSERT_TRUE(condition, handler)                                 \
    if (!(condition)) {                                                 \
        fprintf(stderr, "\n%s:%d: ASSERT_TRUE: false\n",                \
                __FILE__, __LINE__);                                    \
        handler                                                         \
        return -1;                                                      \
    }

#define ASSERT_FALSE(condition, handler)                                \
    if (condition) {                                                    \
        fprintf(stderr, "\n%s:%d: ASSERT_FALSE: false\n",               \
                __FILE__, __LINE__);                                    \
        handler                                                         \
        return -1;                                                      \
    }

#define TEST_SUITE(name)                        \
    static int name##_body(void);               \
    static int name(void) {                     \
        if (name##_body() < 0) {                \
            return -1;                          \
        }                                       \
        return 0;                               \
    }                                           \
    static int name##_body(void)

#define TEST_MAIN()                             \
    static int main_body(void);                 \
    int main(void) {                            \
        if (main_body() < 0) {                  \
            return -1;                          \
        }                                       \
        return 0;                               \
    }                                           \
    static int main_body(void)

#endif  // TEST_H_
