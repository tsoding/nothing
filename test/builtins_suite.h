#ifndef BUILTINS_SUITE_H_
#define BUILTINS_SUITE_H_

#include "test.h"
#include "ebisp/gc.h"

TEST(lambda_p_test)
{
    Gc *gc = create_gc();

    ASSERT_TRUE(!lambda_p(NIL(gc)),
                "nil should not be lambda");
    ASSERT_TRUE(!lambda_p(list(gc, 1, SYMBOL(gc, "lambda"))),
                "(lambda) should not be lambda");
    ASSERT_TRUE(!lambda_p(list(gc, 2,
                                SYMBOL(gc, "lambda"),
                                list(gc, 1, NUMBER(gc, 1)))),
                "(lambda (1)) should not be lambda");
    ASSERT_TRUE(lambda_p(list(gc, 2,
                                 SYMBOL(gc, "lambda"),
                                 list(gc, 1, SYMBOL(gc, "a")))),
                "(lambda (a)) should be lambda");
    ASSERT_TRUE(lambda_p(list(gc, 3,
                                SYMBOL(gc, "lambda"),
                                list(gc, 1, SYMBOL(gc, "a")),
                                STRING(gc, "hello"))),
                "(lambda (a) \"hello\") should be lambda");

    destroy_gc(gc);

    return 0;
}

TEST_SUITE(builtins_suite)
{
    TEST_RUN(lambda_p_test);

    return 0;
}

#endif  // BUILTINS_SUITE_H_
