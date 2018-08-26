#include <stdio.h>
#include <string.h>

#include "script/parser.h"
#include "script/tokenizer.h"

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    const char *code = "(1 . (\"2\" . (hello-world . (\"\" .nil))))";

    struct ParseResult result = parse_expr(next_token(code));

    if (result.is_error) {
        print_parse_error(stderr, code, result);
    } else {
        print_expr_as_sexpr(result.expr);
    }

    return 0;
}
