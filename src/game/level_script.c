#include "system/stacktrace.h"

#include "ebisp/gc.h"
#include "ebisp/interpreter.h"
#include "ebisp/scope.h"
#include "game/level/player/rigid_rect.h"
#include "level.h"
#include "level_script.h"
#include "system/log.h"

static struct EvalResult
rect_apply_force(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    trace_assert(gc);
    trace_assert(scope);
    trace_assert(param);

    Level *level = (Level*) param;

    const char *rect_id = NULL;
    struct Expr force = void_expr();
    struct EvalResult result = match_list(gc, "se", args, &rect_id, &force);
    if (result.is_error) {
        return result;
    }

    long int force_x = 0L;
    long int force_y = 0L;
    result = match_list(gc, "dd", force, &force_x, &force_y);
    if (result.is_error) {
        return result;
    }

    Rigid_rect *rigid_rect = level_rigid_rect(level, rect_id);
    if (rigid_rect != NULL) {
        log_info("Found rect `%s`\n", rect_id);
        log_info("Applying force (%ld, %ld)\n", force_x, force_y);
        rigid_rect_apply_force(rigid_rect, vec((float) force_x, (float) force_y));
        return eval_success(NIL(gc));
    } else {
        return eval_failure(list(gc, "qs", "unexisting-rigid-rect", rect_id));
    }
}

static struct EvalResult
hide_label(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    trace_assert(param);
    trace_assert(gc);
    trace_assert(scope);

    Level *level = (Level*) param;
    const char *label_id = NULL;

    struct EvalResult result = match_list(gc, "s", args, &label_id);
    if (result.is_error) {
        return result;
    }

    level_hide_label(level, label_id);

    return eval_success(NIL(gc));
}

void load_level_library(Gc *gc, struct Scope *scope, Level *level)
{
    set_scope_value(
        gc,
        scope,
        SYMBOL(gc, "rect-apply-force"),
        NATIVE(gc, rect_apply_force, level));
    set_scope_value(
        gc,
        scope,
        SYMBOL(gc, "hide-label"),
        NATIVE(gc, hide_label, level));
}
