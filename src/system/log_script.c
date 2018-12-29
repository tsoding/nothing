#include "system/stacktrace.h"

#include "ebisp/gc.h"
#include "ebisp/scope.h"
#include "ebisp/interpreter.h"
#include "log_script.h"
#include "log.h"

static struct EvalResult
print(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    trace_assert(gc);
    trace_assert(scope);
    (void) param;

    const char *s = NULL;
    struct EvalResult result = match_list(gc, "s", args, &s);
    if (result.is_error) {
        return result;
    }

    log_info("%s\n", s);

    return eval_success(NIL(gc));
}

void load_log_library(Gc *gc, struct Scope *scope)
{
    set_scope_value(gc, scope, SYMBOL(gc, "print"), NATIVE(gc, print, NULL));
}
