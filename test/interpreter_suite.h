#ifndef INTERPRETER_SUITE_H_
#define INTERPRETER_SUITE_H_

#include "test.h"
#include "ebisp/builtins.h"
#include "ebisp/expr.h"
#include "ebisp/interpreter.h"

TEST(equal_test)
{
    Gc *gc = create_gc();

    struct Expr nil1 = NIL(gc);
    struct Expr nil2 = NIL(gc);
    ASSERT_TRUE(equal(nil1, nil2), {
            fprintf(stderr, "Expected: ");
            print_expr_as_sexpr(stderr, nil1);
            fprintf(stderr, "\n");

            fprintf(stderr, "Actual: ");
            print_expr_as_sexpr(stderr, nil2);
            fprintf(stderr, "\n");
    });

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
    ASSERT_TRUE(equal(list1, list2), {
            fprintf(stderr, "Expected: ");
            print_expr_as_sexpr(stderr, list1);
            fprintf(stderr, "\n");

            fprintf(stderr, "Actual: ");
            print_expr_as_sexpr(stderr, list2);
            fprintf(stderr, "\n");
    });

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

    struct Expr a_pair = CONS(gc, a, INTEGER(gc, 10));
    struct Expr b_pair = CONS(gc, b, INTEGER(gc, 20));
    struct Expr c_pair = CONS(gc, c, INTEGER(gc, 30));

    struct Expr alist =
        CONS(gc, a_pair,
             CONS(gc, b_pair,
                  CONS(gc, c_pair, nil)));

    struct Expr assoc_result = assoc(a, alist);

    ASSERT_TRUE(equal(a_pair, assoc_result), {
            fprintf(stderr, "Expected: ");
            print_expr_as_sexpr(stderr, a_pair);
            fprintf(stderr, "\n");

            fprintf(stderr, "Actual: ");
            print_expr_as_sexpr(stderr, assoc_result);
            fprintf(stderr, "\n");
    });

    destroy_gc(gc);

    return 0;
}

TEST(match_list_test)
{
    Gc *gc = create_gc();

    struct Expr input = list(
        gc, "dsqe",
        42,
        "hello",
        "world",
        CONS(gc, INTEGER(gc, 1), INTEGER(gc, 2)));

    long int d = 0;
    const char *s = NULL;
    const char *q = NULL;
    struct Expr e = NIL(gc);

    struct EvalResult result = match_list(gc, "dsqe", input, &d, &s, &q, &e);
    ASSERT_FALSE(result.is_error, {
            fprintf(stderr, "Unpack failed: ");
            print_expr_as_sexpr(stderr, result.expr);
            fprintf(stderr, "\n");
    });

    ASSERT_TRUE(d == 42, {
        fprintf(stderr, "Expected: 42, Actual: %ld\n", d);
    });
    ASSERT_TRUE(strcmp(s, "hello") == 0, {
        fprintf(stderr, "Expected: hello, Actual: %s\n", s);
    });
    ASSERT_TRUE(strcmp(q, "world") == 0, {
        fprintf(stderr, "Expected: world, Actual: %s\n", q);
    });
    ASSERT_TRUE(equal(e, CONS(gc, INTEGER(gc, 1), INTEGER(gc, 2))), {
            fprintf(stderr, "Expected: ");
            print_expr_as_sexpr(stderr, CONS(gc, INTEGER(gc, 1), INTEGER(gc, 2)));
            fprintf(stderr, ", Actual: ");
            print_expr_as_sexpr(stderr, e);
            fprintf(stderr, "\n");
    });

    destroy_gc(gc);

    return 0;
}

TEST(match_list_empty_list_test)
{
    Gc *gc = create_gc();

    long int d = 0;
    struct EvalResult result = match_list(gc, "d", NIL(gc), &d);

    ASSERT_TRUE(result.is_error, {
            fprintf(stderr, "Unpack did not fail\n");
    });

    destroy_gc(gc);

    return 0;
}

TEST(match_list_head_tail_test)
{
    Gc *gc = create_gc();

    struct Expr input = list(gc, "dddd", 1, 2, 3, 4);

    long int x = 0;
    struct Expr xs = NIL(gc);

    struct EvalResult result = match_list(gc, "d*", input, &x, &xs);
    ASSERT_TRUE(!result.is_error, {
            fprintf(stderr, "Could not unpack input: ");
            print_expr_as_sexpr(stderr, result.expr);
            fprintf(stderr, "\n");
    });

    ASSERT_TRUE(x == 1, {
            fprintf(stderr, "Expected: 1, Actual: %ld\n", x);
    });

    ASSERT_TRUE(equal(xs, list(gc, "ddd", 2, 3, 4)), {
            fprintf(stderr, "Expected: (2 3 4), Actual: ");
            print_expr_as_sexpr(stderr, xs);
            fprintf(stderr, "\n");
    })

    destroy_gc(gc);

    return 0;
}

TEST(match_list_wildcard_test)
{
    Gc *gc = create_gc();

    struct Expr input = list(
        gc, "dddd", 1, 2, 3, 4);

    long int x = 0, y = 0;
    struct EvalResult result = match_list(gc, "dddd", input, &x, NULL, &y, NULL);
    ASSERT_TRUE(!result.is_error, {
            fprintf(stderr, "Matching failed: ");
            print_expr_as_sexpr(stderr, result.expr);
            fprintf(stderr, "\n");
    });

    ASSERT_LONGINTEQ(1L, x);
    ASSERT_LONGINTEQ(3L, y);

    destroy_gc(gc);

    return 0;
}

TEST(match_list_singleton_tail_test)
{
    Gc *gc = create_gc();

    struct Expr input = list(gc, "d", 1);
    long int x;
    struct Expr xs = NIL(gc);
    struct EvalResult res = match_list(gc, "d*", input, &x, &xs);

    ASSERT_TRUE(!res.is_error, {
            fprintf(stderr, "Matching failed: ");
            print_expr_as_sexpr(stderr, res.expr);
            fprintf(stderr, "\n");
    });

    ASSERT_LONGINTEQ(1L, x);
    ASSERT_TRUE(nil_p(xs), {
            fprintf(stderr, "Tail doesn't appear to be NIL: ");
            print_expr_as_sexpr(stderr, xs);
            fprintf(stderr, "\n");
    });

    destroy_gc(gc);

    return 0;
}

TEST_SUITE(interpreter_suite)
{
    TEST_RUN(equal_test);
    TEST_RUN(assoc_test);
    TEST_RUN(match_list_test);
    TEST_RUN(match_list_empty_list_test);
    TEST_RUN(match_list_head_tail_test);
    TEST_RUN(match_list_wildcard_test);
    TEST_RUN(match_list_singleton_tail_test);

    return 0;
}

#endif  // INTERPRETER_SUITE_H_
