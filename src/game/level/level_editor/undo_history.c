#include <stdlib.h>
#include <stdio.h>

#include <SDL.h>

#include "system/nth_alloc.h"
#include "system/lt.h"
#include "system/stacktrace.h"
#include "undo_history.h"

typedef struct {
    RevertAction revert;
} HistoryAction;

void undo_history_push(UndoHistory *undo_history,
                       RevertAction revert,
                       void *context_data,
                       size_t context_data_size)
{
    trace_assert(undo_history);

    HistoryAction action = {
        .revert = revert,
    };

    stack_push(&undo_history->actions, context_data, context_data_size);
    stack_push(&undo_history->actions, &action, sizeof(action));
}

void undo_history_pop(UndoHistory *undo_history)
{
    trace_assert(undo_history);

    if (stack_empty(&undo_history->actions) > 0) {
        HistoryAction action = *(HistoryAction *)stack_top_element(&undo_history->actions);
        stack_pop(&undo_history->actions);

        size_t context_size = stack_top_size(&undo_history->actions);
        void *context = stack_top_element(&undo_history->actions);

        action.revert(context, context_size);
        stack_pop(&undo_history->actions);
    }
}
