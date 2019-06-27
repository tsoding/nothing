#include "system/stacktrace.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "builtins.h"
#include "expr.h"
#include "gc.h"
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

static intptr_t value_of_expr(struct Expr expr)
{
    if (expr.type == EXPR_CONS) {
        return (intptr_t) expr.cons;
    } else if (expr.type == EXPR_ATOM) {
        return (intptr_t) expr.atom;
    } else {
        return 0;
    }
}

static int compare_exprs(const void *a, const void *b)
{
    trace_assert(a);
    trace_assert(b);

    const intptr_t ptr_a = value_of_expr(*(const struct Expr *)a);
    const intptr_t ptr_b = value_of_expr(*(const struct Expr *)b);
    const intptr_t d = ptr_b - ptr_a;

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

    Gc *gc = PUSH_LT(lt, calloc(1, sizeof(Gc)), free);
    if (gc == NULL) {
        RETURN_LT(lt, NULL);
    }
    gc->lt = lt;

    gc->exprs = PUSH_LT(lt, calloc(GC_INITIAL_CAPACITY, sizeof(struct Expr)), free);
    if (gc->exprs == NULL) {
        RETURN_LT(lt, NULL);
    }

    gc->visited = PUSH_LT(lt, calloc(GC_INITIAL_CAPACITY, sizeof(int)), free);
    if (gc->visited == NULL) {
        RETURN_LT(lt, NULL);
    }

    gc->size = 0;
    gc->capacity = GC_INITIAL_CAPACITY;

    return gc;
}

void destroy_gc(Gc *gc)
{
    trace_assert(gc);

    for (size_t i = 0; i < gc->size; ++i) {
        destroy_expr(gc->exprs[i]);
    }

    RETURN_LT0(gc->lt);
}

int gc_add_expr(Gc *gc, struct Expr expr)
{
    trace_assert(gc);

    if (gc->size >= gc->capacity) {
        const size_t new_capacity = gc->capacity * 2;
        struct Expr *const new_exprs = realloc(
            gc->exprs,
            sizeof(struct Expr) * new_capacity);

        if (new_exprs == NULL) {
            return -1;
        }

        int *const new_visited = realloc(
            gc->visited,
            sizeof(int) * new_capacity);

        if (new_visited == NULL) {
            return -1;
        }

        gc->capacity = new_capacity;
        gc->exprs = REPLACE_LT(gc->lt, gc->exprs, new_exprs);
        gc->visited = REPLACE_LT(gc->lt, gc->visited, new_visited);
    }

    gc->exprs[gc->size++] = expr;

    return 0;
}

static long int gc_find_expr(Gc *gc, struct Expr expr)
{
    trace_assert(gc);
    (void) expr;

    struct Expr *result =
        (struct Expr *) bsearch(&expr, gc->exprs, gc->size,
                                sizeof(struct Expr), compare_exprs);

    if (result == NULL) {
        return -1;
    }

    return (long int) (result - gc->exprs);
}

static void gc_traverse_expr(Gc *gc, struct Expr root)
{
    trace_assert(gc);
    trace_assert(root.type != EXPR_VOID);
    const long int root_index = gc_find_expr(gc, root);
    if (root_index < 0) {
        fprintf(stderr, "GC tried to collect something that was not registered\n");
        print_expr_as_sexpr(stderr, root);
        fprintf(stderr, "\n");
        trace_assert(root_index >= 0);
    }

    if (gc->visited[root_index]) {
        return;
    }

    gc->visited[root_index] = 1;

    if (cons_p(root)) {
        gc_traverse_expr(gc, root.cons->car);
        gc_traverse_expr(gc, root.cons->cdr);
    } else if (root.type == EXPR_ATOM
               && root.atom->type == ATOM_LAMBDA) {
        gc_traverse_expr(gc, root.atom->lambda.args_list);
        gc_traverse_expr(gc, root.atom->lambda.body);
        gc_traverse_expr(gc, root.atom->lambda.envir);
    }
}

void gc_collect(Gc *gc, struct Expr root)
{
    trace_assert(gc);
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

void gc_inspect(const Gc *gc)
{
    for (size_t i = 0; i < gc->size; ++i) {
        if (gc->exprs[i].type == EXPR_VOID) {
            printf(".");
        } else {
            printf("+");
        }
    }
    printf("\n");
}
