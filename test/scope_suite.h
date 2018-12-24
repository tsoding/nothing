#ifndef SCOPE_SUITE_H_
#define SCOPE_SUITE_H_

#include "test.h"
#include "ebisp/scope.h"
#include "ebisp/expr.h"

TEST(set_scope_value_test)
{
    Gc *gc = create_gc();

    struct Expr z = SYMBOL(gc, "z");
    struct Expr x = SYMBOL(gc, "x");
    struct Expr y = SYMBOL(gc, "y");

    struct Scope scope = {
        .expr = CONS(gc, NIL(gc), NIL(gc))
    };

    push_scope_frame(gc, &scope,
                     list(gc, "ee", x, y),
                     list(gc, "ss", "hello", "world"));

    set_scope_value(gc, &scope, z, STRING(gc, "foo"));

    ASSERT_TRUE(equal(CONS(gc, x, STRING(gc, "hello")), get_scope_value(&scope, x)),
                { fprintf(stderr, "Unexpected value of `x`\n"); });
    ASSERT_TRUE(equal(CONS(gc, y, STRING(gc, "world")), get_scope_value(&scope, y)),
                { fprintf(stderr, "Unexpected value of `y`\n"); });
    ASSERT_TRUE(equal(CONS(gc, z, STRING(gc, "foo")), get_scope_value(&scope, z)),
                { fprintf(stderr, "Unexpected value of `z`\n"); });

    pop_scope_frame(gc, &scope);

    ASSERT_TRUE(equal(NIL(gc), get_scope_value(&scope, x)),
                { fprintf(stderr, "Unexpected value of `x`\n"); });
    ASSERT_TRUE(equal(NIL(gc), get_scope_value(&scope, y)),
                { fprintf(stderr, "Unexpected value of `y`\n"); });
    ASSERT_TRUE(equal(CONS(gc, z, STRING(gc, "foo")), get_scope_value(&scope, z)),
                { fprintf(stderr, "Unexpected value of `z`\n"); });


    destroy_gc(gc);

    return 0;
}

TEST_SUITE(scope_suite)
{
    TEST_RUN(set_scope_value_test);

    return 0;
}

#endif  // SCOPE_SUITE_H_
