#ifndef INTERPRETER_SUITE_H_
#define INTERPRETER_SUITE_H_

#include "test.h"
#include "ebisp/builtins.h"

TEST(equal_test)
{
    Gc *gc = create_gc();

    struct Expr nil1 = NIL(gc);
    struct Expr nil2 = NIL(gc);
    ASSERT_TRUE(equal(nil1, nil2), "nils are not equal");

    struct Expr list1 =
        CONS(gc, SYMBOL(gc, "a"),
             CONS(gc, SYMBOL(gc, "b"),
                  CONS(gc, SYMBOL(gc, "c"),
                       NIL(gc))));
    struct Expr list2 =
        CONS(gc, SYMBOL(gc, "a"),
             CONS(gc, SYMBOL(gc, "b"),
                  CONS(gc, SYMBOL(gc, "c"),
                       NIL(gc))));
    ASSERT_TRUE(equal(list1, list2), "lists are not equal");

    destroy_gc(gc);

    return 0;
}

TEST(assoc_test)
{
    Gc *gc = create_gc();

    struct Expr nil = NIL(gc);

    struct Expr a = SYMBOL(gc, "a");
    struct Expr b = SYMBOL(gc, "b");
    struct Expr c = SYMBOL(gc, "c");

    struct Expr a_pair = CONS(gc, a, NUMBER(gc, 10.0f));
    struct Expr b_pair = CONS(gc, b, NUMBER(gc, 20.0f));
    struct Expr c_pair = CONS(gc, c, NUMBER(gc, 30.0f));

    struct Expr alist =
        CONS(gc, a_pair,
             CONS(gc, b_pair,
                  CONS(gc, c_pair, nil)));

    ASSERT_TRUE(equal(a_pair, assoc(a, alist)), "unexpected pair retrieved");

    destroy_gc(gc);

    return 0;
}

TEST_SUITE(interpreter_suite)
{
    TEST_RUN(equal_test);
    TEST_RUN(assoc_test);

    return 0;
}

#endif  // INTERPRETER_SUITE_H_
