#include <stdlib.h>
#include <stdio.h>

#include <SDL.h>

#include "system/nth_alloc.h"
#include "system/lt.h"
#include "system/stacktrace.h"
#include "undo_history.h"
#include "stack.h"

struct UndoHistory {
    Lt *lt;
    Stack actions;
};

typedef struct {
    RevertAction revert;
    void *layer;
} Action;

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

    Action action = {
        .revert = revert,
        .layer = layer
    };

    stack_push(&undo_history->actions, context_data, context_data_size);
    stack_push(&undo_history->actions, &action, sizeof(action));
}

void undo_history_pop(UndoHistory *undo_history)
{
    trace_assert(undo_history);

    if (stack_empty(&undo_history->actions) > 0) {
        Action action = *(Action *)stack_top_element(&undo_history->actions);
        stack_pop(&undo_history->actions);

        size_t context_size = stack_top_size(&undo_history->actions);
        void *context = stack_top_element(&undo_history->actions);

        action.revert(action.layer, context, context_size);
        stack_pop(&undo_history->actions);
    }
}
