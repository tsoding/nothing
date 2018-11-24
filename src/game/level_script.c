#include <assert.h>

#include "ebisp/gc.h"
#include "ebisp/interpreter.h"
#include "ebisp/scope.h"
#include "game/level/player/rigid_rect.h"
#include "level.h"
#include "level_script.h"
#include "system/log.h"

struct EvalResult
hide_goal(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    assert(param);
    assert(gc);
    assert(scope);

    Level * const level = (Level*)param;
    const char * const goal_id = NULL;

    struct EvalResult result = unpack_args(gc, "s", args, &goal_id);
    if (result.is_error) {
        return result;
    }

    level_hide_goal(level, goal_id);

    return eval_success(NIL(gc));
}

struct EvalResult
show_goal(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    assert(param);
    assert(gc);
    assert(scope);

    Level * const level = (Level*)param;
    const char *goal_id = NULL;

    struct EvalResult result = unpack_args(gc, "s", args, &goal_id);
    if (result.is_error) {
        return result;
    }

    level_show_goal(level, goal_id);

    return eval_success(NIL(gc));
}

struct EvalResult rect_apply_force(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    assert(gc);
    assert(scope);
    assert(param);

    /* TODO(#401): rect_apply_force doesn't sanitize it's input */

    Level *level = (Level*) param;
    const char *rect_id = CAR(args).atom->str;
    struct Expr vector_force_expr = CAR(CDR(args));
    const float force_x = (float) CAR(vector_force_expr).atom->num;
    const float force_y = (float) CDR(vector_force_expr).atom->num;

    print_expr_as_sexpr(stdout, args); printf("\n");

    Rigid_rect *rigid_rect = level_rigid_rect(level, rect_id);
    if (rigid_rect != NULL) {
        log_info("Found rect `%s`\n", rect_id);
        log_info("Applying force (%f, %f)\n", force_x, force_y);
        rigid_rect_apply_force(rigid_rect, vec(force_x, force_y));
    } else {
        log_fail("Couldn't find rigid_rect `%s`\n", rect_id);
    }

    return eval_success(NIL(gc));
}

struct EvalResult
hide_label(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    assert(param);
    assert(gc);
    assert(scope);
    (void) args;

    /* TODO: hide-label is not implemented */

    return not_implemented(gc);
}

struct EvalResult
show_label(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    assert(param);
    assert(gc);
    assert(scope);
    (void) args;

    /* TODO: show-label is not implemented */

    return not_implemented(gc);
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
        SYMBOL(gc, "hide-goal"),
        NATIVE(gc, hide_goal, level));
    set_scope_value(
        gc,
        scope,
        SYMBOL(gc, "show-goal"),
        NATIVE(gc, show_goal, level));
    set_scope_value(
        gc,
        scope,
        SYMBOL(gc, "show-label"),
        NATIVE(gc, show_label, level));
    set_scope_value(
        gc,
        scope,
        SYMBOL(gc, "hide-label"),
        NATIVE(gc, hide_label, level));
}
