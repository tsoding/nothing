#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "script/expr.h"

static struct Expr create_nil(void)
{
    return atom_as_expr(create_atom(ATOM_SYMBOL, "nil"));
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

struct ParseResult parse_success(struct Expr expr)
{
    struct ParseResult result = {
        .is_error = false,
        .expr = expr
    };

    return result;
}

struct ParseResult parse_failure(const char *error)
{
    struct ParseResult result = {
        .is_error = true,
        .error = error
    };

    return result;
}

static void skip_whitespaces(const char *str, size_t *cursor, size_t n)
{
    assert(str);
    assert(cursor);
    (void) n;

    /* TODO: skip_whitespaces is not implemented */
}

struct ParseResult create_expr_from_str(const char *str,
                                        size_t *cursor,
                                        size_t n)
{
    assert(str);
    assert(cursor);

    skip_whitespaces(str, cursor, n);
    if (*cursor >= n) {
        return parse_failure("EOF");
    }

    switch (str[*cursor]) {
    case '(': {
        (*cursor)++;
        struct ParseResult car = create_expr_from_str(str, cursor, n);
        if (car.is_error) {
            return car;
        }

        skip_whitespaces(str, cursor, n);
        if (*cursor >= n) {
            return parse_failure("EOF");
        }

        if (str[*cursor] != '.') {
            return parse_failure("Expected .");
        }
        (*cursor)++;

        skip_whitespaces(str, cursor, n);
        if (*cursor >= n) {
            return parse_failure("EOF");
        }

        struct ParseResult cdr = create_expr_from_str(str, cursor, n);
        if (cdr.is_error) {
            return cdr;
        }

        skip_whitespaces(str, cursor, n);
        if (*cursor >= n) {
            return parse_failure("EOF");
        }

        if (str[*cursor] != ')') {
            return parse_failure("Expected )");
        }

        (*cursor)++;

        return parse_success(cons_as_expr(create_cons(car.expr, cdr.expr)));
    }

    case '"': {
        /* TODO: create_expr_from_str does not support strings */
        return parse_failure("Strings are not supported");
    }

    default: {
        if (isdigit(str[*cursor])) {
            /* TODO: create_expr_from_str does not support numbers */
            return parse_failure("Numbers are not supported");
        } else if (isalpha(str[*cursor])) {
            /* TODO: create_expr_from_str does not support symbols */
            return parse_failure("Symbols are not supported");
        } else {
            return parse_failure("Unexpected sequence of characters");
        }
    }
    }

    return parse_success(create_nil());
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
