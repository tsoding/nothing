#include "system/stacktrace.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ebisp/expr.h"
#include "ebisp/gc.h"
#include "system/str.h"

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

void print_atom_as_sexpr(FILE *stream, struct Atom *atom)
{
    trace_assert(atom);

    switch (atom->type) {
    case ATOM_SYMBOL: {
        fprintf(stream, "%s", atom->sym);
    } break;

    case ATOM_INTEGER: {
        fprintf(stream, "%ld", atom->num);
    } break;

    case ATOM_REAL: {
        fprintf(stream, "%f", atom->real);
    } break;

    case ATOM_STRING: {
        fprintf(stream, "\"%s\"", atom->str);
    } break;

    case ATOM_LAMBDA: {
        /* TODO(#649): Print LAMBDAs with arglists (and maybe bodies) in print_atom_as_sexpr and atom_as_sexpr */
        fprintf(stream, "<lambda>");
    } break;

    case ATOM_NATIVE: {
        fprintf(stream, "<native>");
    } break;
    }
}

static void print_atom_as_c(FILE *stream, struct Atom *atom)
{
    trace_assert(stream);
    trace_assert(atom);

    switch(atom->type) {
    case ATOM_SYMBOL: {
        fprintf(stream, "SYMBOL(gc, \"%s\")", atom->sym);
    } break;

    case ATOM_INTEGER: {
        fprintf(stream, "INTEGER(gc, %ld)", atom->num);
    } break;

    case ATOM_REAL: {
        fprintf(stream, "REAL(gc, %f)", atom->real);
    } break;

    case ATOM_STRING: {
        fprintf(stream, "STRING(gc, \"%s\")", atom->str);
    } break;

    case ATOM_LAMBDA: {
        fprintf(stream, "CONS(gc, SYMBOL(gc, \"lambda\"), CONS(gc, ");
        print_expr_as_c(stream, atom->lambda.args_list);
        fprintf(stream, ", CONS(gc, ");
        print_expr_as_c(stream, atom->lambda.body);
        fprintf(stream, ")))");
    } break;

    case ATOM_NATIVE: {
        fprintf(stream, "NIL(gc)");
    } break;
    }
}

void print_cons_as_sexpr(FILE *stream, struct Cons *head)
{
    trace_assert(head);

    struct Cons *cons = head;

    fprintf(stream, "(");
    print_expr_as_sexpr(stream, cons->car);

    while (cons->cdr.type == EXPR_CONS) {
        cons = cons->cdr.cons;
        fprintf(stream, " ");
        print_expr_as_sexpr(stream, cons->car);
    }

    if (cons->cdr.atom->type != ATOM_SYMBOL ||
        strcmp("nil", cons->cdr.atom->sym) != 0) {
        fprintf(stream, " . ");
        print_expr_as_sexpr(stream, cons->cdr);
    }

    fprintf(stream, ")");
}

static void print_cons_as_c(FILE *stream, struct Cons *cons)
{
    trace_assert(stream);
    trace_assert(cons);

    fprintf(stream, "CONS(gc, ");
    print_expr_as_c(stream, cons->car);
    fprintf(stream, ", ");
    print_expr_as_c(stream, cons->cdr);
    fprintf(stream, ")");
}

void print_expr_as_sexpr(FILE *stream, struct Expr expr)
{
    switch (expr.type) {
    case EXPR_ATOM:
        print_atom_as_sexpr(stream, expr.atom);
        break;

    case EXPR_CONS:
        print_cons_as_sexpr(stream, expr.cons);
        break;

    case EXPR_VOID:
        break;
    }
}

