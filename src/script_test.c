#include <stdio.h>

#include "script/expr.h"

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    struct Expr expr = cons_as_expr(
        create_cons(
            atom_as_expr(create_atom(ATOM_NUMBER, 1.0)),
            cons_as_expr(
                create_cons(
                    atom_as_expr(create_atom(ATOM_SYMBOL, "hello")),
                    cons_as_expr(
                        create_cons(
                            atom_as_expr(create_atom(ATOM_STRING, "world")),
                            atom_as_expr(create_atom(ATOM_SYMBOL, "nil"))))))));

    print_expr_as_sexpr(expr);

    destroy_expr(expr);

    return 0;
}
