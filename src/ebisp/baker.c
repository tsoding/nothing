#include <stdio.h>
#include "ebisp/parser.h"
#include "ebisp/expr.h"
#include "ebisp/gc.h"

int main(int argc, char *argv[])
{
    if (argc < 4) {
        fprintf(stderr, "Using: baker <file.lisp> <output.h> <function-name>");
        return -1;
    }

    const char *input = argv[1];
    const char *output = argv[2];
    const char *function_name = argv[3];

    Gc *gc = create_gc();
    struct ParseResult result = read_all_exprs_from_file(gc, input);
    if (result.is_error) {
        fprintf(stderr, "Parsing Error: %s\n", result.error_message);
        return -1;
    }

    FILE *output_file = fopen(output, "w+");
    if (output_file == NULL) {
        fprintf(stderr, "Printing Error: Could not open file %s\n", output);
        return -1;
    }

    fprintf(output_file, "static struct Expr %s(Gc *gc) {\n", function_name);
    fprintf(output_file, "    return ");
    print_expr_as_c(output_file, result.expr);
    fprintf(output_file, ";\n");
    fprintf(output_file, "}\n");

    fclose(output_file);

    return 0;
}
