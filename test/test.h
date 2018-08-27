#ifndef TEST_H_
#define TEST_H_

#define TEST_RUN(name)                          \
    if (name() < 0) {                           \
        return -1;                              \
    }

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
