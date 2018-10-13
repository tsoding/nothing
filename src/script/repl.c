#include <assert.h>
#include <stdbool.h>

#include "parser.h"
#include "interpreter.h"
#include "scope.h"
#include "gc.h"

#define REPL_BUFFER_MAX 1024

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

static void eval_line(Gc *gc, Scope *scope, const char *line)
{
    while (*line != 0) {
        gc_collect(gc, scope->expr);

        struct ParseResult parse_result = read_expr_from_string(gc, line);
        if (parse_result.is_error) {
            print_parse_error(stderr, line, parse_result);
            return;
        }

        struct EvalResult eval_result = eval(gc, scope, parse_result.expr);
        if (eval_result.is_error) {
            fprintf(stderr, "Error:\t");
            print_expr_as_sexpr(stderr, eval_result.expr);
            fprintf(stderr, "\n");
            continue;
        }

        print_expr_as_sexpr(stderr, eval_result.expr);
        fprintf(stdout, "\n");

        line = next_token(parse_result.end).begin;
    }
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    char buffer[REPL_BUFFER_MAX + 1];

    Gc *gc = create_gc();
    struct Scope scope = {
        .expr = CONS(gc, NIL(gc), NIL(gc))
    };

    set_scope_value(gc, &scope, SYMBOL(gc, "quit"), NATIVE(gc, quit, NULL));
    set_scope_value(gc, &scope, SYMBOL(gc, "gc-inspect"), NATIVE(gc, gc_inspect_adapter, NULL));
    set_scope_value(gc, &scope, SYMBOL(gc, "scope"), NATIVE(gc, get_scope, NULL));

    while (true) {
        printf("> ");

        if (fgets(buffer, REPL_BUFFER_MAX, stdin) == NULL) {
            return -1;
        }

        eval_line(gc, &scope, buffer);
    }

    destroy_gc(gc);

    return 0;
}
