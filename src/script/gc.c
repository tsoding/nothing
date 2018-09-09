#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "expr.h"
#include "gc.h"
#include "system/error.h"
#include "system/lt.h"

#define GC_INITIAL_CAPACITY 256

struct Gc
{
    Lt *lt;
    struct Expr *exprs;
    int *visited;
    size_t size;
    size_t capacity;
};

static int compare_exprs(const void *a, const void *b)
{
    assert(a);
    assert(b);

    const struct Expr *expr_a = (const struct Expr *)a;
    const struct Expr *expr_b = (const struct Expr *)b;

    const void *ptr_a = NULL;
    if (expr_a->type == EXPR_CONS) {
        ptr_a = expr_a->cons;
    } else if (expr_a->type == EXPR_ATOM) {
        ptr_a = expr_a->atom;
    }

    const void *ptr_b = NULL;
    if (expr_b->type == EXPR_CONS) {
        ptr_b = expr_b->cons;
    } else if (expr_b->type == EXPR_ATOM) {
        ptr_b = expr_b->atom;
    }

    const long int d = (long int) ptr_b - (long int) ptr_a;

    if (d < 0) {
        return -1;
    } else if (d > 0) {
        return 1;
    } else {
        return 0;
    }
}

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

    gc->exprs = PUSH_LT(lt, malloc(sizeof(struct Expr) * GC_INITIAL_CAPACITY), free);
    if (gc->exprs == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    gc->visited = PUSH_LT(lt, malloc(sizeof(int) * GC_INITIAL_CAPACITY), free);
    if (gc->visited == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    gc->size = 0;
    gc->capacity = GC_INITIAL_CAPACITY;

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

    if (gc->size >= gc->capacity) {
        const size_t new_capacity = gc->capacity * 2;
        struct Expr *const new_exprs = realloc(
            gc->exprs,
            sizeof(struct Expr) * new_capacity);

        if (new_exprs == NULL) {
            return -1;
        }

        gc->capacity = new_capacity;
        gc->exprs = REPLACE_LT(gc->lt, gc->exprs, new_exprs);
    }

    gc->exprs[gc->size++] = expr;

    return 0;
}

static int gc_find_expr(Gc *gc, struct Expr expr)
{
    assert(gc);
    (void) expr;
    /* TODO: gc_find_expr is not implemented  */
    return 0;
}

static void gc_traverse_expr(Gc *gc, struct Expr root)
{
    assert(gc);
    const int root_index = gc_find_expr(gc, root);
    assert(root_index >= 0);

    if (gc->visited[root_index]) {
        return;
    }

    if (cons_p(root)) {
        gc_traverse_expr(gc, root.cons->car);
        gc_traverse_expr(gc, root.cons->cdr);
    }

    gc->visited[root_index] = 1;
}

void gc_collect(Gc *gc, struct Expr root)
{
    assert(gc);
    (void) root;

    /* Sort gc->exprs O(nlogn) */
    qsort(gc->exprs, gc->size, sizeof(struct Expr), compare_exprs);

    /* Defragment O(n) */
    while(gc->size > 0 && gc->exprs[gc->size - 1].type == EXPR_VOID) {
        gc->size--;
    }

    /* Initialize visited array O(n) */
    memset(gc->visited, 0, sizeof(int) * gc->size);

    /* Traverse root O(nlogn)  */
    gc_traverse_expr(gc, root);

    /* Dealloc unvisted O(n)   */
    for (size_t i = 0; i < gc->size; ++i) {
        if (!gc->visited[i]) {
            destroy_expr(gc->exprs[i]);
            gc->exprs[i] = void_expr();
        }
    }
}
