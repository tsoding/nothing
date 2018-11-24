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

    struct Expr a_pair = CONS(gc, a, NUMBER(gc, 10.0f));
    struct Expr b_pair = CONS(gc, b, NUMBER(gc, 20.0f));
    struct Expr c_pair = CONS(gc, c, NUMBER(gc, 30.0f));

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

TEST(unpack_args_test)
{
    Gc *gc = create_gc();

    struct Expr input = list(
        gc, 4,
        NUMBER(gc, 42),
        STRING(gc, "hello"),
        SYMBOL(gc, "world"),
        CONS(gc, NUMBER(gc, 1), NUMBER(gc, 2)));

    long int d = 0;
    const char *s = NULL;
    const char *q = NULL;
    struct Expr e = NIL(gc);

    struct EvalResult result = unpack_args(gc, "dsqe", input, &d, &s, &q, &e);
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
    ASSERT_TRUE(equal(e, CONS(gc, NUMBER(gc, 1), NUMBER(gc, 2))), {
            fprintf(stderr, "Expected: ");
            print_expr_as_sexpr(stderr, CONS(gc, NUMBER(gc, 1), NUMBER(gc, 2)));
            fprintf(stderr, ", Actual: ");
            print_expr_as_sexpr(stderr, e);
            fprintf(stderr, "\n");
    });

    destroy_gc(gc);

    return 0;
}

TEST(unpack_args_empty_list_test)
{
    Gc *gc = create_gc();

    long int d = 0;
    struct EvalResult result = unpack_args(gc, "d", NIL(gc), &d);

    ASSERT_TRUE(result.is_error, {
            fprintf(stderr, "Unpack did not fail\n");
    });

    destroy_gc(gc);

    return 0;
}

TEST_SUITE(interpreter_suite)
{
    TEST_RUN(equal_test);
    TEST_RUN(assoc_test);
    TEST_RUN(unpack_args_test);
    TEST_RUN(unpack_args_empty_list_test);

    return 0;
}

#endif  // INTERPRETER_SUITE_H_
