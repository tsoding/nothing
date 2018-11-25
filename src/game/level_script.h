#ifndef LEVEL_SCRIPT_H_
#define LEVEL_SCRIPT_H_

#include "ebisp/expr.h"

typedef struct Gc Gc;
struct Scope;
typedef struct Level Level;

void load_level_library(Gc *gc, struct Scope *scope, Level *level);

#endif  // LEVEL_SCRIPT_H_