void print_expr_as_c(FILE *stream, struct Expr expr)
{
    trace_assert(stream);
    (void) expr;

    switch (expr.type) {
    case EXPR_ATOM:
        print_atom_as_c(stream, expr.atom);
        break;

    case EXPR_CONS:
        print_cons_as_c(stream, expr.cons);
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

struct Atom *create_real_atom(Gc *gc, float real)
{
    struct Atom *atom = malloc(sizeof(struct Atom));
    if (atom == NULL) {
        return NULL;
    }
    atom->type = ATOM_REAL;
    atom->real = real;

    if (gc_add_expr(gc, atom_as_expr(atom)) < 0) {
        free(atom);
        return NULL;
    }

    return atom;
}

struct Atom *create_integer_atom(Gc *gc, long int num)
{
    struct Atom *atom = malloc(sizeof(struct Atom));
    if (atom == NULL) {
        return NULL;
    }
    atom->type = ATOM_INTEGER;
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

struct Atom *create_lambda_atom(Gc *gc, struct Expr args_list, struct Expr body, struct Expr envir)
{
    struct Atom *atom = malloc(sizeof(struct Atom));

    if (atom == NULL) {
        goto error;
    }

    atom->type = ATOM_LAMBDA;
    atom->lambda.args_list = args_list;
    atom->lambda.body = body;
    atom->lambda.envir = envir;

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

struct Atom *create_native_atom(Gc *gc, NativeFunction fun, void *param)
{
    struct Atom *atom = malloc(sizeof(struct Atom));

    if (atom == NULL) {
        goto error;
    }

    atom->type = ATOM_NATIVE;
    atom->native.fun = fun;
    atom->native.param = param;

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

    case ATOM_LAMBDA:
    case ATOM_NATIVE:
    case ATOM_INTEGER:
    case ATOM_REAL: {
        /* Nothing */
    } break;
    }

    free(atom);
}

static int atom_as_sexpr(struct Atom *atom, char *output, size_t n)
{
    trace_assert(atom);
    trace_assert(output);

    switch (atom->type) {
    case ATOM_SYMBOL: {
        return snprintf(output, n, "%s", atom->sym);
    }

    case ATOM_INTEGER: {
        return snprintf(output, n, "%ld", atom->num);
    }

    case ATOM_REAL: {
        return snprintf(output, n, "%f", atom->real);
    }

    case ATOM_STRING: {
        return snprintf(output, n, "\"%s\"", atom->str);
    }

    case ATOM_LAMBDA:
        return snprintf(output, n, "<lambda>");

    case ATOM_NATIVE:
        return snprintf(output, n, "<native>");
    }

    return 0;
}

static int cons_as_sexpr(struct Cons *head, char *output, size_t n)
{
    trace_assert(head);
    trace_assert(output);

    /* TODO(#378): cons_as_sexpr does not handle encoding errors of snprintf */

    struct Cons *cons = head;

    int m = (int) n;

    int c = snprintf(output, n, "(");
    if (m - c <= c) {
        return c;
    }

    c += expr_as_sexpr(cons->car, output + c, (size_t) (m - c));
    if (m - c <= 0) {
        return c;
    }

    while (cons->cdr.type == EXPR_CONS) {
        cons = cons->cdr.cons;

        c += snprintf(output + c, (size_t) (m - c), " ");
        if (m - c <= 0) {
            return c;
        }

        c += expr_as_sexpr(cons->car, output + c, (size_t) (m - c));
        if (m - c <= 0) {
            return c;
        }
    }

    if (cons->cdr.atom->type != ATOM_SYMBOL ||
        strcmp("nil", cons->cdr.atom->sym) != 0) {

        c += snprintf(output + c, (size_t) (m - c), " . ");
        if (m - c <= 0) {
            return c;
        }

        c += expr_as_sexpr(cons->cdr, output + c, (size_t) (m - c));
        if (m - c <= 0) {
            return c;
        }
    }

    c += snprintf(output + c, (size_t) (m - c), ")");
    if (m - c <= 0) {
        return c;
    }

    return c;
}

int expr_as_sexpr(struct Expr expr, char *output, size_t n)
{
    switch(expr.type) {
    case EXPR_ATOM:
        return atom_as_sexpr(expr.atom, output, n);

    case EXPR_CONS:
        return cons_as_sexpr(expr.cons, output, n);

    case EXPR_VOID:
        return 0;
    }

    return 0;
}

const char *expr_type_as_string(enum ExprType expr_type)
{
    switch (expr_type) {
    case EXPR_ATOM: return "EXPR_ATOM";
    case EXPR_CONS: return "EXPR_CONS";
    case EXPR_VOID: return "EXPR_VOID";
    }

    return "";
}

const char *atom_type_as_string(enum AtomType atom_type)
{
    switch (atom_type) {
    case ATOM_SYMBOL: return "ATOM_SYMBOL";
    case ATOM_INTEGER: return "ATOM_INTEGER";
    case ATOM_REAL: return "ATOM_REAL";
    case ATOM_STRING: return "ATOM_STRING";
    case ATOM_LAMBDA: return "ATOM_LAMBDA";
    case ATOM_NATIVE: return "ATOM_NATIVE";
    }

    return "";
}
