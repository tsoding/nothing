#ifndef ATOM_H_
#define ATOM_H_

#include <stdbool.h>

struct Cons;
struct Atom;

#define NUMBER(X) atom_as_expr(create_number_atom(X))
#define STRING(S) atom_as_expr(create_string_atom(S, NULL))
#define SYMBOL(S) atom_as_expr(create_symbol_atom(S, NULL))
#define CONS(CAR, CDR) cons_as_expr(create_cons(CAR, CDR))
#define NIL SYMBOL("nil")

enum ExprType
{
    EXPR_ATOM = 0,
    EXPR_CONS,
    EXPR_VOID
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

struct Expr atom_as_expr(struct Atom *atom);
struct Expr cons_as_expr(struct Cons *cons);
struct Expr void_expr(void);

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

struct Atom *create_number_atom(float num);
struct Atom *create_string_atom(const char *str, const char *str_end);
struct Atom *create_symbol_atom(const char *sym, const char *sym_end);
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
