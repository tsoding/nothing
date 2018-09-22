#include <assert.h>

#include "console.h"
#include "game/edit_field.h"
#include "game/level.h"
#include "script/gc.h"
#include "script/interpreter.h"
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
