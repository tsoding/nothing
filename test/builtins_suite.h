#ifndef BUILTINS_SUITE_H_
#define BUILTINS_SUITE_H_

#include "test.h"
#include "script/gc.h"

TEST(callable_p_test)
{
    Gc *gc = create_gc();

    ASSERT_TRUE(!callable_p(NIL(gc)),
                "nil should not be callable");
    ASSERT_TRUE(!callable_p(list(gc, 1, SYMBOL(gc, "lambda"))),
                "(lambda) should not be callable");
    ASSERT_TRUE(!callable_p(list(gc, 2,
                                SYMBOL(gc, "lambda"),
                                list(gc, 1, NUMBER(gc, 1)))),
                "(lambda (1)) should not be callable");
    ASSERT_TRUE(callable_p(list(gc, 2,
                                 SYMBOL(gc, "lambda"),
                                 list(gc, 1, SYMBOL(gc, "a")))),
                "(lambda (a)) should be callable");
    ASSERT_TRUE(callable_p(list(gc, 3,
                                SYMBOL(gc, "lambda"),
                                list(gc, 1, SYMBOL(gc, "a")),
                                STRING(gc, "hello"))),
                "(lambda (a) \"hello\") should be callable");

    destroy_gc(gc);

    return 0;
}

TEST_SUITE(builtins_suite)
{
    TEST_RUN(callable_p_test);

    return 0;
}

#endif  // BUILTINS_SUITE_H_
