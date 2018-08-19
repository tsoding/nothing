#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "script/expr.h"

struct Expr atom_as_expr(struct Atom *atom)
{
    struct Expr expr = {
        .type = EXPR_ATOM,
        .atom = atom
    };

    return expr;
}

struct Expr cons_as_expr(struct Cons *cons)
{
    struct Expr expr = {
        .type = EXPR_CONS,
        .cons = cons
    };

    return expr;
}

void print_atom_as_sexpr(struct Atom *atom)
{
    assert(atom);

    switch (atom->type) {
    case ATOM_SYMBOL:
        printf("%s", atom->sym);
        break;

    case ATOM_NUMBER:
        printf("%f", atom->num);
        break;

    case ATOM_STRING:
        printf("\"%s\"", atom->str);
        break;
    }
}

void print_cons_as_sexpr(struct Cons *cons)
{
    assert(cons);

    printf("(");
    print_expr_as_sexpr(cons->car);
    printf(" . ");
    print_expr_as_sexpr(cons->cdr);
    printf(")");
}

void print_expr_as_sexpr(struct Expr expr)
{
    switch (expr.type) {
    case EXPR_ATOM:
        print_atom_as_sexpr(expr.atom);
        break;

    case EXPR_CONS:
        print_cons_as_sexpr(expr.cons);
        break;
    }
}

struct Expr create_expr_from_str(const char *str)
{
    /* TODO(#283): create_expr_from_str is not implemented */
    assert(str);
    return atom_as_expr(0);
}

void destroy_expr(struct Expr expr)
{
    switch (expr.type) {
    case EXPR_ATOM:
        destroy_atom(expr.atom);
        break;

    case EXPR_CONS:
        destroy_cons(expr.cons);
        break;
    }
}

struct Cons *create_cons(struct Expr car, struct Expr cdr)
{
    struct Cons *cons = malloc(sizeof(struct Cons));
    if (cons == NULL) {
        return NULL;
    }

    cons->car = car;
    cons->cdr = cdr;

    return cons;
}

void destroy_cons(struct Cons *cons)
{
    destroy_expr(cons->car);
    destroy_expr(cons->cdr);
    free(cons);
}

struct Atom *create_atom(enum AtomType type, ...)
{
    struct Atom *atom = malloc(sizeof(struct Atom));
    if (atom == NULL) {
        return NULL;
    }

    va_list args;
    va_start(args, type);

    atom->type = type;

    switch (type) {
    case ATOM_SYMBOL:
    case ATOM_STRING: {
        const char *arg_str = va_arg(args, const char *);
        const size_t n = strlen(arg_str);
        char *str = malloc(sizeof(char) * (n + 1));

        if (str == NULL) {
            free(atom);
            return NULL;
        }

        memcpy(str, arg_str, n + 1);
        atom->str = str;
    } break;

    case ATOM_NUMBER: {
        atom->num = (float) va_arg(args, double);
    } break;
    }

    va_end(args);

    return atom;
}

void destroy_atom(struct Atom *atom)
{
    switch (atom->type) {
    case ATOM_SYMBOL:
    case ATOM_STRING: {
        free(atom->str);
    } break;

    case ATOM_NUMBER: {
        /* Nothing */
    } break;
    }

    free(atom);
}
