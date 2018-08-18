#ifndef ATOM_H_
#define ATOM_H_

struct Cons;
struct Atom;

enum ExprType
{
    EXPR_ATOM = 0,
    EXPR_CONS
};

// TODO: there is no way to execute struct Expr
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
struct Expr create_expr_from_str(const char *str);
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
