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
    ASSERT_FLOATEQ(1.0f, expr.cons->car.atom->num, 1e-3f);

    expr = expr.cons->cdr;
    ASSERT_INTEQ(EXPR_CONS, expr.type);
    ASSERT_INTEQ(EXPR_ATOM, expr.cons->car.type);
    ASSERT_INTEQ(ATOM_NUMBER, expr.cons->car.atom->type);
    ASSERT_FLOATEQ(2.0f, expr.cons->car.atom->num, 1e-3f);

    expr = expr.cons->cdr;
    ASSERT_INTEQ(EXPR_CONS, expr.type);
    ASSERT_INTEQ(EXPR_ATOM, expr.cons->car.type);
    ASSERT_INTEQ(ATOM_NUMBER, expr.cons->car.atom->type);
    ASSERT_FLOATEQ(3.0f, expr.cons->car.atom->num, 1e-3f);

    expr = expr.cons->cdr;
    ASSERT_INTEQ(EXPR_ATOM, expr.type);
    ASSERT_INTEQ(ATOM_SYMBOL, expr.atom->type);
    ASSERT_STREQ("nil", expr.atom->sym);

    destroy_gc(gc);

    return 0;
}

TEST_SUITE(parser_suite)
{
    TEST_RUN(read_expr_from_file_test);

    return 0;
}

#endif  // PARSER_SUITE_H_
