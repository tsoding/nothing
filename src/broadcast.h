#ifndef BROADCAST_H_
#define BROADCAST_H_

#include "ebisp/expr.h"

typedef struct Broadcast Broadcast;
typedef struct Game Game;

Broadcast *create_broadcast(Game *game);
void destroy_broadcast(Broadcast *broadcast);

struct EvalResult
broadcast_send(Broadcast *broadcast,
               Gc *gc,
               struct Scope *scope,
               struct Expr path);

struct EvalResult broadcast_load_library(Broadcast *broadcast,
                                         Gc *gc,
                                         struct Scope *scope);

struct EvalResult
unknown_target(Gc *gc, const char *source, const char *target);

#endif  // BROADCAST_H_
