#ifndef ATOM_H_
#define ATOM_H_

#include <stdbool.h>

struct Cons;
struct Atom;

enum ExprType
{
    EXPR_ATOM = 0,
    EXPR_CONS
};

// TODO(#285): there is no way to execute struct Expr
struct Expr
{
    enum ExprType type;
    union {
        struct Cons *cons;
        struct Atom *atom;
    };
};

struct ParseResult
{
    bool is_error;
    union {
        struct Expr expr;
        const char *error;
    };
};

struct ParseResult parse_success(struct Expr expr);
struct ParseResult parse_failure(const char *error);

struct Expr atom_as_expr(struct Atom *atom);
struct Expr cons_as_expr(struct Cons *cons);
struct ParseResult create_expr_from_str(const char *str,
                                        size_t *cursor,
                                        size_t n);
void destroy_expr(struct Expr expr);
void print_expr_as_sexpr(struct Expr expr);

enum AtomType
{
    ATOM_SYMBOL = 0,
    ATOM_NUMBER,
    ATOM_STRING,
};

struct Atom
{
    enum AtomType type;
    union
    {
        float num;             // ATOM_NUMBER
        char *sym;             // ATOM_SYMBOL
        char *str;             // ATOM_STRING
    };
};

struct Atom *create_atom(enum AtomType type, ...);
void destroy_atom(struct Atom *atom);
void print_atom_as_sexpr(struct Atom *atom);

struct Cons
{
    struct Expr car;
    struct Expr cdr;
};

struct Cons *create_cons(struct Expr car, struct Expr cdr);
void destroy_cons(struct Cons *cons);
void print_cons_as_sexpr(struct Cons *cons);

#endif  // ATOM_H_
