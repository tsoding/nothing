#ifndef EXPR_H_
#define EXPR_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Gc Gc;
typedef struct Scope Scope;

struct Cons;
struct Atom;

#define INTEGER(G, X) atom_as_expr(create_integer_atom(G, X))
#define REAL(G, X) atom_as_expr(create_real_atom(G, X))
#define STRING(G, S) atom_as_expr(create_string_atom(G, S, NULL))
#define SYMBOL(G, S) atom_as_expr(create_symbol_atom(G, S, NULL))
#define NATIVE(G, F, P) atom_as_expr(create_native_atom(G, F, P))
#define CONS(G, CAR, CDR) cons_as_expr(create_cons(G, CAR, CDR))
#define NIL(G) SYMBOL(G, "nil")
#define T(G) SYMBOL(G, "t")

#define CAR(O) ((O).cons->car)
#define CDR(O) ((O).cons->cdr)

enum ExprType
{
    EXPR_ATOM = 0,
    EXPR_CONS,
    EXPR_VOID
};

struct Expr
{
    enum ExprType type;
    union {
        struct Cons *cons;
        struct Atom *atom;
    };
};

// Prototype to prevent https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54113
const char *expr_type_as_string(enum ExprType expr_type);

struct Expr atom_as_expr(struct Atom *atom);
struct Expr cons_as_expr(struct Cons *cons);
struct Expr void_expr(void);

void destroy_expr(struct Expr expr);
void print_expr_as_sexpr(FILE *stream, struct Expr expr);
void print_expr_as_c(FILE *stream, struct Expr expr);
int expr_as_sexpr(struct Expr expr, char *output, size_t n);

// TODO(#337): EvalResult does not belong to expr unit
struct EvalResult
{
    bool is_error;
    struct Expr expr;
};


typedef struct EvalResult (*NativeFunction)(void *param, Gc *gc, struct Scope *scope, struct Expr args);

struct Native
{
    NativeFunction fun;
    void *param;
};

struct Lambda
{
    struct Expr args_list;
    struct Expr body;
    struct Expr envir;
};

enum AtomType
{
    ATOM_SYMBOL = 0,
    ATOM_INTEGER,
    ATOM_REAL,
    ATOM_STRING,
    ATOM_LAMBDA,
    ATOM_NATIVE
};

const char *atom_type_as_string(enum AtomType atom_type);

struct Atom
{
    enum AtomType type;
    union
    {
        // TODO(#330): Atom doesn't support floats
        long int num;           // ATOM_INTEGER
        float real;             // ATOM_REAL
        char *sym;              // ATOM_SYMBOL
        char *str;              // ATOM_STRING
        struct Lambda lambda;   // ATOM_LAMBDA
        struct Native native;   // ATOM_NATIVE
    };
};

struct Atom *create_integer_atom(Gc *gc, long int num);
struct Atom *create_real_atom(Gc *gc, float num);
struct Atom *create_string_atom(Gc *gc, const char *str, const char *str_end);
struct Atom *create_symbol_atom(Gc *gc, const char *sym, const char *sym_end);
struct Atom *create_lambda_atom(Gc *gc, struct Expr args_list, struct Expr body, struct Expr envir);
struct Atom *create_native_atom(Gc *gc, NativeFunction fun, void *param);
void destroy_atom(struct Atom *atom);
void print_atom_as_sexpr(FILE *stream, struct Atom *atom);

struct Cons
{
    struct Expr car;
    struct Expr cdr;
};

struct Cons *create_cons(Gc *gc, struct Expr car, struct Expr cdr);
void destroy_cons(struct Cons *cons);
void print_cons_as_sexpr(FILE *stream, struct Cons *cons);

#endif  // EXPR_H_
