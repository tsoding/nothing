#include <assert.h>
#include <stdbool.h>

#include "parser.h"
#include "interpreter.h"
#include "scope.h"
#include "gc.h"

#define REPL_BUFFER_MAX 1024

static struct EvalResult quit(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    assert(scope);
    (void) args;
    (void) param;

    exit(0);

    return eval_success(NIL(gc));
}

static void eval_line(Gc *gc, Scope *scope, const char *line)
{
    const char *read_iter = line;
    while (*read_iter != 0) {
        printf("Before parse:\t");
        gc_inspect(gc);

        struct ParseResult parse_result = read_expr_from_string(gc, read_iter);
        if (parse_result.is_error) {
            print_parse_error(stderr, line, parse_result);
            return;
        }
        printf("After parse:\t");
        gc_inspect(gc);

        struct EvalResult eval_result = eval(gc, scope, parse_result.expr);
        printf("After eval:\t");
        gc_inspect(gc);

        gc_collect(gc, CONS(gc, scope->expr, eval_result.expr));
        printf("After collect:\t");
        gc_inspect(gc);

        printf("Scope:\t");
        print_expr_as_sexpr(scope->expr);
        printf("\n");

        if (eval_result.is_error) {
            printf("Error:\t");
        }

        print_expr_as_sexpr(eval_result.expr);
        printf("\n");

        read_iter = next_token(parse_result.end).begin;
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
