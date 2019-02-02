#include <stdio.h>
#include "ebisp/parser.h"
#include "ebisp/expr.h"
#include "ebisp/gc.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Using: ./baker <file.lisp>");
        return -1;
    }

    const char *filename = argv[1];

    Gc *gc = create_gc();
    struct ParseResult result = read_all_exprs_from_file(gc, filename);
    if (result.is_error) {
        fprintf(stderr, "%s\n", result.error_message);
        return -1;
    }

    print_expr_as_c(stderr, result.expr);

    return 0;
}
