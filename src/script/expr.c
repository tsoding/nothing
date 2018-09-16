#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "script/expr.h"
#include "script/gc.h"

static char *string_duplicate(const char *str,
                              const char *str_end)
{
    if (str_end != NULL && str > str_end) {
        return NULL;
    }

    const size_t n = str_end == NULL ? strlen(str) : (size_t) (str_end - str);
    char *dup_str = malloc(sizeof(char) * (n + 1));
    if (dup_str == NULL) {
        return NULL;
    }

    memcpy(dup_str, str, n);
    dup_str[n] = '\0';

    return dup_str;
}

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

struct Expr void_expr(void)
{
    struct Expr expr = {
        .type = EXPR_VOID
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
        printf("%ld", atom->num);
        break;

    case ATOM_STRING:
        printf("\"%s\"", atom->str);
        break;

    case ATOM_NATIVE:
        printf("<native>");
        break;
    }
}

void print_cons_as_sexpr(struct Cons *head)
{
    assert(head);

    struct Cons *cons = head;

    printf("(");
    print_expr_as_sexpr(cons->car);

    while (cons->cdr.type == EXPR_CONS) {
        cons = cons->cdr.cons;
        printf(" ");
        print_expr_as_sexpr(cons->car);
    }

    if (cons->cdr.atom->type != ATOM_SYMBOL ||
        strcmp("nil", cons->cdr.atom->sym) != 0) {
        printf(" . ");
        print_expr_as_sexpr(cons->cdr);
    }

    printf(")");
}

void print_expr_as_sexpr(struct Expr expr)
{
    /* TODO(#296): print_expr_as_sexpr doesn't support lists */
    switch (expr.type) {
    case EXPR_ATOM:
        print_atom_as_sexpr(expr.atom);
        break;

    case EXPR_CONS:
        print_cons_as_sexpr(expr.cons);
        break;

    case EXPR_VOID:
        break;
    }
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

    case EXPR_VOID:
        break;
    }
}

struct Cons *create_cons(Gc *gc, struct Expr car, struct Expr cdr)
{
    struct Cons *cons = malloc(sizeof(struct Cons));
    if (cons == NULL) {
        return NULL;
    }

    cons->car = car;
    cons->cdr = cdr;

    if (gc_add_expr(gc, cons_as_expr(cons)) < 0) {
        free(cons);
        return NULL;
    }

    return cons;
}

void destroy_cons(struct Cons *cons)
{
    free(cons);
}

struct Atom *create_number_atom(Gc *gc, long int num)
{
    struct Atom *atom = malloc(sizeof(struct Atom));
    if (atom == NULL) {
        return NULL;
    }
    atom->type = ATOM_NUMBER;
    atom->num = num;

    if (gc_add_expr(gc, atom_as_expr(atom)) < 0) {
        free(atom);
        return NULL;
    }

    return atom;
}

struct Atom *create_string_atom(Gc *gc, const char *str, const char *str_end)
{
    struct Atom *atom = malloc(sizeof(struct Atom));

    if (atom == NULL) {
        goto error;
    }

    atom->type = ATOM_STRING;
    atom->str = string_duplicate(str, str_end);

    if (atom->str == NULL) {
        goto error;
    }

    if (gc_add_expr(gc, atom_as_expr(atom)) < 0) {
        goto error;
    }

    return atom;

error:
    if (atom != NULL) {
        if (atom->str != NULL) {
            free(atom->str);
        }
        free(atom);
    }

    return NULL;
}

struct Atom *create_symbol_atom(Gc *gc, const char *sym, const char *sym_end)
{
    struct Atom *atom = malloc(sizeof(struct Atom));

    if (atom == NULL) {
        goto error;
    }

    atom->type = ATOM_SYMBOL;
    atom->sym = string_duplicate(sym, sym_end);

    if (atom->sym == NULL) {
        goto error;
    }

    if (gc_add_expr(gc, atom_as_expr(atom)) < 0) {
        goto error;
    }

    return atom;

error:
    if (atom != NULL) {
        if (atom->sym != NULL) {
            free(atom->sym);
        }
        free(atom);
    }

    return NULL;
}

struct Atom *create_native_atom(Gc *gc, NativeFunction fun)
{
    struct Atom *atom = malloc(sizeof(struct Atom));

    if (atom == NULL) {
        goto error;
    }

    atom->type = ATOM_NATIVE;
    atom->fun = fun;

    if (gc_add_expr(gc, atom_as_expr(atom)) < 0) {
        goto error;
    }

    return atom;

error:
    if (atom != NULL) {
        free(atom);
    }

    return NULL;
}

void destroy_atom(struct Atom *atom)
{
    switch (atom->type) {
    case ATOM_SYMBOL:
    case ATOM_STRING: {
        free(atom->str);
    } break;

    case ATOM_NATIVE:
    case ATOM_NUMBER: {
        /* Nothing */
    } break;
    }

    free(atom);
}
