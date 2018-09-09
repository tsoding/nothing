#ifndef ATOM_H_
#define ATOM_H_

#include <stdlib.h>
#include <stdbool.h>

typedef struct Gc Gc;

struct Cons;
struct Atom;

#define NUMBER(G, X) atom_as_expr(create_number_atom(G, X))
#define STRING(G, S) atom_as_expr(create_string_atom(G, S, NULL))
#define SYMBOL(G, S) atom_as_expr(create_symbol_atom(G, S, NULL))
#define CONS(G, CAR, CDR) cons_as_expr(create_cons(G, CAR, CDR))
#define NIL(G) SYMBOL(G, "nil")

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
struct Expr clone_expr(Gc *gc, struct Expr expr);

void destroy_expr_rec(struct Expr expr);
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

struct Atom *create_number_atom(Gc *gc, float num);
struct Atom *create_string_atom(Gc *gc, const char *str, const char *str_end);
struct Atom *create_symbol_atom(Gc *gc, const char *sym, const char *sym_end);
void destroy_atom(struct Atom *atom);
void print_atom_as_sexpr(struct Atom *atom);

struct Cons
{
    struct Expr car;
    struct Expr cdr;
};

struct Cons *create_cons(Gc *gc, struct Expr car, struct Expr cdr);
void destroy_cons(struct Cons *cons);
void destroy_cons_rec(struct Cons *cons);
void print_cons_as_sexpr(struct Cons *cons);

#endif  // ATOM_H_
