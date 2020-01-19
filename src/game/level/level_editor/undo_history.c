#include <stdlib.h>
#include <stdio.h>

#include <SDL.h>

#include "system/nth_alloc.h"
#include "system/stacktrace.h"
#include "undo_history.h"
#include "config.h"

typedef struct {
    RevertAction revert;
    void *context_data;
    size_t context_data_size;
} HistoryItem;

UndoHistory *create_undo_history(Memory *memory)
{
    UndoHistory *result = memory_alloc(memory, sizeof(UndoHistory));
    result->actions = create_ring_buffer_from_buffer(
        memory,
        sizeof(HistoryItem),
        UNDO_HISTORY_CAPACITY);
    result->memory = memory;
    return result;
}

void undo_history_push(UndoHistory *undo_history,
                       RevertAction revert,
                       void *context_data,
                       size_t context_data_size)
{
    trace_assert(undo_history);

    // TODO(#1244): undo_history_push kinda leaks the memory
    HistoryItem item = {
        .revert = revert,
        .context_data = memory_alloc(undo_history->memory, context_data_size),
        .context_data_size = context_data_size
    };
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

void undo_history_clean(UndoHistory *undo_history)
{
    trace_assert(undo_history);

    while (undo_history->actions.count) {
        ring_buffer_pop(&undo_history->actions);
    }
}
