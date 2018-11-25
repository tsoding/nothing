#ifndef LEVEL_SCRIPT_H_
#define LEVEL_SCRIPT_H_

#include "ebisp/expr.h"

typedef struct Gc Gc;
struct Scope;
typedef struct Level Level;

// TODO(#525): Native lisp functions should probably have some kind of prefix

struct EvalResult
rect_apply_force(void *param, Gc *gc, struct Scope *scope, struct Expr args);

struct EvalResult
hide_goal(void *param, Gc *gc, struct Scope *scope, struct Expr args);

struct EvalResult
show_goal(void *param, Gc *gc, struct Scope *scope, struct Expr args);

struct EvalResult
hide_label(void *param, Gc *gc, struct Scope *scope, struct Expr args);

struct EvalResult
show_label(void *param, Gc *gc, struct Scope *scope, struct Expr args);

struct EvalResult
get_player_jump_count(void *param, Gc *gc, struct Scope *scope, struct Expr args);

void load_level_library(Gc *gc, struct Scope *scope, Level *level);

#endif  // LEVEL_SCRIPT_H_
