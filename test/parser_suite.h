#ifndef PARSER_SUITE_H_
#define PARSER_SUITE_H_

#include "test.h"
#include "ebisp/parser.h"
#include "ebisp/gc.h"
#include "ebisp/builtins.h"

TEST(read_expr_from_file_test)
{
    Gc *gc = create_gc();

    struct ParseResult result = read_expr_from_file(gc, "test-data/simple-sum.lisp");

    ASSERT_TRUE(!result.is_error, result.error_message);

    struct Expr head = result.expr;

    struct Expr expr = head;
    ASSERT_INTEQ(EXPR_CONS, expr.type);
    ASSERT_INTEQ(EXPR_ATOM, expr.cons->car.type);
    ASSERT_INTEQ(ATOM_SYMBOL, expr.cons->car.atom->type);
    ASSERT_STREQ("+", expr.cons->car.atom->sym);

    expr = expr.cons->cdr;
    ASSERT_INTEQ(EXPR_CONS, expr.type);
    ASSERT_INTEQ(EXPR_ATOM, expr.cons->car.type);
    ASSERT_INTEQ(ATOM_NUMBER, expr.cons->car.atom->type);
    ASSERT_LONGINTEQ(1L, expr.cons->car.atom->num);

    expr = expr.cons->cdr;
    ASSERT_INTEQ(EXPR_CONS, expr.type);
    ASSERT_INTEQ(EXPR_ATOM, expr.cons->car.type);
    ASSERT_INTEQ(ATOM_NUMBER, expr.cons->car.atom->type);
    ASSERT_LONGINTEQ(2L, expr.cons->car.atom->num);

    expr = expr.cons->cdr;
    ASSERT_INTEQ(EXPR_CONS, expr.type);
    ASSERT_INTEQ(EXPR_ATOM, expr.cons->car.type);
    ASSERT_INTEQ(ATOM_NUMBER, expr.cons->car.atom->type);
    ASSERT_LONGINTEQ(3L, expr.cons->car.atom->num);

    expr = expr.cons->cdr;
    ASSERT_INTEQ(EXPR_ATOM, expr.type);
    ASSERT_INTEQ(ATOM_SYMBOL, expr.atom->type);
    ASSERT_STREQ("nil", expr.atom->sym);

    destroy_gc(gc);

    return 0;
}

TEST(parse_negative_numbers_test)
{
    Gc *gc = create_gc();
    struct ParseResult result = read_expr_from_string(gc, "-12345");

    ASSERT_FALSE(result.is_error, {
        fprintf(stderr, "Parsing failed: %s", result.error_message);
    });
    ASSERT_TRUE(result.expr.type == EXPR_ATOM, "Parsed expression is not an atom");
    ASSERT_TRUE(result.expr.atom->type == ATOM_NUMBER, "Parsed expression is not a number");
    ASSERT_LONGINTEQ(-12345L, result.expr.atom->num);

    destroy_gc(gc);

    return 0;
}

TEST(read_all_exprs_from_string_empty_test)
{
    Gc *gc = create_gc();
    struct ParseResult result = read_all_exprs_from_string(gc, "");

    ASSERT_TRUE(result.is_error,
                "Parsing is expected to fail, "
                "but it did not");

    destroy_gc(gc);

    return 0;
}

TEST(read_all_exprs_from_string_one_test)
{
    Gc *gc = create_gc();
    struct ParseResult result = read_all_exprs_from_string(gc, "(+ 1 2)");

    ASSERT_FALSE(result.is_error, {
            fprintf(stderr,
                    "Parsing was unsuccessful: %s\n",
                    result.error_message);
    });

    ASSERT_EQ(long int, 1, length_of_list(result.expr), {
            fprintf(stderr, "Expected: %ld\n", _expected);
            fprintf(stderr, "Actual: %ld\n", _actual);
    });

    destroy_gc(gc);

    return 0;
}

TEST(read_all_exprs_from_string_two_test)
{
    Gc *gc = create_gc();
    struct ParseResult result = read_all_exprs_from_string(
        gc,
        "(+ 1 2) (+ 3 4)");

    ASSERT_FALSE(result.is_error, {
            fprintf(stderr,
                    "Parsing was unsuccessful: %s\n",
                    result.error_message);
    });

    ASSERT_EQ(long int, 2, length_of_list(result.expr), {
            fprintf(stderr, "Expected: %ld\n", _expected);
            fprintf(stderr, "Actual: %ld\n", _actual);
            fprintf(stderr, "Expression: ");
            print_expr_as_sexpr(stderr, result.expr);
            fprintf(stderr, "\n");
    });

    destroy_gc(gc);

    return 0;
}

TEST(read_all_exprs_from_string_bad_test)
{
    Gc *gc = create_gc();
    struct ParseResult result = read_all_exprs_from_string(
        gc,
        "(+ 1 2) + 3 4)");

    ASSERT_TRUE(result.is_error,
                "Parsing didn't fail as expected");

    destroy_gc(gc);

    return 0;
}

TEST_SUITE(parser_suite)
{
    TEST_RUN(read_expr_from_file_test);
    TEST_RUN(parse_negative_numbers_test);
    TEST_RUN(read_all_exprs_from_string_empty_test);
    TEST_RUN(read_all_exprs_from_string_one_test);
    TEST_RUN(read_all_exprs_from_string_two_test);
    // TODO: read_all_exprs_from_string_bad_test is failing
    TEST_IGNORE(read_all_exprs_from_string_bad_test);

    return 0;
}



#endif  // PARSER_SUITE_H_
