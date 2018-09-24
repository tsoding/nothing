#include <assert.h>

#include "console.h"
#include "game/edit_field.h"
#include "game/level.h"
#include "game/level/player/rigid_rect.h"
#include "script/gc.h"
#include "script/interpreter.h"
#include "script/parser.h"
#include "script/scope.h"
#include "system/error.h"
#include "system/lt.h"

struct Console
{
    Lt *lt;
    Gc *gc;
    struct Scope scope;
    Edit_field *edit_field;
    Level *level;
};

/* TODO(#353): Console does not show previous expression */
/* TODO(#354): Console does not allow to travel the history by pressing up and down */
/* TODO(#355): Console does not support Emacs keybindings */
/* TODO(#356): Console does not support autocompletion */
/* TODO(#357): Console does not show the state of the GC of the script */
/* TODO(#358): Console does not support copy, cut, paste operations */

static struct EvalResult rect_apply_force(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    assert(gc);
    assert(scope);
    assert(param);

    Level *level = (Level*) param;
    const char *rect_id = CAR(args).atom->str;
    struct Expr vector_force_expr = CAR(CDR(args));
    const float force_x = (float) CAR(vector_force_expr).atom->num;
    const float force_y = (float) CDR(vector_force_expr).atom->num;

    print_expr_as_sexpr(args); printf("\n");

    Rigid_rect *rigid_rect = level_rigid_rect(level, rect_id);
    if (rigid_rect != NULL) {
        printf("Found rect `%s`\n", rect_id);
        printf("Applying force (%f, %f)\n", force_x, force_y);
        rigid_rect_apply_force(rigid_rect, vec(force_x, force_y));
    } else {
        fprintf(stderr, "Couldn't find rigid_rect `%s`", rect_id);
    }

    return eval_success(NIL(gc));
}

Console *create_console(Level *level,
                        const Sprite_font *font)
{
    Lt *lt = create_lt();

    if (lt == NULL) {
        return NULL;
    }

    Console *console = PUSH_LT(lt, malloc(sizeof(Console)), free);
    if (console == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    console->lt = lt;

    console->gc = PUSH_LT(lt, create_gc(), destroy_gc);
    if (console->gc == NULL) {
        RETURN_LT(lt, NULL);
    }

    console->scope.expr = CONS(console->gc,
                               NIL(console->gc),
                               NIL(console->gc));
    set_scope_value(
        console->gc,
        &console->scope,
        SYMBOL(console->gc, "rect-apply-force"),
        NATIVE(console->gc, rect_apply_force, level));

    console->edit_field = PUSH_LT(
        lt,
        create_edit_field(
            font,
            vec(3.0f, 3.0f),
            color(0.0f, 0.0f, 0.0f, 1.0f)),
        destroy_edit_field);
    if (console->edit_field == NULL) {
        RETURN_LT(lt, NULL);
    }

    console->level = level;

    return console;
}

void destroy_console(Console *console)
{
    assert(console);
    RETURN_LT0(console->lt);
}

int console_handle_event(Console *console,
                         const SDL_Event *event)
{
    switch(event->type) {
    case SDL_KEYDOWN:
        switch(event->key.keysym.sym) {
        case SDLK_RETURN: {
            const char *source_code = edit_field_as_text(console->edit_field);
            struct ParseResult parse_result = read_expr_from_string(console->gc,
                                                                    source_code);
            if (parse_result.is_error) {
                /* TODO(#359): Console doesn't report any parsing errors visually */
                print_parse_error(stderr, source_code, parse_result);
                return 0;
            }

            struct EvalResult eval_result = eval(
                console->gc,
                &console->scope,
                parse_result.expr);

            if (eval_result.is_error) {
                /* TODO: Console doesn't report any eval errors visually */
                printf("Error:\t");
                print_expr_as_sexpr(eval_result.expr);
                printf("\n");
            }

            gc_collect(console->gc, console->scope.expr);

            edit_field_clean(console->edit_field);

        } return 0;
        }
        break;
    }

    return edit_field_handle_event(console->edit_field,
                                   event);
}

int console_render(const Console *console,
                   SDL_Renderer *renderer)
{
    return edit_field_render(console->edit_field,
                             renderer,
                             vec(0.0f, 0.0f));
}
