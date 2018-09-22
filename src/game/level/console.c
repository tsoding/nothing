#include <assert.h>

#include "console.h"
#include "script/gc.h"
#include "script/interpreter.h"
#include "script/scope.h"
#include "system/lt.h"
#include "system/error.h"
#include "game/level.h"

struct Console
{
    Lt *lt;
    Gc *gc;
    struct Scope scope;
    Level *level;
};

Console *create_console(Level *level)
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

    console->level = level;

    return console;
}

void destroy_console(Console *console)
{
    assert(console);
    RETURN_LT0(console->lt);
}
