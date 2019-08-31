#include <stdlib.h>
#include <stdio.h>

#include <SDL.h>

#include "system/nth_alloc.h"
#include "system/lt.h"
#include "system/stacktrace.h"
#include "undo_history.h"
#include "stack.h"

struct UndoHistory
{
    Lt *lt;
    Stack actions;
};



typedef struct {
    void *layer;
    Context context;
    RevertAction revert;
} Action;

static inline
Action create_action(void *layer, RevertAction revert,
                     void *context_data,
                     size_t context_data_size)
{
    trace_assert(layer);
    trace_assert(revert);
    trace_assert(context_data_size < CONTEXT_SIZE);

    Action action = {
        .layer = layer,
        .revert = revert
    };

    if (context_data) {
        memcpy(action.context.data, context_data, context_data_size);
    }

    return action;
}

UndoHistory *create_undo_history(void)
{
    Lt *lt = create_lt();

    UndoHistory *undo_history = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(UndoHistory)),
        free);
    undo_history->lt = lt;

    return undo_history;
}

void destroy_undo_history(UndoHistory *undo_history)
{
    trace_assert(undo_history);
    destroy_stack(undo_history->actions);
    RETURN_LT0(undo_history->lt);
}

void undo_history_push(UndoHistory *undo_history,
                       void *layer,
                       RevertAction revert,
                       void *context_data,
                       size_t context_data_size)
{
    trace_assert(undo_history);

    Action action = create_action(
        layer,
        revert,
        context_data,
        context_data_size);

    stack_push(
        &undo_history->actions,
        &action,
        sizeof(action));
}

void undo_history_pop(UndoHistory *undo_history)
{
    trace_assert(undo_history);

    if (stack_empty(&undo_history->actions) > 0) {
        Action *action = stack_top_element(&undo_history->actions);
        action->revert(action->layer, action->context);
        stack_pop(&undo_history->actions);
    }
}
