#include <assert.h>

#include "scope.h"
#include "interpreter.h"
#include "gc.h"
#include "expr.h"
#include "repl_runtime.h"

static struct EvalResult gc_inspect_adapter(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    assert(gc);
    assert(scope);
    (void) param;
    (void) args;

    gc_inspect(gc);

    return eval_success(NIL(gc));
}

static struct EvalResult quit(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    assert(gc);
    assert(scope);
    (void) args;
    (void) param;

    exit(0);

    return eval_success(NIL(gc));
}

static struct EvalResult get_scope(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    assert(gc);
    assert(scope);
    (void) param;
    (void) args;

    return eval_success(scope->expr);
}

static struct EvalResult print(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    assert(gc);
    assert(scope);
    (void) param;

    const char *s = NULL;
    struct EvalResult result = match_list(gc, "s", args, &s);
    if (result.is_error) {
        return result;
    }

    printf("%s\n", s);

    return eval_success(NIL(gc));
}

void load_repl_runtime(Gc *gc, struct Scope *scope)
{
    set_scope_value(gc, scope, SYMBOL(gc, "quit"), NATIVE(gc, quit, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "gc-inspect"), NATIVE(gc, gc_inspect_adapter, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "scope"), NATIVE(gc, get_scope, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "print"), NATIVE(gc, print, NULL));
}
