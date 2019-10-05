#include "system/stacktrace.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "builtins.h"

static bool equal_atoms(struct Atom *atom1, struct Atom *atom2)
{
    trace_assert(atom1);
    trace_assert(atom2);

    if (atom1->type != atom2->type) {
        return false;
    }

    switch (atom1->type) {
    case ATOM_SYMBOL: {
        return strcmp(atom1->sym, atom2->sym) == 0;
    }

    case ATOM_INTEGER: {
        return atom1->num == atom2->num;
    }

    case ATOM_REAL: {
        return atom1->real == atom2->real;
    }

    case ATOM_STRING: {
        return strcmp(atom1->str, atom2->str) == 0;
    }

    case ATOM_LAMBDA: {
        return atom1 == atom2;
    }

    case ATOM_NATIVE: {
        return atom1->native.fun == atom2->native.fun
            && atom1->native.param == atom2->native.param;
    }
    }

    return false;
}

static bool equal_cons(struct Cons *cons1, struct Cons *cons2)
{
    trace_assert(cons1);
    trace_assert(cons2);
    return equal(cons1->car, cons2->car) && equal(cons1->cdr, cons2->cdr);
}

bool equal(struct Expr obj1, struct Expr obj2)
{
    if (obj1.type != obj2.type) {
        return false;
    }

    switch (obj1.type) {
    case EXPR_ATOM:
        return equal_atoms(obj1.atom, obj2.atom);

    case EXPR_CONS:
        return equal_cons(obj1.cons, obj2.cons);

    case EXPR_VOID:
        return true;
    }

    return true;
}

bool nil_p(struct Expr obj)
{
    return symbol_p(obj)
        && strcmp(obj.atom->sym, "nil") == 0;
}


bool symbol_p(struct Expr obj)
{
    return obj.type == EXPR_ATOM
        && obj.atom->type == ATOM_SYMBOL;
}

bool integer_p(struct Expr obj)
{
    return obj.type == EXPR_ATOM
        && obj.atom->type == ATOM_INTEGER;
}

bool string_p(struct Expr obj)
{
    return obj.type == EXPR_ATOM
        && obj.atom->type == ATOM_STRING;
}

bool cons_p(struct Expr obj)
{
    return obj.type == EXPR_CONS;
}

bool list_p(struct Expr obj)
{
    if (nil_p(obj)) {
        return true;
    }

    if (obj.type == EXPR_CONS) {
        return list_p(obj.cons->cdr);
    }

    return false;
}

bool list_of_symbols_p(struct Expr obj)
{
    if (nil_p(obj)) {
        return true;
    }

    if (obj.type == EXPR_CONS && symbol_p(obj.cons->car)) {
        return list_of_symbols_p(obj.cons->cdr);
    }

    return false;
}

bool lambda_p(struct Expr obj)
{
    return obj.type == EXPR_ATOM
        && obj.atom->type == ATOM_LAMBDA;
}

long int length_of_list(struct Expr obj)
{
    long int count = 0;

    while (!nil_p(obj)) {
        count++;
        obj = obj.cons->cdr;
    }

    return count;
}

struct Expr assoc(struct Expr key, struct Expr alist)
{
    while (cons_p(alist)) {
        if (cons_p(CAR(alist)) && equal(CAR(CAR(alist)), key)) {
            return CAR(alist);
        }

        alist = CDR(alist);
    }

    return alist;
}

const char *specials[] = {
    "set", "quote", "begin",
    "defun", "lambda", "λ",
    "when", "quasiquote"
};

bool is_special(const char *name)
{
    trace_assert(name);

    size_t n = sizeof(specials) / sizeof(const char*);
    for (size_t i = 0; i < n; ++i) {
        if (strcmp(name, specials[i]) == 0) {
            return true;
        }
    }

    return false;
}


static struct Expr
list_rec(Gc *gc, const char *format, va_list args)
{
    trace_assert(gc);
    trace_assert(format);

    if (*format == 0) {
        return NIL(gc);
    }

    switch (*format) {
    case 'd': {
        long int p = va_arg(args, long int);
        return CONS(gc, INTEGER(gc, p),
                    list_rec(gc, format + 1, args));
    }

    case 's': {
        const char* p = va_arg(args, const char*);
        return CONS(gc, STRING(gc, p),
                    list_rec(gc, format + 1, args));
    }

    case 'q': {
        const char* p = va_arg(args, const char*);
        return CONS(gc, SYMBOL(gc, p),
                    list_rec(gc, format + 1, args));
    }

    case 'e': {
        struct Expr p = va_arg(args, struct Expr);
        return CONS(gc, p, list_rec(gc, format + 1, args));
    }

    default: {
        fprintf(stderr, "Wrong format parameter: %c\n", *format);
        trace_assert(0);
    }
    }

    return NIL(gc);
}

struct Expr
list(Gc *gc, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    struct Expr result = list_rec(gc, format, args);
    va_end(args);

    return result;
}

struct Expr bool_as_expr(Gc *gc, bool condition)
{
    return condition ? T(gc) : NIL(gc);
}
