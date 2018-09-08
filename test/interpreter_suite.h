#ifndef INTERPRETER_SUITE_H_
#define INTERPRETER_SUITE_H_

#include "test.h"
#include "script/builtins.h"

TEST(equal_test)
{
    struct Expr nil1 = NIL;
    struct Expr nil2 = NIL;
    ASSERT_TRUE(equal(nil1, nil2), "nils are not equal");
    destroy_expr(nil1);
    destroy_expr(nil2);

    struct Expr list1 =
        CONS(SYMBOL("a"),
             CONS(SYMBOL("b"),
                  CONS(SYMBOL("c"),
                       NIL)));
    struct Expr list2 =
        CONS(SYMBOL("a"),
             CONS(SYMBOL("b"),
                  CONS(SYMBOL("c"),
                       NIL)));
    ASSERT_TRUE(equal(list1, list2), "lists are not equal");
    destroy_expr(list1);
    destroy_expr(list2);

    return 0;
}

TEST(assoc_test)
{
    struct Expr nil = NIL;

    struct Expr a = SYMBOL("a");
    struct Expr b = SYMBOL("b");
    struct Expr c = SYMBOL("c");

    struct Expr a_pair = CONS(a, NUMBER(10.0f));
    struct Expr b_pair = CONS(b, NUMBER(20.0f));
    struct Expr c_pair = CONS(c, NUMBER(30.0f));

    struct Expr alist =
        CONS(a_pair,
             CONS(b_pair,
                  CONS(c_pair, nil)));

    ASSERT_TRUE(equal(a_pair, assoc(a, alist)), "unexpected pair retrieved");

    destroy_expr(alist);
    return 0;
}
TEST_SUITE(interpreter_suite)
{
    TEST_RUN(equal_test);
    TEST_RUN(assoc_test);

    return 0;
}

#endif  // INTERPRETER_SUITE_H_
