#include <assert.h>
#include <stdlib.h>

#include "expr.h"
#include "gc.h"
#include "system/error.h"
#include "system/lt.h"

#define GC_INITIAL_EXPRS_CAPACITY 100

struct Gc
{
    Lt *lt;
    struct Expr *exprs;
    size_t exprs_size;
    size_t exprs_capacity;
};

Gc *create_gc(void)
{
    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Gc *gc = PUSH_LT(lt, malloc(sizeof(Gc)), free);
    if (gc == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    gc->lt = lt;

    gc->exprs = PUSH_LT(lt, malloc(sizeof(struct Expr) * GC_INITIAL_EXPRS_CAPACITY), free);
    if (gc->exprs == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    gc->exprs_size = 0;
    gc->exprs_capacity = GC_INITIAL_EXPRS_CAPACITY;

    return gc;
}

void destroy_gc(Gc *gc)
{
    assert(gc);
    RETURN_LT0(gc->lt);
}

void gc_add_atom(Gc *gc, const struct Atom *atom)
{
    assert(gc);
    assert(atom);
}

void gc_add_cons(Gc *gc, const struct Cons *cons)
{
    assert(gc);
    assert(cons);
}

void gc_collect(Gc *gc, struct Expr root)
{
    assert(gc);
    (void) root;
}
