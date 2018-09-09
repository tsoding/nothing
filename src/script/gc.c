#include <assert.h>
#include <stdlib.h>

#include "expr.h"
#include "gc.h"
#include "system/error.h"
#include "system/lt.h"

#define GC_INITIAL_EXPRS_CAPACITY 256

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

int gc_add_expr(Gc *gc, struct Expr expr)
{
    assert(gc);

    if (gc->exprs_size >= gc->exprs_capacity) {
        const size_t new_capacity = gc->exprs_capacity * 2;
        struct Expr *const new_exprs = realloc(
            gc->exprs,
            sizeof(struct Expr) * new_capacity);

        if (new_exprs == NULL) {
            return -1;
        }

        gc->exprs_capacity = new_capacity;
        gc->exprs = REPLACE_LT(gc->lt, gc->exprs, new_exprs);
    }

    gc->exprs[gc->exprs_size++] = expr;

    return 0;
}

void gc_collect(Gc *gc, struct Expr root)
{
    assert(gc);
    (void) root;
}
