#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "builtins.h"

static bool equal_atoms(struct Atom *atom1, struct Atom *atom2)
{
    assert(atom1);
    assert(atom2);

    if (atom1->type != atom2->type) {
        return false;
    }

    switch (atom1->type) {
    case ATOM_SYMBOL:
        return strcmp(atom1->sym, atom2->sym) == 0;

    case ATOM_NUMBER:
        return atom1->num == atom2->num;

    case ATOM_STRING:
        return strcmp(atom1->str, atom2->str) == 0;

    case ATOM_NATIVE:
        return atom1->native.fun == atom2->native.fun
            && atom1->native.param == atom2->native.param;
    }

    return false;
}

static bool equal_cons(struct Cons *cons1, struct Cons *cons2)
{
    assert(cons1);
    assert(cons2);
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
    if (!list_p(obj)) {
        return false;
    }

    if (length_of_list(obj) < 2) {
        return false;
    }

    if (!symbol_p(obj.cons->car)) {
        return false;
    }

    if (strcmp("lambda", obj.cons->car.atom->sym) != 0) {
        return false;
    }

    if (!list_of_symbols_p(obj.cons->cdr.cons->car)) {
        return false;
    }

    return true;
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
        if (cons_p(alist.cons->car) && equal(alist.cons->car.cons->car, key)) {
            return alist.cons->car;
        }

        alist = alist.cons->cdr;
    }

    return alist;
}

static struct Expr list_rec(Gc *gc, size_t n, va_list args)
{
    if (n == 0) {
        return NIL(gc);
    }

    struct Expr obj = va_arg(args, struct Expr);
    return CONS(gc, obj, list_rec(gc, n - 1, args));
}

struct Expr list(Gc *gc, size_t n, ...)
{
    va_list args;
    va_start(args, n);
    struct Expr obj = list_rec(gc, n, args);
    va_end(args);
    return obj;
}
