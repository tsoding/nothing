#ifndef GAME_SCRIPT_H_
#define GAME_SCRIPT_H_

#include "ebisp/expr.h"

typedef struct Game Game;

void load_game_library(Gc *gc, struct Scope *scope, Game *game);

#endif  // GAME_SCRIPT_H_
