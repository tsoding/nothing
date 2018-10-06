#ifndef PARSER_SUITE_H_
#define PARSER_SUITE_H_

#include "test.h"
#include "script/parser.h"
#include "script/gc.h"

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

    ASSERT_FALSE(result.is_error, "Parsing failed");
    ASSERT_TRUE(result.expr.type == EXPR_ATOM, "Parsed expression is not an atom");
    ASSERT_TRUE(result.expr.atom->type == ATOM_NUMBER, "Parsed expression is not a number");
    ASSERT_LONGINTEQ(-12345L, result.expr.atom->num);

    destroy_gc(gc);

    return 0;
}

TEST_SUITE(parser_suite)
{
    TEST_RUN(read_expr_from_file_test);
    TEST_RUN(parse_negative_numbers_test);

    return 0;
}

#endif  // PARSER_SUITE_H_
