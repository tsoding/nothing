#ifndef BUILTINS_SUITE_H_
#define BUILTINS_SUITE_H_

#include "test.h"
#include "ebisp/gc.h"

TEST(lambda_p_test)
{
    Gc *gc = create_gc();

    ASSERT_TRUE(!lambda_p(NIL(gc)),
                { fprintf(stderr, "nil should not be lambda"); });
    ASSERT_TRUE(!lambda_p(list(gc, "q", "lambda")),
                { fprintf(stderr, "(lambda) should not be lambda"); });
    ASSERT_TRUE(!lambda_p(list(gc, "qe", "lambda", list(gc, "d", 1))),
                { fprintf(stderr, "(lambda (1)) should not be lambda"); });
    ASSERT_TRUE(lambda_p(list(gc, "qe",
                                     "lambda",
                                     list(gc, "q", "a"))),
                { fprintf(stderr, "(lambda (a)) should be lambda"); });
    ASSERT_TRUE(lambda_p(list(gc, "qes",
                                     "lambda",
                                     list(gc, "q", "a"),
                                     "hello")),
                { fprintf(stderr, "(lambda (a) \"hello\") should be lambda"); });

    destroy_gc(gc);

    return 0;
}

TEST_SUITE(builtins_suite)
{
    TEST_RUN(lambda_p_test);

    return 0;
}

#endif  // BUILTINS_SUITE_H_
