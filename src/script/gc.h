#ifndef GC_H_
#define GC_H_

typedef struct Gc Gc;

Gc *create_gc(void);
void destroy_gc(Gc *gc);

int gc_add_expr(Gc *gc, struct Expr expr);
void gc_collect(Gc *gc, struct Expr root);

#endif  // GC_H_
