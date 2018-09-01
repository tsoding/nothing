#ifndef INTERPRETER_SUITE_H_
#define INTERPRETER_SUITE_H_

#include "test.h"
#include "script/interpreter.h"

TEST(equal_test)
{
    struct Expr nil1 = NIL;
    struct Expr nil2 = NIL;
    ASSERT_TRUE(equal(nil1, nil2), "nils are not equal");
    destroy_expr(nil1);
    destroy_expr(nil2);

    struct Expr list1 =
        CONS(NUMBER(1.0f),
             CONS(NUMBER(2.0f),
                  CONS(NUMBER(3.0f),
                       NIL)));
    struct Expr list2 =
        CONS(NUMBER(1.0f),
             CONS(NUMBER(2.0f),
                  CONS(NUMBER(3.0f),
                       NIL)));
    ASSERT_TRUE(equal(list1, list2), "lists are not equal");

    return 0;
}

TEST_SUITE(interpreter_suite)
{
    TEST_RUN(equal_test);

    return 0;
}

#endif  // INTERPRETER_SUITE_H_
