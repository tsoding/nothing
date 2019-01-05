#include "ebisp/interpreter.h"
#include "ebisp/scope.h"
#include "game.h"
#include "game_script.h"
#include "system/stacktrace.h"

static struct EvalResult
send(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    trace_assert(param);
    trace_assert(gc);
    trace_assert(scope);

    Game *game = (Game*) param;

    struct Expr path = void_expr();
    struct EvalResult result = match_list(gc, "e", args, &path);
    if (result.is_error) {
        return result;
    }

    return game_send(game, gc, scope, path);
}

void load_game_library(Gc *gc, struct Scope *scope, Game *game)
{
    trace_assert(gc);
    trace_assert(scope);
    trace_assert(game);

    set_scope_value(gc, scope, SYMBOL(gc, "send"), NATIVE(gc, send, game));
}
