#ifndef GC_H_
#define GC_H_

#include "expr.h"

typedef struct Gc Gc;

Gc *create_gc(void);
void destroy_gc(Gc *gc);

int gc_add_expr(Gc *gc, struct Expr expr);
void gc_collect(Gc *gc, struct Expr root);
void gc_inspect(const Gc *gc);

#endif  // GC_H_
