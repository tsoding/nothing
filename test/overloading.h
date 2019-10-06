#ifndef OVERLOADING_H_
#define OVERLOADING_H_

#include <stdio.h>
#include "ebisp/expr.h"

static
void print_float(float x, FILE *stream)
{
    fprintf(stream, "%f", x);
}

static
void print_int(int x, FILE *stream)
{
    fprintf(stream, "%d", x);
}

static
void print_expr_type(enum ExprType x, FILE *stream)
{
    fputs(expr_type_as_string(x), stream);
}

static
void print_atom_type(enum AtomType x, FILE *stream)
{
    fputs(atom_type_as_string(x), stream);
}

static
void print_expr(struct Expr expr, FILE *stream)
{
    print_expr_as_sexpr(stream, expr);
}

#define print(x, stream)                             \
    _Generic((x),                                    \
             float: print_float,                     \
             int: print_int,                         \
             enum ExprType: print_expr_type,         \
             enum AtomType: print_atom_type,         \
             const char *: fputs,                    \
             struct Expr: print_expr                 \
        )(x, stream)

static
int equal_floats(float x, float y)
{
    return fabsf(x - y) < 1e-6;
}

static
int equal_ints(long long int x, long long int y)
{
    return x == y;
}

#define EQUAL(x, y)                             \
    _Generic((x),                               \
             float: equal_floats,               \
             long long int: equal_ints,         \
             enum ExprType: equal_ints,         \
             enum AtomType: equal_ints,         \
             struct Expr: equal                 \
        )((x), (y))

#endif  // OVERLOADING_H_
