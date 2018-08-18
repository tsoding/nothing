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

        strncpy(str, arg_str, n);
        atom->text = str;
    } break;

    case ATOM_NUMBER: {
        atom->number = va_arg(args, int);
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
        free(atom->text);
    } break;

    case ATOM_NUMBER: {
        /* Nothing */
    } break;
    }

    free(atom);
}
