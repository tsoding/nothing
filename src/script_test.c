#include <stdio.h>
#include <string.h>

#include "script/parser.h"

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    const char *code = "(1 . (2 . 3))";
    size_t cursor = 0;
    const size_t n = strlen(code);

    struct ParseResult result = create_expr_from_str(code, &cursor, n);

    if (result.is_error) {
        print_parse_error(stderr, code, result);
    } else {
        print_expr_as_sexpr(result.expr);
    }

    return 0;
}
