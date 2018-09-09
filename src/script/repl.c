#include <stdbool.h>

#include "parser.h"
#include "interpreter.h"
#include "scope.h"
#include "gc.h"

#define REPL_BUFFER_MAX 1024

char buffer[REPL_BUFFER_MAX + 1];

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    Gc *gc = create_gc();

    while (true) {
        printf("> ");

        if (fgets(buffer, REPL_BUFFER_MAX, stdin) == NULL) {
            return -1;
        }

        struct ParseResult parse_result = read_expr_from_string(gc, buffer);
        if (parse_result.is_error) {
            print_parse_error(stderr, buffer, parse_result);
            continue;
        }

        struct EvalResult eval_result = eval(gc, NIL(gc), parse_result.expr);
        if (eval_result.is_error) {
            print_eval_error(stderr, eval_result);
            destroy_expr_rec(parse_result.expr);
            destroy_expr_rec(eval_result.expr);
            continue;
        }

        print_expr_as_sexpr(eval_result.expr);
        destroy_expr_rec(parse_result.expr);
        destroy_expr_rec(eval_result.expr);
    }

    return 0;
}
