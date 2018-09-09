#include <assert.h>
#include <stdlib.h>

#include "expr.h"
#include "gc.h"

Gc *create_gc(void)
{
    return NULL;
}

void destroy_gc(Gc *gc)
{
    assert(gc);
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
