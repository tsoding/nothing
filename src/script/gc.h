#ifndef GC_H_
#define GC_H_

typedef struct Gc Gc;

Gc *create_gc(void);
void destroy_gc(Gc *gc);

void gc_add_atom(Gc *gc, const struct Atom *atom);
void gc_add_cons(Gc *gc, const struct Cons *cons);

void gc_collect(Gc *gc, struct Expr root);

#endif  // GC_H_
