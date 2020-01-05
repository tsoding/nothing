#include <stdlib.h>
#include <stdio.h>

#include <SDL.h>

#include "system/nth_alloc.h"
#include "system/lt.h"
#include "system/stacktrace.h"
#include "undo_history.h"
#include "config.h"

typedef struct {
    RevertAction revert;
    void *context_data;
    size_t context_data_size;
} HistoryItem;

static
void undo_history_destroy_item(void *item)
{
    free(((HistoryItem*)item)->context_data);
}

UndoHistory create_undo_history(void)
{
    UndoHistory result;
    result.actions = create_ring_buffer(
        sizeof(HistoryItem),
        UNDO_HISTORY_CAPACITY,
        undo_history_destroy_item);
    return result;
}

void undo_history_push(UndoHistory *undo_history,
                       RevertAction revert,
                       void *context_data,
                       size_t context_data_size)
{
    trace_assert(undo_history);

    HistoryItem item = {
        .revert = revert,
        .context_data = malloc(context_data_size),
        .context_data_size = context_data_size
    };
    trace_assert(item.context_data);
    memcpy(item.context_data, context_data, context_data_size);

    ring_buffer_push(&undo_history->actions, &item);
}

void undo_history_pop(UndoHistory *undo_history)
{
    trace_assert(undo_history);

    if (undo_history->actions.count > 0) {
        HistoryItem *item = ring_buffer_top(&undo_history->actions);
        item->revert(item->context_data, item->context_data_size);
        ring_buffer_pop(&undo_history->actions);
    }
}
