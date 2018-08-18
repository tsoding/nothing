#ifndef ATOM_H_
#define ATOM_H_

struct Cons;
struct Atom;

enum ExprType
{
    EXPR_ATOM = 0,
    EXPR_CONS
};

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
void destroy_expr(struct Expr expr);

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
        int number;             // ATOM_NUMBER
        char *name;             // ATOM_SYMBOL
        char *text;             // ATOM_STRING
    };
};

struct Atom *create_atom(enum AtomType type, ...);
void destroy_atom(struct Atom *atom);

struct Cons
{
    struct Expr car;
    struct Expr cdr;
};

struct Cons *create_cons(struct Expr car, struct Expr cdr);
void destroy_cons(struct Cons *cons);

#endif  // ATOM_H_
