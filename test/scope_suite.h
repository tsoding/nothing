#ifndef SCOPE_SUITE_H_
#define SCOPE_SUITE_H_

#include "test.h"
#include "script/scope.h"
#include "script/expr.h"

TEST(set_scope_value_test)
{
    Gc *gc = create_gc();

    struct Expr z = SYMBOL(gc, "z");
    struct Expr x = SYMBOL(gc, "x");
    struct Expr y = SYMBOL(gc, "y");

    struct Expr initial_scope =
        list(gc, 2,
             list(gc, 2,
                  CONS(gc, x, STRING(gc, "hello")),
                  CONS(gc, y, STRING(gc, "world"))),
             NIL(gc));

    struct Expr expected_scope =
        list(gc, 2,
             list(gc, 2,
                  CONS(gc, x, STRING(gc, "hello")),
                  CONS(gc, y, STRING(gc, "world"))),
             list(gc, 1,
                  CONS(gc, z, STRING(gc, "foo"))));

    struct Expr actual_scope = set_scope_value(gc, initial_scope, z, STRING(gc, "foo"));

    ASSERT_TRUE(equal(expected_scope, actual_scope), "Unexpected scope");

    destroy_gc(gc);

    return 0;
}

TEST_SUITE(scope_suite)
{
    TEST_RUN(set_scope_value_test);

    return 0;
}

#endif  // SCOPE_SUITE_H_
