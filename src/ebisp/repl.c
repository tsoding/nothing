#include "system/stacktrace.h"
#include <stdbool.h>

#include "gc.h"
#include "interpreter.h"
#include "parser.h"
#include "repl_runtime.h"
#include "scope.h"
#include "std.h"

#define REPL_BUFFER_MAX 1024

static void eval_line(Gc *gc, Scope *scope, const char *line)
{
    /* TODO(#465): eval_line could be implemented with read_all_exprs_from_string */
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
            return;
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
    struct Scope scope = create_scope(gc);

    load_std_library(gc, &scope);
    load_repl_runtime(gc, &scope);

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
