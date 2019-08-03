#include <stdlib.h>

#include <SDL.h>

#include "system/nth_alloc.h"
#include "system/lt.h"
#include "dynarray.h"
#include "system/stacktrace.h"
#include "undo_history.h"

struct UndoHistory
{
    Lt *lt;
    Dynarray *actions;
};

UndoHistory *create_undo_history(void)
{
    Lt *lt = create_lt();

    UndoHistory *undo_history = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(UndoHistory)),
        free);
    undo_history->lt = lt;

    undo_history->actions = PUSH_LT(
        lt,
        create_dynarray(sizeof(Action)),
        destroy_dynarray);

    return undo_history;
}

void destroy_undo_history(UndoHistory *undo_history)
{
    trace_assert(undo_history);
    RETURN_LT0(undo_history->lt);
}

void undo_history_push(UndoHistory *undo_history, Action action)
{
    trace_assert(undo_history);
    dynarray_push(undo_history->actions, &action);
}

void undo_history_pop(UndoHistory *undo_history)
{
    trace_assert(undo_history);

    if (dynarray_count(undo_history->actions) > 0) {
        Action action;
        dynarray_pop(undo_history->actions, &action);
        action.revert(action.layer, action.context);
    }
}
