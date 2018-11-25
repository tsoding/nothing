#ifndef REPL_RUNTIME_H_
#define REPL_RUNTIME_H_

typedef struct Gc Gc;
struct Scope;

void load_repl_runtime(Gc *gc, struct Scope *scope);

#endif  // REPL_RUNTIME_H_
